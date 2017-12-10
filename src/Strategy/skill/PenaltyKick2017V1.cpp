#include "Penaltykick2017V1.h"
#include "skill/Factory.h"
#include <CommandFactory.h>
#include <utils.h>
#include <VisionModule.h>
#include <KickStatus.h>
#include <RobotCapability.h>
#include <WorldModel.h>
#include "GDebugEngine.h"
#include "WorldDefine.h"
#include "KickDirection.h"
#include "DribbleStatus.h"
#include "RobotSensor.h"
#include "param.h"

namespace {
	bool VERBOSE_MODE = false;

	const double MOVE_TO_KICK_POINT_SPEED = 100; // 走到开球点的速度
	const double KICK_PREPARE_DIST = 15;
	double goalBuffer = 5;
	CGeoPoint ourGoal;
	CGeoPoint leftGoal;
	CGeoPoint rightGoal;
	double ROTATE_ANGLE = Param::Math::PI / 3;
	enum PenaltyKickState {
		PREPARING = 1, // 走到踢球点
		CHIPKICK = 2,
		DIRECTKICK = 3
	};
}

CPenaltyKick2017V1::CPenaltyKick2017V1()
{
	ourGoal = CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0);
	leftGoal = CGeoPoint(Param::Field::PITCH_LENGTH / 2, -Param::Field::GOAL_WIDTH / 2 + goalBuffer);
	rightGoal = CGeoPoint(Param::Field::PITCH_LENGTH / 2, Param::Field::GOAL_WIDTH / 2 - goalBuffer);
	_lastCycle = 0;
	_theirGoalie = 0;
	_targetside = 0;
	_round = 0;
	_prepared = 0;
	_canShoot = 0;
	_getball = 0;
}

void CPenaltyKick2017V1::plan(const CVisionModule* pVision)
{
	if (state() == BEGINNING || pVision->Cycle() - _lastCycle > 6) {
		setState(PREPARING);
		_round = 0;
		_prepared = 0;
		_canShoot = 0;
		_getball = 0;
		_theirGoalie = 0;
		for (int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
			if (Utils::InTheirPenaltyArea(pVision->TheirPlayer(i).Pos(), 0)) {
				_theirGoalie = i;
			}
		}
		int tempnum = rand();
		if (tempnum % 2 == 0) {
			_targetPoint = leftGoal;
			_targetside = 0;
			cout << "left kick" << endl;
		}
		else {
			_targetPoint = rightGoal;
			_targetside = 1;
			cout << "right kick" << endl;
		}
	}

	int rolenum = task().executor;
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& me = pVision->OurPlayer(rolenum);
	const PlayerVisionT& enemy = pVision->TheirPlayer(_theirGoalie);
	_canShoot = canShoot(pVision);

	if (_prepared) {
		//if (enemy.Pos().x() < 150) {
			setState(CHIPKICK);
			planChipKick(pVision);
		//}
		//else {
		//	setState(DIRECTKICK);
		//	planDirectKick(pVision);
		//}
	}
	else {
		setState(PREPARING);
		planPrepare(pVision);
	}

	++_round;
	CStatedTask::plan(pVision);
	_lastCycle = pVision->Cycle();
}

void CPenaltyKick2017V1::planPrepare(const CVisionModule * pVision)
{
	int rolenum = task().executor;
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& me = pVision->OurPlayer(rolenum);
	const PlayerVisionT& enemy = pVision->TheirPlayer(_theirGoalie);

	if (VERBOSE_MODE) cout << "preparing" << endl;
	TaskT myTask(task());
	if (_round > 10) _prepared = 1;
	CVector ball2Goal(ourGoal.x() - ball.Pos().x(), ourGoal.y() - ball.Pos().y());
	myTask.player.pos = ball.Pos() + Utils::Polar2Vector(-10, ball2Goal.dir());
	myTask.player.angle = ball2Goal.dir();
	KickStatus::Instance()->clearAll();
	setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
}

void CPenaltyKick2017V1::planChipKick(const CVisionModule * pVision)
{
	int rolenum = task().executor;
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& me = pVision->OurPlayer(rolenum);
	const PlayerVisionT& enemy = pVision->TheirPlayer(_theirGoalie);

	TaskT myTask(task());
	CVector ball2Goal(ourGoal.x() - ball.Pos().x(), ourGoal.y() - ball.Pos().y());
	CVector ball2Enemy(enemy.Pos().x() - ball.Pos().x(), enemy.Pos().y() - ball.Pos().y());
	if (VERBOSE_MODE) cout << ball2Enemy.mod() << " " << enemy.Pos() << " " << ball.Pos() << " " << RobotSensor::Instance()->IsInfraredOn(rolenum) << endl;
	if (ball2Enemy.mod() < 50) {
		if (VERBOSE_MODE) cout << "chip" << endl;
		myTask.player.pos = ball.Pos();
		myTask.player.angle = ball2Goal.dir();
		KickStatus::Instance()->setChipKick(rolenum, 80);
		setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
	}
	else if (ball2Enemy.mod() > 70) {
		if (VERBOSE_MODE) cout << "lightkick" << endl;
		myTask.player.pos = ball.Pos();
		myTask.player.angle = ball2Goal.dir();
		KickStatus::Instance()->setKick(rolenum, 80);
		setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
	}
	else {
		if (VERBOSE_MODE) cout << "getball" << endl;
		myTask.player.pos = ball.Pos() + Utils::Polar2Vector(-10, ball2Goal.dir());
		myTask.player.angle = ball2Goal.dir();
		myTask.player.flag |= PlayerStatus::DRIBBLING | PlayerStatus::DO_NOT_STOP;
		KickStatus::Instance()->clearAll();
		setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
	}

	if (ball.Pos().x() > enemy.Pos().x()) {
		if (VERBOSE_MODE) cout << "kick" << endl;
		myTask.player.pos = ball.Pos();
		myTask.player.angle = ball2Goal.dir();
		myTask.player.flag = PlayerStatus::DO_NOT_STOP;
		KickStatus::Instance()->setKick(rolenum, 9999);
		setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
	}
}

void CPenaltyKick2017V1::planDirectKick(const CVisionModule * pVision)
{
	int rolenum = task().executor;
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& me = pVision->OurPlayer(rolenum);
	const PlayerVisionT& enemy = pVision->TheirPlayer(_theirGoalie);
	static int tmpCnt;

	TaskT myTask(task());
	CVector ball2Goal(ourGoal.x() - ball.Pos().x(), ourGoal.y() - ball.Pos().y());
	if (ball.Pos().x() < 0) {
		if (VERBOSE_MODE) cout << "light kick" << endl;
		myTask.player.pos = ball.Pos();
		myTask.player.angle = ball2Goal.dir();
		myTask.player.flag |= PlayerStatus::QUICKLY;
		KickStatus::Instance()->setKick(rolenum, 10);
		setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
	}
	else if (ball.Pos().y() < 80) {
		if (_getball) {
			cout << "move " << ball.Pos().y() << endl;
			myTask.player.pos = CGeoPoint(150, 100);
			myTask.player.angle = ball2Goal.dir();
			myTask.player.flag |= PlayerStatus::DRIBBLING | PlayerStatus::QUICKLY;
			KickStatus::Instance()->clearAll();
			DribbleStatus::Instance()->setDribbleCommand(rolenum, 3);
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
		}
		else {
			if (VERBOSE_MODE) cout << "getball " << ball.Pos().y() << endl;
			myTask.player.pos = ball.Pos() + Utils::Polar2Vector(-8, ball2Goal.dir());
			myTask.player.angle = ball2Goal.dir();
			myTask.player.flag |= PlayerStatus::DRIBBLING | PlayerStatus::DO_NOT_STOP;
			KickStatus::Instance()->clearAll();
			DribbleStatus::Instance()->setDribbleCommand(rolenum, 3);
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
			if ((me.Pos() - ball.Pos()).mod() < 10.5) tmpCnt++;
			if (tmpCnt > 10) {
				tmpCnt = 0;
				_getball = 1;
			}
		}
	}
	else {
		myTask.player.pos = ball.Pos();
		CGeoPoint targetPoint;
		if (enemy.Vel().mod() > 10) {
			if (enemy.Pos().y() / enemy.Vel().y() > 0) targetPoint = enemy.Pos().y() > 0 ? leftGoal : rightGoal;
			else targetPoint = enemy.Pos().y() > 0 ? rightGoal : leftGoal;
		}
		myTask.player.angle = (targetPoint - ball.Pos()).dir();
		CGeoLine ballVelLine(ball.Pos(), myTask.player.angle);
		CGeoLine gateLine(ourGoal, Param::Math::PI / 2);
		CGeoLineLineIntersection gateInter(ballVelLine, gateLine);
		if (VERBOSE_MODE) cout << "kick " << gateInter.IntersectPoint() << endl;
		myTask.player.flag |= PlayerStatus::DRIBBLING;
		KickStatus::Instance()->setKick(rolenum, 9999);
		DribbleStatus::Instance()->setDribbleCommand(rolenum, 3);
		setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
	}
}

bool CPenaltyKick2017V1::canShoot(const CVisionModule* pVision)
{
	int rolenum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(rolenum);
	const MobileVisionT& ball = pVision->Ball();
	bool distAble = ball.Pos().dist2(me.Pos()) < 60;

	CGeoLine me2BallLine(me.Pos(), (ball.Pos() - me.Pos()).dir());
	CGeoLine goalLine(CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0), Param::Math::PI / 2);
	CGeoLineLineIntersection taskInter(me2BallLine, goalLine);
	bool dirAble = (fabs(taskInter.IntersectPoint().y()) < 45);

	return dirAble;
}
