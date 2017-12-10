#include "PenaltyDef2017V2.h"
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
	CGeoPoint ourGate;
	CGeoPoint ourGateLeft;
	CGeoPoint ourGateRight;

	const double MAX_ACC = 500;
	const double MAX_RUSH_DIST = 150;
}

CPenaltyDef2017V2::CPenaltyDef2017V2()
{
	penaltyPos = CGeoPoint(150, 0);
	ourGate = CGeoPoint(-450, 0);
	ourGateLeft = CGeoPoint(-450, -45);
	ourGateRight = CGeoPoint(-450, 45);
	_lastCycle = 0;
	_rushFlag = 0;
	_turnFlag = 0;
}

void CPenaltyDef2017V2::plan(const CVisionModule* pVision)
{
	if (pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1)
	{
		_rushFlag = 0;
		_turnFlag = 0;
	}

	int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const MobileVisionT& ball = pVision->Ball();
	const int enemyNum = BestPlayer::Instance()->getTheirBestPlayer();
	const PlayerVisionT& enemy = pVision->TheirPlayer(enemyNum);
	TaskT myTask(task());

	_rushFlag = (ourGate - me.Pos()).mod() > MAX_RUSH_DIST ? 1 : 0;

	double compenDist = 0;
	CGeoLine enemyDirLine(enemy.Pos(), enemy.Dir());
	CGeoLine gateLine(CGeoPoint(-Param::Field::PITCH_LENGTH / 2, 0), Param::Math::PI / 2);
	CGeoLineLineIntersection gateInter(enemyDirLine, gateLine);
	CVector enemy2Ball(ball.Pos().x() - enemy.Pos().x(), ball.Pos().y() - enemy.Pos().y());
	CGeoLine gate2BallLine;
	if (fabs(gateInter.IntersectPoint().y()) < 55) {
		_case = 1;
		gate2BallLine = CGeoLine(ball.Pos(), enemy.Dir());
	}
	else {
		_case = 2;
		compenDist = -(fabs(enemy.Pos().y()) < 36 ? fabs(enemy.Pos().y()) : 36) * gateInter.IntersectPoint().y() / fabs(gateInter.IntersectPoint().y());
		gate2BallLine = CGeoLine(ball.Pos(), ourGate + Utils::Polar2Vector(compenDist, -Param::Math::PI / 2));
	}

	CGeoLineLineIntersection gate2BallInter(gate2BallLine, gateLine);
	if (fabs(gate2BallInter.IntersectPoint().y()) > 150) gate2BallLine = CGeoLine(enemy.Pos(), ourGate);
	CGeoCirlce gateCircle(ourGate, MAX_RUSH_DIST);
	CGeoLineCircleIntersection taskInter(gate2BallLine, gateCircle);
	myTask.player.pos = taskInter.point1().x() > taskInter.point2().x() ? taskInter.point1() : taskInter.point2();
	myTask.player.angle = (ball.Pos() - me.Pos()).dir();
	myTask.player.flag |= PlayerStatus::QUICKLY;

	if (isBallShoot(pVision) && fabs(ball.Vel().dir() - (me.Pos() - ball.Pos()).dir()) < 0.05) {
		CGeoPoint taskPoint = taskInter.point1().x() > -450 ? taskInter.point1() : taskInter.point2();
		myTask.player.pos = ball.Vel().dir() - (me.Pos() - ball.Pos()).dir() > 0 ? CGeoPoint(me.Pos().x(), 9999) : CGeoPoint(me.Pos().x(), -9999);
		myTask.player.flag |= PlayerStatus::DO_NOT_STOP;
	}
	if ((ball.Pos() - ourGate).mod() + 20 < (me.Pos() - ball.Pos()).mod()) {
		myTask.player.pos = enemy.Pos() + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_SIZE * 3, enemy2Ball.dir());
		myTask.player.flag |= PlayerStatus::DO_NOT_STOP;
	}
	KickStatus::Instance()->setKick(robotNum, 9999);
	setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
	if (debug) cout << _case << gate2BallInter.IntersectPoint() << myTask.player.pos << endl;

	CStatedTask::plan(pVision);
	_lastCycle = pVision->Cycle();
}

CPlayerCommand* CPenaltyDef2017V2::execute(const CVisionModule* pVision)
{
	if (subTask()) {
		return subTask()->execute(pVision);
	}
	return NULL;
}

bool CPenaltyDef2017V2::isBallShoot(const CVisionModule * pVision)
{
	int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const MobileVisionT& ball = pVision->Ball();
	const int enemyNum = BestPlayer::Instance()->getTheirBestPlayer();
	const PlayerVisionT& enemy = pVision->TheirPlayer(enemyNum);

	CVector enemy2Ball(ball.Pos().x() - enemy.Pos().x(), ball.Pos().y() - enemy.Pos().y());
	bool ballDirFrontOpp = (ball.Vel().dir() - enemy2Ball.dir()) < Param::Math::PI; //角度条件
	bool ballDistFrontOpp = (enemy2Ball.mod() > 25); //距离条件
	CGeoLine enemyFaceLine(enemy.Pos(), enemy.Dir());
	CGeoLine ballVelLine(ball.Pos(), ball.Vel().dir());
	CGeoLine goalLine(CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0), Param::Math::PI / 2);
	CGeoLineLineIntersection taskInter(enemyFaceLine, goalLine);
	CGeoLineLineIntersection ballVelInter(ballVelLine, goalLine);
	bool isFaceOurDoor = fabs(taskInter.IntersectPoint().y()) < 45; //脸朝向门内
	bool isVelEnough = ball.Vel().mod() > 300 && ball.Vel().x() < 0; //速度条件

	if (debug) cout << "dist:" << ballDistFrontOpp << " dir:" << ballDirFrontOpp << " vel:" << (ball.Vel().mod() > 150) 
		<< " face:" << isFaceOurDoor << " " << (ballDistFrontOpp && ballDirFrontOpp && isVelEnough) 
		<< " " << (fabs(ball.Vel().dir() - (me.Pos() - ball.Pos()).dir()) < 0.05) << " " << ball.Vel().mod() << " ";

	return ballDistFrontOpp && ballDirFrontOpp && isVelEnough;
}
