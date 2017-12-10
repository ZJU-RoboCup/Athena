#include "SupportPos.h"

#include "BallSpeedModel.h"
#include "BestPlayer.h"
#include "ChipBallJudge.h"
#include "GDebugEngine.h"
#include "TaskMediator.h"
#include "WorldModel.h"
#include "RobotSensor.h"
#include "param.h"
#include "utils.h"

#include <cmath>

using namespace Param::Field;
using namespace Param::Math;
using namespace Param::Vehicle::V2;
namespace{
	enum{
		Left,
		Right
	};
	const bool _debugEngine= false;
	const bool intendedCreateFreeKick = false;
	const int defendAreaBuffer = 30;
	const double shootPower = 1000;
	const double shootSpeed = 800;

	const double vel_factor = 0.9;
	const double cal_acc = 10; //Ħ������ Copied from BallSpeedModel

	CGeoRectangle enemyDefendArea = CGeoRectangle(CGeoPoint(PITCH_LENGTH / 2, PENALTY_AREA_WIDTH / 2 + defendAreaBuffer), 
		CGeoPoint(PITCH_LENGTH / 2 - PENALTY_AREA_DEPTH- defendAreaBuffer, -PENALTY_AREA_WIDTH / 2 - defendAreaBuffer));
	CGeoSegment goalLine = CGeoSegment(CGeoPoint(PITCH_LENGTH / 2, PENALTY_AREA_WIDTH / 2), CGeoPoint(PITCH_LENGTH / 2, -PENALTY_AREA_WIDTH / 2));   //�����߶�
}

CSupportPos::CSupportPos(){
	_lastCycle = 0;
	for (int _i = 0; _i < 10; _i++){
		_defenderList[_i] = 0;
	}
	reflectDir = 0;
	reflectPos = CGeoPoint(PITCH_LENGTH / 2, 0);
	_supportPos = CGeoPoint(0, 0);
	ourLeaderNum = 0;
	ourSupporterNum = 0;
	_leaderChangeCounter = 0;
	_supporterChangeCounter = 0;
	_roleChangeCounter = 0;
	_savedCycle = 0;
	_isRoleChanged = false;
}	

void CSupportPos::generatePos(const CVisionModule* pVision,int num,int num2){
	if (pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1){
		for (int _i = 0; _i < 10; _i++){
			_defenderList[_i] = 0;
		}
		reflectDir = 0;
		reflectPos = CGeoPoint(PITCH_LENGTH / 2, 0);
		_supportPos = CGeoPoint(0, 0);
		_lastPos = CGeoPoint(1000, 1000);
		ourLeaderNum = 0;
		ourSupporterNum = 0;
		_leaderChangeCounter = 0;
		_supporterChangeCounter = 0;
		_savedCycle = pVision->Cycle();
		_roleChangeCounter = 0;
		_isRoleChanged = false;
	}
	//��Leader\Supporter��ƥ�䱣��һ��֡����ά���ܵ�ĺ㶨������ɫ��Luaֱ���Ļ����Ͳ���Ҫά��һ��֡�ˣ�
	//��Luaֱ�Ӵ�Leader�ĳ��ţ�Ȼ�����Leader�ĳ������������������ܵ㣨���������ܵ������������������λ���Լ����ϵ����ƣ�
	if (ourLeaderNum == 0){
		ourLeaderNum = num;
		_leaderChangeCounter = 0;
	}
	if (num != ourLeaderNum){
		_leaderChangeCounter++;
		_supporterChangeCounter++;
	}
	else{
		_leaderChangeCounter=0;
		_supporterChangeCounter=0;
	}
	if (_leaderChangeCounter == 1){
		ourLeaderNum = num;
		_leaderChangeCounter = 0;
		_roleChangeCounter++;
	}
	if (num2 == 0){
		if (ourSupporterNum == 0){
			if (BestPlayer::Instance()->ourFastestPlayerToBallList().at(0).num != num){
				ourSupporterNum = BestPlayer::Instance()->ourFastestPlayerToBallList().at(0).num;
			}
			else{
				ourSupporterNum = BestPlayer::Instance()->ourFastestPlayerToBallList().at(1).num;
			}
			_supporterChangeCounter = 0;
		}
		if (_supporterChangeCounter == 1){
			if (BestPlayer::Instance()->ourFastestPlayerToBallList().at(0).num != num){
				ourSupporterNum = BestPlayer::Instance()->ourFastestPlayerToBallList().at(0).num;
			}
			else{
				ourSupporterNum = BestPlayer::Instance()->ourFastestPlayerToBallList().at(1).num;
			}
			_supporterChangeCounter = 0;
		}
	}
	else
	{
		ourSupporterNum = num2;
	}
	

	PlayerVisionT ourSupporter = pVision->OurPlayer(ourSupporterNum);
	PlayerVisionT ourLeader = pVision->OurPlayer(ourLeaderNum);
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const CGeoPoint ballRawPos = ball.RawPos();
	const double BallVelDir = Utils::Normalize(ball.Vel().dir());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + PI);
	const PlayerVisionT& enemyLeader = pVision->TheirPlayer(BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num);
	const PlayerVisionT& enemyGoalie = pVision->TheirPlayer(BestPlayer::Instance()->getTheirGoalie());
	const int enemyDefenderAmount = calcEnemyDefenderAmount(pVision);
	double reflectPower;
	CGeoPoint defaultPos = CGeoPoint(ourLeader.Pos().x(), -ourLeader.Pos().y());  //�Է��޷��س�ʱ��Ĭ����������Leader�Գ�
	CGeoCirlce bestZone;//��ѽ�������
	CGeoPoint  bestPoint;//��ѽ�����
	bool isNeedStay = false;//�Ƿ���Ҫά��֮ǰ��������
	if ( enemyDefenderAmount!= 0&&isBallInOurControl(pVision)){
		if (calcBallReflectDirAndPos(pVision, calcEnemyDefenderAmount(pVision)) != 0){
			reflectPower = calcBallReflectPower(pVision, ourLeader.Pos(), reflectPos);
			bestZone  = calcTheBestZone(pVision,1.5*reflectPower);
			bestPoint = calcTheBestPoint(pVision, bestZone);
			if (_supportPos == CGeoPoint(0, 0)){
				_supportPos = bestPoint;
			}
			if (!BallStatus::Instance()->IsBallKickedOut()&&(bestPoint-CGeoPoint(PITCH_LENGTH/2,0)).mod()>100){
				_supportPos = bestPoint;
			}
		}
	}
	else{
		_supportPos = defaultPos;
	}
	checkAvoidShootLine(pVision);
	checkAvoidCenterLine(pVision, 130, 30);
	

	if (_lastPos == CGeoPoint(1000, 1000)){
		_lastPos = _supportPos;
	}
	if (!ball.Valid() || BallStatus::Instance()->IsBallKickedOut()){
		_supportPos = _lastPos;
	}
	_lastPos = _supportPos;
	//Debug��Ϣ
	if (_debugEngine){
		//GDebugEngine::Instance()->gui_debug_msg(ourLeader.Pos()+Utils::Polar2Vector(PLAYER_SIZE/2,0), "LLLLL");
		//GDebugEngine::Instance()->gui_debug_msg(ourSupporter.Pos() + Utils::Polar2Vector(PLAYER_SIZE / 2, 0), "SSSSS");
		//GDebugEngine::Instance()->gui_debug_arc(bestZone.Center(), bestZone.Radius(), 0, 360, 1);
		GDebugEngine::Instance()->gui_debug_arc(_supportPos, 10, 0, 360, 1);
		//GDebugEngine::Instance()->gui_debug_x(_supportPos);
	}

}

CGeoPoint CSupportPos::getSupPosNew(const CVisionModule* pVision,int num1,int num2){
	if (advancePassed == true)
		return _supportPos;
	if (pVision->Cycle() == _lastCycle &&_lastCycle != 0 ) {
		return _supportPos;
	}
	else{
		_lastCycle = pVision->Cycle();
	}
	//cout<<"debug:"<<num2<<endl;
	generatePos(pVision,num1,num2);
	//cout << "real:"<<_supportPos.x() << "   " << _supportPos.y() << endl;
	return _supportPos;
}

CGeoPoint CSupportPos::getSupPos(const CVisionModule* pVision,int num){
	if (advancePassed == true)
		return _supportPos;
	if (pVision->Cycle() == _lastCycle &&_lastCycle != 0 ) {
		return _supportPos;
	}
	else{
		_lastCycle = pVision->Cycle();
	}
	generatePos(pVision,num);
	return _supportPos;
}


//�ж����Ƿ����ҷ��Ŀ�����
bool CSupportPos::isBallInOurControl(const CVisionModule* pVision){
	PlayerVisionT ourLeader = pVision->OurPlayer(ourLeaderNum);
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const CGeoPoint ballRawPos = ball.RawPos();
	const double BallVelDir = Utils::Normalize(ball.Vel().dir());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + PI);
	const PlayerVisionT& enemyLeader = pVision->TheirPlayer(BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num);
	const PlayerVisionT& enemyGoalie = pVision->TheirPlayer(BestPlayer::Instance()->getTheirGoalie());
	const bool isSensored = RobotSensor::Instance()->IsInfoValid(ourLeaderNum) && RobotSensor::Instance()->IsInfraredOn(ourLeaderNum);	//If Infrared on 
	const bool isHasBallInVision = (ball.Pos() - ourLeader.Pos()).mod() < PLAYER_SIZE + 3 && fabs((ball.Pos() - ourLeader.Pos()).dir() - ourLeader.Dir()) < PI * 6 / 180;
	const bool isUnderControlled = (ourLeader.Pos() - ball.Pos()).mod() < (enemyLeader.Pos() - ball.Pos()).mod();
	return isSensored || isHasBallInVision || isUnderControlled;
}



//ͳ���ڽ��������ķ��س���������
int CSupportPos::calcEnemyDefenderAmount(const CVisionModule* pVision){
	PlayerVisionT ourLeader = pVision->OurPlayer(ourLeaderNum);
	int enemyDefenderAmount=0;
	int enemyCount = 0;
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const CGeoPoint ballRawPos = ball.RawPos();
	const double BallVelDir = Utils::Normalize(ball.Vel().dir());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + PI);
	const int enemyAmount =VisionModule::Instance()->getTheirValidNum(); //Size���� ���б�Ķ�Ӧ��ϵ��ҪУ��
	if (enemyAmount == 0){
		return 0;
	}
	if (BestPlayer::Instance()->getTheirGoalie() != 0){
		_defenderList[enemyCount] = BestPlayer::Instance()->getTheirGoalie();
		enemyCount++;
		enemyDefenderAmount++;
	}
	for (int _i = 0; _i < enemyAmount; _i++){
		if (enemyDefendArea.HasPoint(pVision->TheirPlayer(BestPlayer::Instance()->theirFastestPlayerToBallList().at(_i).num).Pos()) 
			&& BestPlayer::Instance()->theirFastestPlayerToBallList().at(_i).num != BestPlayer::Instance()->getTheirGoalie()){
			enemyDefenderAmount++;
			_defenderList[enemyCount] = BestPlayer::Instance()->theirFastestPlayerToBallList().at(_i).num;
			enemyCount++;
		}
	}
	return enemyDefenderAmount;
}


//���㷴���Ƕ��뷴����,���ҽ����Է����ڷ�����Աʱ����������Ա)
//�޷��ض�Աʱ������0,����ɹ�����1������ʧ�ܷ���2
//�������洢��reflectPos��reflectDir��
double CSupportPos::calcBallReflectDirAndPos(const CVisionModule* pVision, const int enemyDefenderAmount){
	PlayerVisionT ourLeader = pVision->OurPlayer(ourLeaderNum);
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const CGeoPoint ballRawPos = ball.RawPos();
	const double BallVelDir = Utils::Normalize(ball.Vel().dir());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + PI);
	const PlayerVisionT& enemyLeader = pVision->TheirPlayer(BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num);
	const PlayerVisionT& enemyGoalie = pVision->TheirPlayer(BestPlayer::Instance()->getTheirGoalie());
	int currentDefender=0;//��ǰ����������
	int effectDefender = 0;//ʵ����Ч������(һ��ʵ�ʱ����У�һ�����Ž���һ�����س�ʵ������赲��
	bool isAnyDefenderEffect = false;
	CGeoPoint TreflectPos;  //Temp ��ʱ����
	CGeoCirlce defenderCircle;
	CGeoSegment defenderFaceSeg;
	double defenderFaceDir;
	double defaultDir;
	double finalDir = 0;
	double minDist = PITCH_LENGTH;

	finalDir = (CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0) - ourLeader.Pos()).dir();
	static bool isInBack = false;
	
	if (ourLeader.Pos().x()<-150){
		isInBack = true;
	}
	if (ourLeader.Pos().x()>50){
		isInBack = false;
	}

	double antiFinalDir = Utils::Normalize(finalDir + PI);
	CGeoLine shootLine = CGeoLine(pVision->OurPlayer(ourLeaderNum).Pos(), finalDir);

	
	//ֻ��һ�����ض�Ա��Ĭ�ϴ˷��ض�Ա��Ϊ����Ա,������Ա������ǰ
	if (enemyDefenderAmount == 0){
		GDebugEngine::Instance()->gui_debug_line(CGeoPoint(PITCH_LENGTH / 2, PENALTY_AREA_WIDTH / 2), CGeoPoint(PITCH_LENGTH / 2, -PENALTY_AREA_WIDTH / 2),2);
		CGeoLineLineIntersection inter = CGeoLineLineIntersection(shootLine, goalLine);
		reflectPos = inter.IntersectPoint();
		reflectDir = PI - finalDir; 
	}

	//�������ض�Ա�����ҵ�һ����ײ��,������ײ�㣬����ΪĬ�������ź��򴦷�����ײ��
	else{
		for (int _i = 0; _i < enemyDefenderAmount; _i++){
			currentDefender = _defenderList[_i];
			defenderFaceDir = pVision->TheirPlayer(currentDefender).Dir();
			defaultDir = (pVision->TheirPlayer(currentDefender).Pos() - CGeoPoint(PITCH_LENGTH / 2, 0)).dir();
			if (fabs(defenderFaceDir-defaultDir)>PI*30/180){
				defenderFaceDir = defaultDir;  //�����س��ĳ����������ʱ��Ĭ�ϳ���Ϊ��������
			}
			defenderCircle = CGeoCirlce(pVision->TheirPlayer(currentDefender).Pos(), PLAYER_SIZE);
			CGeoLineCircleIntersection inter = CGeoLineCircleIntersection(shootLine, defenderCircle);
			//���ڻ�����һ��Ϊ����ȱ�ڵ�Բ����������ʵ������ԣ�����һ��ᱻ���س����������赲���˴�����ײ��ͳһ��������Բ����������С������ƽ�е��߶��ϣ�����ʵ��ײ�����ᳬ��3cm����ʵ���ܵ�Ĺ����п��Ժ���
			if (inter.intersectant()){
				isAnyDefenderEffect = true;
				defenderFaceSeg = CGeoSegment((pVision->TheirPlayer(currentDefender).Pos() + Utils::Polar2Vector(PLAYER_SIZE, defenderFaceDir) + Utils::Polar2Vector(PLAYER_SIZE, defenderFaceDir + PI / 2)),
					(pVision->TheirPlayer(currentDefender).Pos() + Utils::Polar2Vector(PLAYER_SIZE, defenderFaceDir) + Utils::Polar2Vector(PLAYER_SIZE, defenderFaceDir - PI / 2)));
				if (_debugEngine){
					GDebugEngine::Instance()->gui_debug_line((pVision->TheirPlayer(currentDefender).Pos() + Utils::Polar2Vector(PLAYER_SIZE, antiFinalDir) + Utils::Polar2Vector(100, defenderFaceDir + PI / 2)),
						(pVision->TheirPlayer(currentDefender).Pos() + Utils::Polar2Vector(PLAYER_SIZE, defenderFaceDir) + Utils::Polar2Vector(100, defenderFaceDir - PI / 2)), 2);
				}
				CGeoLineLineIntersection inter2 = CGeoLineLineIntersection(shootLine, defenderFaceSeg);
				TreflectPos = inter2.IntersectPoint();
				if ((TreflectPos - ourLeader.Pos()).mod() < minDist){
					effectDefender = currentDefender;
					minDist = (TreflectPos - ourLeader.Pos()).mod();
					reflectPos = TreflectPos;
					reflectDir = antiFinalDir + 2 * (defenderFaceDir - antiFinalDir);  //To Check
				}
			}
		}
		if (!isAnyDefenderEffect){
			CGeoLineLineIntersection inter = CGeoLineLineIntersection(shootLine, goalLine);
			reflectPos = inter.IntersectPoint();
			reflectDir = PI - finalDir; 
		}
	}
	if (fabs(reflectDir - antiFinalDir) < PI * 30 / 180){
		if (reflectDir >= antiFinalDir){
			reflectDir = antiFinalDir + PI * 30 / 180;
		}
		else{
			reflectDir = antiFinalDir - PI * 30 / 180;
		}
	}
	if (_debugEngine){
		GDebugEngine::Instance()->gui_debug_line(ourLeader.Pos(), ourLeader.Pos() + Utils::Polar2Vector(1000, finalDir));
		GDebugEngine::Instance()->gui_debug_x(reflectPos);
		GDebugEngine::Instance()->gui_debug_line(reflectPos, reflectPos + Utils::Polar2Vector(800, reflectDir));
	}
	return 1;
}



//���㷴������,�������ŵ㣬�Լ���������㷴������(���Ƚ��÷������������������ӳ������ȣ�û�б�Ҫ̫׼ȷ��
//��Ҫʵ�ز���
//�˴�������ײ����ʧ
//��������ģ�ͣ�Copied from BallSpeedModel
double CSupportPos::calcBallReflectPower(const CVisionModule *pVision,CGeoPoint fisrtPos, CGeoPoint secondPos){
	double dist = (fisrtPos - secondPos).mod();
	double power = (max(shootSpeed - 2.5*dist - 100, 0) + 50) / 2;
	if (power < 100){
		power = 100;
	}
	else if (power>150){
		power = 150;
	}
	return power;
}


//������ѷ���,���ݷ�������ȷ����ͬʱ�������Ȼ�Է����뾶����Ӱ��
//To Check
CGeoCirlce CSupportPos::calcTheBestZone(const CVisionModule *pVision, const double power){
	CGeoPoint centerPos = reflectPos + Utils::Polar2Vector(power, reflectDir);
	if (_debugEngine)
		GDebugEngine::Instance()->gui_debug_arc(centerPos, (1000 - power) / 12, 0, 360, 1);
	return CGeoCirlce(centerPos, (1000 - power) / 12);
}




//������ѵ�
//��ѵ����������������ڣ������������⣬�������������ķ�Χ���������µ�
CGeoPoint CSupportPos::calcTheBestPoint(const CVisionModule *pVision,const CGeoCirlce bestZone){
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const CGeoPoint ballRawPos = ball.RawPos();
	const double BallVelDir = Utils::Normalize(ball.Vel().dir());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + PI);
	const PlayerVisionT& enemyLeader = pVision->TheirPlayer(BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num);
	const PlayerVisionT& enemyGoalie = pVision->TheirPlayer(BestPlayer::Instance()->getTheirGoalie());
	const bool isEnemyExist = BestPlayer::Instance()->theirFastestPlayerToBallList().size();
	CGeoPoint targetPos = bestZone.Center();
	double projDist = 0;
	double targetDir = (CGeoPoint(PITCH_LENGTH / 2, 0) - targetPos).dir();
	CGeoSegment targetSeg = CGeoSegment(CGeoPoint(PITCH_LENGTH / 2, 0), targetPos);
	CGeoLine targetLine = CGeoLine(CGeoPoint(PITCH_LENGTH / 2, 0), targetPos);
	int currentDefender = 0;
	if (isEnemyExist){
		for (int _i = 0; _i < BestPlayer::Instance()->theirFastestPlayerToBallList().size(); _i++){
			currentDefender=BestPlayer::Instance()->theirFastestPlayerToBallList().at(_i).num;
			if ((pVision->TheirPlayer(currentDefender).Pos() - bestZone.Center()).mod() < bestZone.Radius()&&
				targetSeg.IsPointOnLineOnSegment(targetLine.projection(pVision->TheirPlayer(currentDefender).Pos()))){
				projDist= (targetLine.projection(pVision->TheirPlayer(currentDefender).Pos())-targetPos).mod();
				targetPos = targetPos + Utils::Polar2Vector(projDist + PLAYER_SIZE, targetDir);
				if (!bestZone.HasPoint(targetPos)){
					targetPos = bestZone.Center() + Utils::Polar2Vector(bestZone.Radius(), targetDir);
					return targetPos;
				}
				targetDir = (CGeoPoint(PITCH_LENGTH / 2, 0) - targetPos).dir();
				targetSeg = CGeoSegment(CGeoPoint(PITCH_LENGTH / 2, 0), targetPos);
				targetLine = CGeoLine(CGeoPoint(PITCH_LENGTH / 2, 0), targetPos);
			}
			
		}
	}
	return targetPos;
}

//�ܿ�����·��
void CSupportPos::checkAvoidShootLine(const CVisionModule *pVision){
	PlayerVisionT ourSupporter = pVision->OurPlayer(ourSupporterNum);
	PlayerVisionT ourLeader = pVision->OurPlayer(ourLeaderNum);
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const CGeoPoint ballRawPos = ball.RawPos();
	const double BallVelDir = Utils::Normalize(ball.Vel().dir());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + PI);
	const PlayerVisionT& enemyLeader = pVision->TheirPlayer(BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num);
	const PlayerVisionT& enemyGoalie = pVision->TheirPlayer(BestPlayer::Instance()->getTheirGoalie());
	double finalDir = 0;
	double changeDir = 0;
	bool needDetourToAvoid = false;
	CGeoPoint leaderDeviantPos = CGeoPoint(0, 0);
	if (fabs(ourLeader.Dir() - finalDir) < PI * 7 / 180 && (ourLeader.Pos() - ballPos).mod()<30 || ourLeader.Pos().x()>70){
		needDetourToAvoid = true;
	}
	else{
		needDetourToAvoid = false;
	}
	needDetourToAvoid = true;
	finalDir = (CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0) - ourLeader.Pos()).dir();
	
	double antiFinalDir = Utils::Normalize(finalDir + PI);
	CGeoLine shootLine = CGeoLine(ourLeader.Pos(), finalDir);
	CGeoSegment supportSeg = CGeoSegment(ourSupporter.Pos(), _supportPos);
	CGeoSegment shootSeg = CGeoSegment(ourLeader.Pos(), ourLeader.Pos() + Utils::Polar2Vector(1000, finalDir));
	CGeoLineLineIntersection shootInter = CGeoLineLineIntersection(supportSeg, shootLine);
	CGeoPoint interPoint;
	if (shootInter.Intersectant()){
		interPoint = shootInter.IntersectPoint();
	}
	bool isInter = shootSeg.IsPointOnLineOnSegment(interPoint);
	if (isInter){
	//	cout < "1";
	}
	CGeoPoint projPoint = shootLine.projection(_supportPos);
	CGeoPoint projPoint2 = shootLine.projection(ourSupporter.Pos());
	double projDist = (_supportPos - projPoint).mod();
	if (projDist < PLAYER_SIZE * 5){
		if (shootSeg.IsPointOnLineOnSegment(_supportPos)){
			_supportPos = projPoint + Utils::Polar2Vector(PLAYER_SIZE * 5 + 10, finalDir + PI / 2);
		}
		else{
			changeDir = (projPoint - _supportPos).dir();
			_supportPos = projPoint + Utils::Polar2Vector(PLAYER_SIZE * 5 + 10, changeDir);
		}
	}
	if (needDetourToAvoid && ((_supportPos - ourLeader.Pos()).dir() - finalDir)*((ourSupporter.Pos() - ourLeader.Pos()).dir()-finalDir) < 0
		 &&((shootSeg.IsPointOnLineOnSegment(projPoint2)||!shootSeg.IsPointOnLineOnSegment(projPoint2)&&(projPoint2-ourLeader.Pos()).mod()<PLAYER_SIZE*2)||isInter)){
		if ((ourSupporter.Pos() - ourLeader.Pos()).dir() - finalDir > 0){
			leaderDeviantPos = ourLeader.Pos() + Utils::Polar2Vector(PLAYER_SIZE * 7, finalDir + PI / 2);
		}
		else{
			leaderDeviantPos = ourLeader.Pos() + Utils::Polar2Vector(PLAYER_SIZE * 7, finalDir - PI / 2);
		}
		_supportPos = leaderDeviantPos + Utils::Polar2Vector(PLAYER_SIZE * 15, antiFinalDir);
	}
}

void CSupportPos::keepLeastDistanceFromLeader(const CVisionModule *pVision,double leastDist){
	PlayerVisionT ourSupporter = pVision->OurPlayer(ourSupporterNum);
	PlayerVisionT ourLeader = pVision->OurPlayer(ourLeaderNum);
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const CGeoPoint ballRawPos = ball.RawPos();
	const double BallVelDir = Utils::Normalize(ball.Vel().dir());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + PI);
	const PlayerVisionT& enemyLeader = pVision->TheirPlayer(BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num);
	const PlayerVisionT& enemyGoalie = pVision->TheirPlayer(BestPlayer::Instance()->getTheirGoalie());
	//double changeDir = (ourSupporter.Pos()-ourLeader.Pos()).dir();
	double changeDir = (_supportPos - ball.Pos()).dir();
	if ((_supportPos - ball.Pos()).mod() < leastDist){
		changeDir = (_supportPos - ball.Pos()).dir();
		_supportPos = ball.Pos() + Utils::Polar2Vector(leastDist+20, changeDir);
	}
}


void CSupportPos::checkAvoidCenterLine(const CVisionModule *pVision, double radius,double buffer){
	PlayerVisionT ourSupporter = pVision->OurPlayer(ourSupporterNum);
	PlayerVisionT ourLeader = pVision->OurPlayer(ourLeaderNum);
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const CGeoPoint ballRawPos = ball.RawPos();
	const double BallVelDir = Utils::Normalize(ball.Vel().dir());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + PI);
	const PlayerVisionT& enemyLeader = pVision->TheirPlayer(BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num);
	const PlayerVisionT& enemyGoalie = pVision->TheirPlayer(BestPlayer::Instance()->getTheirGoalie());
	static int ourLeaderInLeftOrRight = ourLeader.Pos().y() < 0 ? Left : Right;
	if (ourLeader.Pos().y()>buffer){
		ourLeaderInLeftOrRight = Right;
	}
	else if(ourLeader.Pos().y() < -buffer){
		ourLeaderInLeftOrRight = Left;
	}
	if (fabs(ourLeader.Pos().y()) < radius ||ourLeader.Pos().y()*_supportPos.y()>0 ||ourLeader.Pos().y()*ourSupporter.Pos().y()>0||ball.Vel().mod()<50){
		if (ourLeaderInLeftOrRight==Right){
			_supportPos.setY(-radius - buffer);
		}
		else{
			_supportPos.setY(radius + buffer);
		}
	}
}