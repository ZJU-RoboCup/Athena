#include "BallAdvanceDecision.h"
#include "ShootRangeList.h"
#include "SituationJudge.h"
#include "BallStatus.h"
#include "GDebugEngine.h"
#include "gpuBestAlgThread.h"
#include <CMmotion.h>
#include <RobotCapability.h>
#include <minwindef.h>

using namespace std;
namespace {
	bool DEBUG = false;
	enum {
		BEGINNING = 1,
		RUSH_TO_BALL,
		COMPUTE,
		SHOOT,
		LIGHT_KICK,
		LIGHT_CHIP,
		PASS,
		RECEIVE,
		RECEIVEPASS,
		RECEIVESHOOT,
		WAITTOUCH
	};
	const double CM_PREDICT_FACTOR = 1.5;
	const double FRICTION = 58;
	const double weight1 = 1.5;//敌方到传球线距离的权值
	const double weight2 = 0.5;//敌方到接球点距离的权值
	const double weight3 = 1;//敌方到射门线距离的权值
	const double weight4 = 100;//射门角度的权值
	const double weight5 = 0;//接球射门角度的权值
	//阀值
	const double thresholdForEnemy2PassLineDist = 70;
	const double thresholdForEnemy2ShootLineDist = 70;
	const double thresholdForEnemy2PointDist = 100;
	CGeoPoint* runPos = GPUBestAlgThread::Instance()->runPos;
	CGeoPoint* confirmPos = GPUBestAlgThread::Instance()->confirmPos;
	int* vehicleNumber = GPUBestAlgThread::Instance()->number;
	CGeoPoint& passPos = GPUBestAlgThread::Instance()->shootPos;
	bool& changeLeaderFlag = GPUBestAlgThread::Instance()->changeLeaderFlag;
	const CGeoRectangle* runArea = GPUBestAlgThread::Instance()->getRunArea();
	bool& changeLeaderOverFlag = GPUBestAlgThread::Instance()->changeLeaderOverFlag;
	int formation[13][3] = { {3,7,9},{8,4,6},{1,9,7},{3,8,9},{1,8,3},{1,8,7},{1,9,3},{1,3,9},{3,7,1},{5,7,9},{1,9,5},{4,5,6},{3,7,5} };
};

CBallAdvanceDecision::CBallAdvanceDecision(){
	const double areaLength = 30;
	const double minX = 0;
	const double maxX = 450;
	const double minY = -300;
	const double maxY = 300;
	const double lineAmount = (maxX - minX) / areaLength;
	const double columnAmount = (maxY - minY) / areaLength;
	area.location.push_back(CGeoRectangle(0, 0, 0, 0));
	area.targetPos.push_back(CGeoPoint(0,0));
	area.status.push_back(false);
	area.lineAmount = lineAmount;
	area.columnAmount = columnAmount;
	double x = maxX;
	double y = minY;
	for (int i = 1; i <= lineAmount; i++) {
		for (int j = 1; j <= columnAmount; j++) {
			area.location.push_back(CGeoRectangle(x, y, x - areaLength, y + areaLength));
			area.targetPos.push_back(CGeoPoint((2 * x - areaLength) / 2, (2 * y + areaLength) / 2));
			area.status.push_back(false);
			y += areaLength;
		}
		x -= areaLength;
		y = minY;
	}
	ballTransit = true;
	changeLeaderOverFlag = false;
	_receiver = 999;
	_stateCounter = 0;
	_lastCycle = 0;
	_lastBallArea = 1;
}

CBallAdvanceDecision::~CBallAdvanceDecision() { 

}

string CBallAdvanceDecision::generateAttackDecision(const CVisionModule* pVision, int meNum) {
	update(pVision, meNum);
	const MobileVisionT& ball = _pVision->Ball();
	const PlayerVisionT& me = _pVision->OurPlayer(_leader);
	bool isBallNearMe = ball.Pos().dist(me.Pos()) < 35;
	bool needRushToBall = me.Pos().dist(ball.Pos()) > 50;
	bool ballIsKickedOut = false;
	double leaderTime = robotTimeCompute(_leader);
	double minTime = 999;
	int bestPlayer = _leader;
	string ballStatus = WorldModel::Instance()->getBallStatus(_pVision->Cycle(), _leader);//world:getBallStatus(vision : Cycle(), gRoleNum["Leader"])
	//cout << "ballStatus " << ballStatus << endl;
	for (int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
		if (i != _leader && i!= vehicleNumber[5]) {
			double getBallTime = robotTimeCompute(i);
			//cout << getBallTime << " " << i << endl;
			if (getBallTime < minTime && getBallTime < leaderTime) {
				bestPlayer = i;
				minTime = getBallTime;
			}
		}				
	}
	//cout << bestPlayer << endl;
	bool reset = false;
	if (_leader != bestPlayer && robotTimeCompute(_leader) > robotTimeCompute(bestPlayer)*1.2)
		_bestPlayerCount++;
	else
		_bestPlayerCount = 0;
	if (_bestPlayerCount >= 40) {
		_receiver = bestPlayer;
		confirmLeader();
		reset = true;
	}
	//cout << "bestplayer " << bestPlayer << endl;
	//cout << _bestPlayerCount << "_bestPlayerCount" << endl;
	if (reset) {
		if (DEBUG)
			cout << "reset" << endl;
		_state = BEGINNING;
		_bestPlayerCount = 0;
		_receiver = 999;
		ballIsKickedOut = true;
		ballTransitRecord = false;
		generateRunPosition();
	}
	if ((_pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1)) {
		_state = BEGINNING;
		_bestPlayerCount = 0;
		_receiver = 999;
		ballIsKickedOut = true;
		ballTransitRecord = false;
		changeLeaderFlag = false;
		changeLeaderOverFlag = false;
		generateRunPosition();
	}
	ballTransit = BallStatus::Instance()->IsBallKickedOut();
	if (!ballTransit)
		ballTransitRecord = true;
	if (ballTransitRecord && ballTransit) {
		//cout << "ball is kickedout" << endl;
		ballIsKickedOut = true;
		ballTransitRecord = false;
	}
	if (changeLeaderOverFlag) {
		changeLeaderOverFlag = false;
		changeLeaderFlag = false;
		//generateRunPosition();
	}
	/*if (ballIsKickedOut) {
		cout << "generateRunPos" << endl;
		generateRunPosition();
	}*/
	int new_state = _state;
	switch (new_state) {
	case BEGINNING:
		new_state = COMPUTE;
		break;
	case RUSH_TO_BALL:
		if (isBallNearMe)
			new_state = COMPUTE;
		break;
	case COMPUTE:
		if (needRushToBall)
			new_state = RUSH_TO_BALL;
		else if (!canShoot()) {
			if (canLightKick())
				new_state = LIGHT_KICK;
			else if (canLightChip())
				new_state = LIGHT_CHIP;
			else {
				generateReceiveBallPosition();
				if (canPass()) {
					new_state = PASS;
					_stateCounter = 0;
				}
				else {
					new_state = SHOOT;
				}
			}
		}
		else
			new_state = SHOOT;
		break;
	case SHOOT:
		if (ballIsKickedOut) {
			new_state = COMPUTE;
			_stateCounter = 0;
		}
		break;
	case LIGHT_KICK:
		if (ballIsKickedOut) {
			new_state = COMPUTE;
			_stateCounter = 0;
		}
		else if (needRushToBall) {
			new_state = RUSH_TO_BALL;
		}
		break;
	case LIGHT_CHIP:
		if (ballIsKickedOut) {
			new_state = COMPUTE;
			_stateCounter = 0;
		}
		else if (needRushToBall) {
			new_state = RUSH_TO_BALL;
		}
		break;
	case PASS:
		if (ballIsKickedOut) {
			if (canWaitTouch()) {
				new_state = WAITTOUCH;
				_stateCounter = 0;
			}
			else {
				new_state = RECEIVE;
				_stateCounter = 0;
			}
			confirmLeader();
		}
		else if (needRushToBall) {
			new_state = COMPUTE;
		}
		break;
	case RECEIVE:
		if (!isBallMovingToMe() && false)
			new_state = COMPUTE;
		if (ball.Vel().mod() < 40) {
			if (isBallNearMe) {
				if (canShoot()) {
					new_state = RECEIVESHOOT;
					_stateCounter = 0;
				}
				else {
					generateReceiveBallPosition();
					if (canPass()) {
						new_state = PASS;
						_stateCounter = 0;
					}
				}
			}
		}
		break;
	case RECEIVEPASS:
		
		break;
	case RECEIVESHOOT:
		if (ballIsKickedOut) {
			new_state = COMPUTE;
			_stateCounter = 0;
		}
		break;
	case WAITTOUCH:
		if (!canShoot()) {
			new_state = RECEIVE;
		}
		if (ballIsKickedOut) {
			new_state = COMPUTE;
			_stateCounter = 0;
		}
		break;
	default:
		new_state = COMPUTE;
		break;	
	}
	if (_state == BEGINNING) {
		_state = new_state;
	}
	else {
		if (_stateCounter == 0) {
			if (DEBUG)
				cout << "change to " << new_state << endl;
			_state = new_state;
			_stateCounter++;
		}
		else {
			if (new_state == _state) {
				_stateCounter = min(State_Counter_Num, _stateCounter + 1);
			}
			else {
				_stateCounter = max(0, _stateCounter - 1);
			}
		}
	}
	string decision;
	switch (_state) {
	case BEGINNING:
		decision = "RUSH_TO_BALL";
		break;
	case RUSH_TO_BALL:
		decision = "GetBall";
		break;
	case COMPUTE:
		decision = "Compute";
		break;
	case SHOOT:
		decision = "Shoot";
		break;
	case LIGHT_KICK:
		decision = "LightKick";
		break;
	case LIGHT_CHIP:
		decision = "LightChip";
		break;
	case PASS:
		decision = "Pass";
		break;
	case RECEIVE:
		decision = "Receive";
		break;
	case RECEIVEPASS:
		decision = "ReceivePass";
		break;
	case RECEIVESHOOT:
		decision = "ReceiveShoot";
		break;
	case WAITTOUCH:
		decision = "WaitTouch";
		break;
	default:
		decision = "Shoot";
		break;
	}
	//cout << decision << " " << ballIsKickedOut << endl;
	if (ballIsKickedOut) {
		generateRunPosition();
	}
	_lastCycle = _pVision->Cycle();
	return decision;
}

bool CBallAdvanceDecision::canShoot() {
	return false;
	const MobileVisionT& ball = _pVision->Ball();
	CShootRangeList shootRangeList(_pVision, _leader, _pVision->OurPlayer(_leader).Pos());
	const CValueRange* bestRange = NULL;
	const CValueRangeList& shootRange = shootRangeList.getShootRange();
	const double minShootDir = 3 * Param::Math::PI / 180;
	double sizeOfDir;
	if (shootRange.size() > 0 && ball.X()>0) {
		bestRange = shootRange.getMaxRangeWidth();
		sizeOfDir = bestRange->getSize();
		if (sizeOfDir > minShootDir) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

bool CBallAdvanceDecision::canWaitTouch() {
	CShootRangeList shootRangeList(_pVision, _receiver, passPos);
	const CValueRange* bestRange = NULL;
	const CValueRangeList& shootRange = shootRangeList.getShootRange();
	double _raw_kick_dir;
	double determindir;
	if (shootRange.size() > 0) {
		bestRange = shootRange.getMaxRangeWidth();
		if (bestRange && bestRange->getWidth() > 0) {	// 要求射门空档足够大
			_raw_kick_dir = bestRange->getMiddle();
		}
		double metoballdir = (_pVision->Ball().Pos() - passPos).dir();
		determindir = fabs(Utils::Normalize(metoballdir - _raw_kick_dir));
	}
	if ((determindir * 180 / Param::Math::PI) < 80) {
		return true;
	}
	else
		return false;
}

void CBallAdvanceDecision::confirmLeader() {
	confirmPos[0] = _pVision->OurPlayer(_receiver).Pos();
	int count = 1;
	int attackerAmount = SituationJudge::Instance()->checkAttackerAmount(_pVision, _leader);
	for (int i = 0; i < attackerAmount; i++) {
		if (i != _receiver) {
			confirmPos[count] = _pVision->OurPlayer(vehicleNumber[i]).Pos();
			count++;  
		}
	}
	changeLeaderFlag = true;
}

bool CBallAdvanceDecision::canLightKick() {
	return false;
	const MobileVisionT& ball = _pVision->Ball();
	const int frontLineX = 100;
	bool excuteLightKick = true;
	if (ball.X() < frontLineX) {
		const MobileVisionT& ball = _pVision->Ball();
		const PlayerVisionT& me = _pVision->OurPlayer(_leader);
		const double determindist = 100;
		double closestDist = 9999;
		for (int i = 1; i <= 6; i++) {
			if (_pVision->TheirPlayer(i).Valid()) {
				double dirMe2Goal = (CGeoPoint(Param::Field::PITCH_LENGTH / 2.0, 0) - me.Pos()).dir();
				double dirMe2Ball = (ball.Pos() - me.Pos()).dir();
				double dirMe2Enemy = (_pVision->TheirPlayer(i).Pos() - me.Pos()).dir();
				double different1 = fabs(Utils::Normalize(dirMe2Goal - dirMe2Enemy));
				double different2 = fabs(Utils::Normalize(dirMe2Ball - dirMe2Enemy));
				closestDist = me.Pos().dist(_pVision->TheirPlayer(i).Pos());
				if (different1 <= Param::Math::PI / 2) {
					if (closestDist < determindist) {
						return false;
					}
				}
			}
		}
		return true;
	}
	return false;
}

bool CBallAdvanceDecision::canLightChip() {
	return false;
}

bool CBallAdvanceDecision::canPass() {
	if (_receiver >= 1 && _receiver <= Param::Field::MAX_PLAYER) {
		return true;
	}
	/*int attackerAmount = SituationJudge::Instance()->checkAttackerAmount(_pVision, _leader);
	if (_receiver >= 1 && _receiver <= Param::Field::MAX_PLAYER) {
		runPos[0] = _pVision->OurPlayer(_receiver).Pos();
		int count = 1;
		for (int i = 0; i < attackerAmount; i++) {
			if (i != _receiver) {
				runPos[count] = _pVision->OurPlayer(vehicleNumber[i]).Pos();
				count++;
			}
		}
		//generateRunPosition();
		return true;
	}*/
	return false;
}

bool CBallAdvanceDecision::isBallMovingToMe() {
	// 判断球是否向我而来
	const PlayerVisionT& me = _pVision->OurPlayer(_leader);
	const MobileVisionT& ball = _pVision->Ball();
	const CGeoPoint& myPos = _pVision->OurPlayer(_leader).Pos();
	const CGeoPoint& ballPos = ball.Pos();
	const double MIN_BALL_MOVING_VEL = 40;
	const double ballVelMod = ball.Vel().mod();
	const double ballVelDir = ball.Vel().dir();
	const double meToBallDir = (myPos - ballPos).dir();
	const double diffBetweenBallVelDirAndMeToBallDir = Utils::Normalize(ballVelDir - meToBallDir);
	bool isBallMovingToMe = false;
	if (ballVelMod > MIN_BALL_MOVING_VEL && fabs(diffBetweenBallVelDirAndMeToBallDir) < Param::Math::PI / 2)
		isBallMovingToMe = true;
	//if (!isBallMovingToMe)
		//cout << isBallMovingToMe << "isBallMovingToMe" << diffBetweenBallVelDirAndMeToBallDir <<"diffBetweenBallVelDirAndMeToBallDir"<<endl;
	return isBallMovingToMe;
}

void CBallAdvanceDecision::generateReceiveBallPosition() {
	if (DEBUG)
		cout << "generateReceivePos" << endl;
	const MobileVisionT& ball = _pVision->Ball();
	const CRobotCapability* robotCap = RobotCapFactory::Instance()->getRobotCap(_pVision->Side(), _leader);
	const double MaxAcceleration = robotCap->maxAcceleration(CVector(0, 0), 0);
	const double MaxSpeed = robotCap->maxSpeed(0);
	const double slack = 0;
	vector <CGeoPoint> tempPoint;
	vector <CGeoPoint> receivePoint;
	vector <CGeoPoint> rushPoint;
	int attackerAmount = SituationJudge::Instance()->checkAttackerAmount(_pVision, _leader);
	bool canFlagPassTo[Param::Field::MAX_PLAYER];
	int maxPointSize = -1;
	int pointCount;
	_receiver = 999;
	if (attackerAmount == 1 && DEBUG)
		cout << "attackerAmount is " << attackerAmount << endl;
	for (int i = 1; i < attackerAmount; i++) {
		pointCount = 0;
		for (int j = 1; j < area.location.size(); j++) {//CHANGED!!!
			double minAnemyTime = 9999;
			for (int k = 1; k <= Param::Field::MAX_PLAYER; k++) {
				const PlayerVisionT& anemy = _pVision->TheirPlayer(k);
				if (anemy.Valid()) {
					//double anemyTime = expectedCMPathTime(anemy, area.targetPos[j], 580, 350, CM_PREDICT_FACTOR);
					double anemyTime = predictedTime(anemy, area.targetPos[j]);
					if (anemyTime < minAnemyTime)
						minAnemyTime = anemyTime;
				}
			}
			//cout << *vehicleNumber << endl;
			const PlayerVisionT& me = _pVision->OurPlayer(vehicleNumber[i]);
			//double myTime = expectedCMPathTime(me, area.targetPos[j], MaxAcceleration, MaxSpeed, CM_PREDICT_FACTOR);
			double myTime = predictedTime(me, area.targetPos[j]);
			double ball2Target = ball.Pos().dist(area.targetPos[j]);
			double ballspeed = ball2Target * 1 + 100;
			if (ballspeed < 200) {
				ballspeed = 200;
			}
			else if (ballspeed > 550) {
				ballspeed = 550;
			}
			if (_pVision->OurPlayer(vehicleNumber[i]).X() > 0)
				tempPoint.push_back(_pVision->OurPlayer(vehicleNumber[i]).Pos());
			double ballPassTime = (-ballspeed + sqrt(ballspeed * ballspeed - 2 * FRICTION * ball2Target)) / (-FRICTION);
			if ((myTime + slack < minAnemyTime) && !Utils::InTheirPenaltyArea(area.targetPos[j], 10) && !Utils::OutOfField(area.targetPos[j])) {
				if (myTime < ballPassTime - 0.2) {
					area.status[j] = true;
					tempPoint.push_back(area.targetPos[j]);
					//GDebugEngine::Instance()->gui_debug_x(area.targetPos[j]);
				}
				//GDebugEngine::Instance()->gui_debug_x(area.targetPos[j]);
				pointCount++;
			}
			else {
				area.status[j] = false;
			}
		}
		if ((int)tempPoint.size()) {
			CGeoPoint theirGoal = CGeoPoint(450, 0);
			if (pointCount > maxPointSize || (pointCount == maxPointSize && _pVision->OurPlayer(vehicleNumber[i]).Pos().dist(theirGoal) < _pVision->OurPlayer(vehicleNumber[_receiver]).Pos().dist(theirGoal))) {
				if (ball.Pos().dist(_pVision->OurPlayer(vehicleNumber[i]).Pos()) > 100) {
					//cout << vehicleNumber[_receiver]<<" " <<_pVision->OurPlayer(vehicleNumber[_receiver]).Pos().dist(theirGoal) << " last dist " << vehicleNumber[vehicleNumber[i]] << " " << _pVision->OurPlayer(vehicleNumber[i]).Pos().dist(theirGoal) << " current dist" << endl;
					receivePoint = tempPoint;
					_receiver = i;
					maxPointSize = pointCount;
					//cout << pointCount << " pointCount " << maxPointSize << endl;
					//cout << "change "<<_receiver << " to " << vehicleNumber[i] << endl;
				}
			}
		}
		//cout << pointCount << " pointCount " << vehicleNumber[i] << endl;
		tempPoint.clear();
	}
	//cout << vehicleNumber[0] << "  " << vehicleNumber[1]<< "  " << vehicleNumber[2] << "  " << vehicleNumber[3] << endl;
	//cout << "-------------------------------" << endl;
	int receiver = 1;
	if (_receiver != 999) {
		if (DEBUG)
			cout << "generateeceiver " << vehicleNumber[_receiver] << endl;
		receiver = _receiver;
		runPos[receiver - 1] = receiveBallPointCompute(vehicleNumber[receiver], receivePoint);
		passPos = runPos[receiver - 1];
	}
	else {
		passPos = receiveBallPointCompute(vehicleNumber[receiver], receivePoint);
	}
	_receiver = vehicleNumber[_receiver];
	/*runPos[receiver-1] = receiveBallPointCompute(vehicleNumber[receiver], receivePoint);
	passPos = runPos[receiver];
	for (int i = 0; i < attackerAmount - 1; i++) {
	//if (i != _receiver)
	//runPos[i] = _pVision->OurPlayer(vehicleNumber[i]).Pos();
	}*/
}

CGeoPoint CBallAdvanceDecision::receiveBallPointCompute(int number, vector <CGeoPoint> targetPoint) {
	if (!(int) targetPoint.size()) return CGeoPoint(450, 0);
	const PlayerVisionT& me = _pVision->OurPlayer(number);
	const MobileVisionT& ball = _pVision->Ball();
	int bestPointNumber;
	for (int i = 0; i < targetPoint.size(); i++) {
		//射门角，接球角大小
		CShootRangeList shootRangeList(_pVision, 0, targetPoint[i]);
		const CValueRange* bestRange = NULL;
		const CValueRangeList& shootRange = shootRangeList.getShootRange();
		double sizeOfDir;
		double kick_dir;
		double determindir;
		double metoballdir = (ball.Pos() - targetPoint[i]).dir();
		double maxPointScore = -0xffff;
		double ball2PointDist = ball.Pos().dist(targetPoint[i]);
		if (shootRange.size() > 0) {
			bestRange = shootRange.getMaxRangeWidth();
			sizeOfDir = bestRange->getSize();
			kick_dir = bestRange->getMiddle();
			determindir = fabs(Utils::Normalize(metoballdir - kick_dir));
		}
		else {
			sizeOfDir = 0;
			kick_dir = (CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0) - targetPoint[i]).dir();
			determindir = fabs(Utils::Normalize(metoballdir - kick_dir));
		}
		//
		double dist, dist2;
		double enemy2PointDist, enemy2MeDist, enemy2ShootLineDist, enemy2PassLineDist;
		enemy2PointDist = enemy2MeDist = enemy2ShootLineDist = enemy2PassLineDist = 9999;
		int enemyNumRecord;
		CGeoLine shootLine(targetPoint[i], kick_dir);
		for (int j = 1; j <= Param::Field::MAX_PLAYER; j++) {
			if (_pVision->TheirPlayer(j).Valid()) {
				dist = _pVision->TheirPlayer(j).Pos().dist(targetPoint[i]);
				if (dist < enemy2PointDist)  enemy2PointDist = dist;//敌方到点距离
				dist = me.Pos().dist(_pVision->TheirPlayer(j).Pos());
				if (dist < enemy2MeDist) {
					enemy2MeDist = dist;//敌方到我的距离
					enemyNumRecord = j;
				}
				dist = _pVision->TheirPlayer(j).Pos().dist(shootLine.projection(_pVision->TheirPlayer(j).Pos()));
				if (dist < enemy2ShootLineDist) {
					enemy2ShootLineDist = dist;//敌方到射门线的距离
				}
				CGeoSegment passLine = CGeoSegment(ball.Pos(), targetPoint[i]);
				if (passLine.IsPointOnLineOnSegment(passLine.projection(_pVision->TheirPlayer(j).Pos()))) {
					dist = _pVision->TheirPlayer(j).Pos().dist(passLine.projection(_pVision->TheirPlayer(j).Pos()));
					if (dist < enemy2PassLineDist)
						enemy2PassLineDist = dist;//敌方到传球线的距离
				}
			}
		}
		enemy2PassLineDist = enemy2PassLineDist > thresholdForEnemy2PassLineDist ? thresholdForEnemy2PassLineDist : enemy2PassLineDist;
		enemy2ShootLineDist = enemy2ShootLineDist > thresholdForEnemy2ShootLineDist ? thresholdForEnemy2ShootLineDist : enemy2ShootLineDist;
		enemy2PointDist = enemy2PointDist > thresholdForEnemy2PointDist ? thresholdForEnemy2PointDist : enemy2PointDist;
		double score = enemy2PassLineDist * weight1 + enemy2PointDist * weight2 + enemy2ShootLineDist * weight3 + sizeOfDir * weight4 + determindir * weight5;
		if (score > maxPointScore) {
			maxPointScore = score;
			bestPointNumber = i;
		}
	}
	GDebugEngine::Instance()->gui_debug_x(targetPoint[bestPointNumber], COLOR_YELLOW);
	return targetPoint[bestPointNumber];
}

void CBallAdvanceDecision::generateRunPosition() {
	if (DEBUG)
		cout << "generateRunPos" << endl;
	const MobileVisionT& ball = _pVision->Ball();
	int attackerAmount = SituationJudge::Instance()->checkAttackerAmount(_pVision, _leader);
	int ballArea = 999;
	CGeoPoint tempPos;
	if (DEBUG)
		cout << "leader is " << _leader << " receiver is " << _receiver << endl;
	if (_receiver >= 1 && _receiver <= Param::Field::MAX_PLAYER) {
		tempPos = _pVision->OurPlayer(_receiver).Pos();
	}
	else {
		tempPos = ball.Pos();
	}
	for (int i = 1; i <= 9; i++) {
		if (runArea[i].HasPoint(tempPos)) {
			ballArea = i;
			break;
		}
	}
	if (ballArea == 999) {
		if (tempPos.x() > 0) {
			if (tempPos.y() > 0) {
				ballArea = 3;
			}
			else {
				ballArea = 1;
			}
		}
		else {
			if (tempPos.y() < -100) {
				ballArea = 11;
			}
			else if (tempPos.y() < 100) {
				ballArea = 12;
			}
			else {
				ballArea = 13;
			}
		}
	}
	if (DEBUG) {
		cout << "ballArea " << ballArea << endl;
		cout << "The runArea is ";
	}
	for (int i = 0; i <= 2; i++) {
		runPos[i] = GPUBestAlgThread::Instance()->generateRunPos(formation[ballArea - 1][i]);
		if (DEBUG)
			cout << formation[ballArea - 1][i] << " ";
	}
	if (DEBUG)
		cout << endl;
}

CGeoPoint CBallAdvanceDecision::runPointCompute(int number, vector <CGeoPoint> targetPoint) {
	return CGeoPoint(0, 0);
}

double CBallAdvanceDecision::robotTimeCompute(int vehicleNumber) {
	if (vehicleNumber<1 || vehicleNumber>Param::Field::MAX_PLAYER) {
		cout << "vehicleNumber error" << endl;
		return 999;
	}
	const MobileVisionT& ball = _pVision->Ball();
	const PlayerVisionT& me = _pVision->OurPlayer(vehicleNumber);
	CGeoPoint ball_predict_pos = ball.Pos();
	const double STEP_TIME = 1.0 / Param::Vision::FRAME_RATE;
	const int increase_step = 5;
	long predict_cycle = 0;
	double ball_time = 0;
	double robot_time = 0;
	const double robot_ahead_slack = 0.2;
	const CRobotCapability* robotCap = RobotCapFactory::Instance()->getRobotCap(_pVision->Side(), vehicleNumber);
	const double MaxSpeed = robotCap->maxSpeed(0);								//speed
	const double MaxAcceleration = robotCap->maxAcceleration(CVector(0, 0), 0);	//acceleration
	const double MAX_TIME = 5;
	if (ball.Vel().mod() > 1) {
		do {
			ball_predict_pos = BallSpeedModel::Instance()->posForTime(predict_cycle, _pVision);
			predict_cycle += increase_step;
			ball_time += increase_step*STEP_TIME;
			CGeoPoint tmpPoint = ball_predict_pos + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER, Utils::Normalize(_pVision->OurPlayer(vehicleNumber).Dir() + Param::Math::PI));
			//robot_time = expectedCMPathTime(me, tmpPoint, MaxAcceleration, MaxSpeed, CM_PREDICT_FACTOR) + STEP_TIME*Param::Latency::TOTAL_LATED_FRAME;
			robot_time = predictedTime(me, tmpPoint);
			//cout << "debug " << vehicleNumber << endl;
		} while (robot_time + robot_ahead_slack > ball_time && !Utils::OutOfField(ball_predict_pos)
			&& robot_time <= MAX_TIME && ball_time <= MAX_TIME);
	}
	else {
		robot_time = predictedTime(me, ball.Pos());
	}
	return robot_time;
}