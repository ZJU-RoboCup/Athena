#include "PenaltyDef2017V1.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include <utils.h>
#include "WorldModel.h"
#include "BestPlayer.h"
#include "GoaliePosV1.h"
#include "KickStatus.h"
#include "AdvanceBallV2.h"
#include "ChipBallJudge.h"
#include "BallSpeedModel.h"

namespace {
	const bool debug = false;

	CGeoPoint penaltyPos;

	const double ROTVEL_LIMIT = 0.03;
	const double MAX_ACC = 500;
}

CPenaltyDef2017V1::CPenaltyDef2017V1()
{
	penaltyPos = CGeoPoint(150, 0);
	_dirDiff = 0;
	_enemyDir = Param::Math::PI;
	_lastEnemyDir = Param::Math::PI;
	_lastCycle = 0;
	_needHorizon = 0;
	_horizonType = 0;
	_isBallShoot = 0;
	_needTurnBack = 0;
}

void CPenaltyDef2017V1::plan(const CVisionModule* pVision)
{
	if (pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1)
	{
		_needHorizon = 0;
		_horizonType = 0;
		_isBallShoot = 0;
		_needTurnBack = 0;
	}

	int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const MobileVisionT& ball = pVision->Ball();
	const int enemyNum = BestPlayer::Instance()->getTheirBestPlayer();
	const PlayerVisionT& enemy = pVision->TheirPlayer(enemyNum);
	TaskT myTask(task());

	if (!_horizonType) {
		if (enemy.Pos().y() > 50 && !_isBallShoot) _horizonType = 1; //对方在中线右边（以防守者的视角
		else if (enemy.Pos().y() < -50 && !_isBallShoot) _horizonType = 2; //对方在中线左边
		else if (!_isBallShoot) _horizonType = 3; //射出时对方在中线上
		if (me.Pos().dist2(ball.Pos()) < 30000) _horizonType = 4; //到一定位置时移动幅度减小
	}
	if (!_isBallShoot) {
		_isBallShoot = isBallShoot(pVision); //判断敌方是否已经射出球
	}
	_needTurnBack = needTurnBack(pVision);
	_needHorizon = needHorizon(pVision);
	if(debug) cout << "needHorizon: " << _needHorizon << " " << _horizonType << " needTurnBack: " << _needTurnBack << " " << _turnType << " isBallShoot: " << _isBallShoot << endl;

	if (_isBallShoot && _needTurnBack) { //球在后面，需要转身
		if (debug) cout << "turn" << endl;
		KickStatus::Instance()->setKick(robotNum, 9999);
		myTask.player.pos = getTurnPoint(pVision);
		myTask.player.flag |= PlayerStatus::DO_NOT_STOP | PlayerStatus::KICK_WHEN_POSSIABLE;
		setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
	}
	else if (_isBallShoot && _needHorizon && !_needTurnBack) { //需要横移
		if (debug) cout << "horizon" << endl;
		myTask.player.pos = getHorizonPoint(pVision); //根据不同情况发点
		myTask.player.angle = CVector(ball.Pos() - me.Pos()).dir();
		myTask.player.flag |= PlayerStatus::DO_NOT_STOP;
		setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
	}
	else {
		if (debug) cout << "preparing" << endl;
		myTask.player.pos = ball.Pos();
		myTask.player.angle = CVector(ball.Pos() - me.Pos()).dir();
		myTask.player.flag |= PlayerStatus::DO_NOT_STOP;
		setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask)); //在不需要横移的情况下尽量前移
	}

	if (debug) {
		cout << "ball: " << ball.Pos().x() << " " << ball.Pos().y() << endl;
		cout << "me: " << me.Pos().x() << " " << me.Pos().y() << endl;
		cout << "task: " << myTask.player.pos.x() << " " << myTask.player.pos.y() << endl;

		cout << "Vel: " << ball.Vel().mod() << " Dir: " << ball.Vel().dir() << " " << (ball.Pos() - me.Pos()).dir() << endl;
	}
	CStatedTask::plan(pVision);
	_lastEnemyDir = _enemyDir;
	_lastCycle = pVision->Cycle();
}

CPlayerCommand* CPenaltyDef2017V1::execute(const CVisionModule* pVision)
{
	if (subTask()) {
		return subTask()->execute(pVision);
	}
	return NULL;
}

CGeoPoint CPenaltyDef2017V1::getHorizonPoint(const CVisionModule * pVision)
{
	CGeoPoint finalPoint(450, 0);
	int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const MobileVisionT& ball = pVision->Ball();
	const int enemyNum = BestPlayer::Instance()->getTheirBestPlayer();
	const PlayerVisionT& enemy = pVision->TheirPlayer(enemyNum);

	double ballTime = BallSpeedModel::Instance()->timeForDist((me.Pos() - ball.Pos()).mod(), pVision);
	double ballAngle = fabs(fabs((me.Pos() - ball.Pos()).dir()) - Param::Math::PI);
	double meAcc = (me.Pos() - ball.Pos()).mod() * tan(ballAngle) / pow(ballTime, 2);
	double forwardAcc, accRatio = 0;
	if (debug) cout << "ballTime: " << ballTime << " meAcc: " << meAcc << " ballAngle: " << ballAngle << endl;

	switch (_horizonType) {
	case 1:
		{
			if (debug) cout << "horizon 1" << endl;
			CGeoLine taskLine(CGeoPoint(-Param::Field::PITCH_LENGTH / 2, - Param::Field::GOAL_WIDTH / 2), ball.Pos());
			CGeoLine forwardLine(me.Pos(), CVector(-Param::Field::PITCH_LENGTH / 2 - ball.Pos().x(), - Param::Field::GOAL_WIDTH / 2 - ball.Pos().y()).dir() + Param::Math::PI / 2);
			CGeoLineLineIntersection taskInter(taskLine, forwardLine); //球和右门框的连线与门将的垂线
			finalPoint = taskInter.IntersectPoint();
		}
		break;
	case 2:
		{
			if (debug) cout << "horizon 2" << endl;
			CGeoLine taskLine(CGeoPoint(-Param::Field::PITCH_LENGTH / 2, Param::Field::GOAL_WIDTH / 2), ball.Pos());
			CGeoLine forwardLine(me.Pos(), CVector(-Param::Field::PITCH_LENGTH / 2 - ball.Pos().x(), Param::Field::GOAL_WIDTH / 2 - ball.Pos().y()).dir() + Param::Math::PI / 2);
			CGeoLineLineIntersection taskInter(taskLine, forwardLine); //球和左门框的连线与门将的垂线
			finalPoint = taskInter.IntersectPoint();
		}
		break;
	case 3:
		{
			if (debug) cout << "horizon 3" << endl;
			finalPoint = CGeoPoint(me.Pos().x(), 9999 * ball.Pos().y() / fabs(ball.Pos().y())); //取球的y轴坐标
		}
		break;
	case 4:
		{
			if (debug) cout << "horizon 4 " << enemy.Dir() << endl;
			finalPoint = CGeoPoint(me.Pos().x(), 300 * (enemy.Dir() / fabs(enemy.Dir())));
		}
		break;
	}
	if (meAcc > 0 && meAcc < MAX_ACC) {
		forwardAcc = sqrt(pow(MAX_ACC, 2) - pow(meAcc, 2));
		accRatio = sqrt(forwardAcc / meAcc);
		if (debug) cout << "accRatio: " << accRatio << " forwardAcc: " << forwardAcc << endl;
		double me2TaskDist = (me.Pos() - finalPoint).mod();
		double forwardDist = me2TaskDist * accRatio;
		finalPoint = CGeoPoint(finalPoint.x() + forwardDist * sin(ballAngle), finalPoint.y()); //根据轴向加速度和径向加速度的比例来发点
	}
	return finalPoint;
}

CGeoPoint CPenaltyDef2017V1::getTurnPoint(const CVisionModule * pVision)
{
	int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const MobileVisionT& ball = pVision->Ball();
	const int enemyNum = BestPlayer::Instance()->getTheirBestPlayer();
	const PlayerVisionT& enemy = pVision->TheirPlayer(enemyNum);
	CGeoPoint finalPoint;

	if (_turnType == 1) {
		if (debug) cout << "turn 1" << endl;
		CVector ball2MeVec(ball.Pos().x() - me.Pos().x(), ball.Pos().y() - me.Pos().y());
		double taskDir;
		if (ball2MeVec.dir() > 0)
			taskDir = ball2MeVec.dir() + 0.2;
		else 
			taskDir = ball2MeVec.dir() - 0.2;
		finalPoint = CGeoPoint(9999 * cos(taskDir), 9999 * sin(taskDir)); //发一个能够把球碰出界外的点
	}
	else if (_turnType == 2) {
		if (debug) cout << "turn 2" << endl;
		finalPoint = CGeoPoint(me.Pos().x() - 100, me.Pos().y()); //刹车
	}
	return finalPoint;
}

bool CPenaltyDef2017V1::needHorizon(const CVisionModule * pVision)
{
	int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const MobileVisionT& ball = pVision->Ball();
	const int enemyNum = BestPlayer::Instance()->getTheirBestPlayer();
	const PlayerVisionT& enemy = pVision->TheirPlayer(enemyNum);

	if (_horizonType == 1) {
		CVector me2BallVec(me.Pos().x() - ball.Pos().x(), me.Pos().y() - ball.Pos().y());
		CVector gate2BallVec(-Param::Field::PITCH_LENGTH / 2 - ball.Pos().x(), - Param::Field::GOAL_WIDTH / 2 - ball.Pos().y());
		if (fabs(fabs(me2BallVec.dir()) - fabs(gate2BallVec.dir())) > 0.05) return true;
	}
	else if (_horizonType == 2) {
		CVector me2BallVec(me.Pos().x() - ball.Pos().x(), me.Pos().y() - ball.Pos().y());
		CVector gate2BallVec(-Param::Field::PITCH_LENGTH / 2 - ball.Pos().x(), Param::Field::GOAL_WIDTH / 2 - ball.Pos().y());
		if (fabs(fabs(me2BallVec.dir()) - fabs(gate2BallVec.dir())) > 0.05) return true;
	}
	else {
		if (fabs(ball.Pos().y()) - fabs(me.Pos().y()) > 1) return true;
	}
	return false;
}

bool CPenaltyDef2017V1::isBallShoot(const CVisionModule * pVision)
{
	int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const MobileVisionT& ball = pVision->Ball();
	const int enemyNum = BestPlayer::Instance()->getTheirBestPlayer();
	const PlayerVisionT& enemy = pVision->TheirPlayer(enemyNum);

	CVector enemy2Ball(ball.Pos().x() - enemy.Pos().x(), ball.Pos().y() - enemy.Pos().y());
	bool ballDirFrontOpp = (ball.Vel().dir() - enemy2Ball.dir()) < Param::Math::PI; //角度条件
	bool ballDistFrontOpp = (enemy2Ball.mod() > 20); //距离条件
	CGeoLine enemyFaceLine(enemy.Pos(), enemy.Dir());
	CGeoLine goalLine(CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0), Param::Math::PI / 2);
	CGeoLineLineIntersection taskInter(enemyFaceLine, goalLine);
	bool isFaceOurDoor = fabs(taskInter.IntersectPoint().y()) < 45; //脸朝向门内
	bool isVelEnough = ball.Vel().mod() > 150; //速度条件
	if (debug) cout << "dir: " << ballDirFrontOpp << " dist: " << ballDistFrontOpp << " face: " << isFaceOurDoor << " vel: " << isVelEnough << endl;

	if (debug) cout << me.Pos().dist2(ball.Pos()) << endl;
	if (me.Pos().dist2(ball.Pos()) < 40000) {
		return isFaceOurDoor;
	}
	return ballDistFrontOpp && ballDirFrontOpp && isFaceOurDoor && isVelEnough;
}

bool CPenaltyDef2017V1::needTurnBack(const CVisionModule * pVision)
{
	int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const MobileVisionT& ball = pVision->Ball();
	if (ball.Pos().x() < me.Pos().x()) { //球在身后
		_turnType = 1;
		return true;
	}
	else if (_isBallShoot && !_needHorizon && (me.Pos() - ball.Pos()).mod() < 200) { //当球射出但不需要横移同时球还在前面时
		_turnType = 2;
		return true;
	}
	return false;
}
