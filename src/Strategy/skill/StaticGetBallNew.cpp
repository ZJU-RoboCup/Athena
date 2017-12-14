#include "StaticGetBallNew.h"
#include "skill/Factory.h"
#include "DribbleStatus.h"
#include "GDebugEngine.h"

namespace {
	bool Verbose = TRUE;
	const double newVehicleBuffer = 0.6;
	const double directGetBallDist = 35;
	const double StopDist = 12;
	const double directGetBallDirLimit = Param::Math::PI / 4;
	bool needAvoidBall;
}
CStaticGetBallNew::CStaticGetBallNew()
{
	canGetBall = false;
}

CStaticGetBallNew::~CStaticGetBallNew()
{
}

void CStaticGetBallNew::plan(const CVisionModule * pVision)
{
	const MobileVisionT& ball = pVision->Ball();
	const int robotNum = task().executor;
	const double finalAngel = task().player.angle;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const int playerFlag = task().player.flag;
	

	TaskT static_get_task(task());
	CVector me2Ball = ball.Pos() - me.Pos();
	CVector ball2Me = me.Pos() - ball.Pos();
	CGeoPoint ballStopPos = ball.Pos();
	const double me2BallDirDiff = Utils::Normalize(me2Ball.dir() - finalAngel);
	bool isInDirectGetBallCircle = me.Pos().dist(ballStopPos) < directGetBallDist;    //是否在直接冲上去拿球距离之内
	bool isGetBallDirReached = fabs(me2BallDirDiff) < directGetBallDirLimit;
	canGetBall = isInDirectGetBallCircle && isGetBallDirReached;     //重要布尔量:是否能直接上前拿球
  
	if (me2Ball.mod()<50) {					//距车0.5m，发送吸球指令
		DribbleStatus::Instance()->setDribbleCommand(robotNum, 3);
	}
	if (!canGetBall && me2Ball.mod() < 30 ) needAvoidBall = true;
	else needAvoidBall = false;
	
	if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(160, -150), "STATIC", COLOR_CYAN);
	static int staticDir;
	staticDir = getStaticDir(pVision, staticDir);
	if (needAvoidBall) {
		if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, -150), "AVOID BALL", COLOR_CYAN);
		if (fabs(me2BallDirDiff) > Param::Math::PI / 3) {
			if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(80, -150), "beside ball", COLOR_CYAN);
			double avoidDir = Utils::Normalize(ball2Me.dir() + staticDir * Param::Math::PI / 4);
			static_get_task.player.pos = ball.Pos() + Utils::Polar2Vector(30, avoidDir);
		}
		else {
			if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(80, -150), "behind ball", COLOR_CYAN);
			double directDist = Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + newVehicleBuffer + Param::Field::BALL_SIZE + StopDist - 2.5;
			static_get_task.player.pos = ball.Pos() + Utils::Polar2Vector(directDist, Utils::Normalize(finalAngel - Param::Math::PI));
			if (fabs(me2BallDirDiff) < 0.2)
				static_get_task.player.pos = ball.Pos() + Utils::Polar2Vector(directDist, Utils::Normalize(finalAngel - Param::Math::PI));
		}
	}
	else {
		if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, -150), "GO TO", COLOR_CYAN);
		if (fabs(me2BallDirDiff) > Param::Math::PI / 2) {
			if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(80, -150), "in front of ball", COLOR_CYAN);
			double gotoDir = Utils::Normalize(finalAngel + staticDir * Param::Math::PI * 3 / 5);
			static_get_task.player.pos = ball.Pos() + Utils::Polar2Vector(30, gotoDir);
			static_get_task.player.vel = Utils::Polar2Vector(100, (static_get_task.player.pos - me.Pos()).dir());
		}
		else {
			if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(80, -150), "behind ball", COLOR_CYAN);
			double directDist = Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + newVehicleBuffer + Param::Field::BALL_SIZE + StopDist - 2.5;
			static_get_task.player.pos = ball.Pos() + Utils::Polar2Vector(directDist, Utils::Normalize(finalAngel - Param::Math::PI));
		}
	}
	/*
	cout << "in static" << endl;
	static_get_task.player.angle = 0;

	if (me2Ball.mod() < 50) {
		if (abs(Utils::Normalize(me2Ball.dir() - finalAngel)) <= Param::Math::PI / 2) {		//下半侧
			if (me2Ball.mod() < 30 && abs(Utils::Normalize(me2Ball.dir() - finalAngel)) < Param::Math::PI / 4)
			{
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(100, 100), "DOWN DIRECT", COLOR_RED);
				static_get_task.player.pos = ball.Pos();
			}
			else {
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(100, 100), "DOWN GET 28", COLOR_RED);
				static_get_task.player.pos = ball.Pos() + Utils::Polar2Vector(28, finalAngel);
			}
		}		//上半侧
		else {
			if (abs(Utils::Normalize(me2Ball.dir() - finalAngel - Param::Math::PI / 2)) < Param::Math::PI / 2) {
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(100, 100), "UP CLOCK", COLOR_CYAN);
				static_get_task.player.pos = ball.Pos() + Utils::Polar2Vector(45, finalAngel - Param::Math::PI / 9 * 5);
			}
			else {
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(100, 100), "UP RE-CLOCK", COLOR_CYAN);
				static_get_task.player.pos = ball.Pos() + Utils::Polar2Vector(45, finalAngel + Param::Math::PI / 9 * 5);
			}
		}
	}
	else {
		GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(100, 100), "GO DIRECT", COLOR_BLUE);
		static_get_task.player.pos = ball.Pos();
	}*/

	/*
	if (me2Ball.mod() < 20) { 
		if (abs(Utils::Normalize(me2Ball.dir() - finalAngel)) < Param::Math::PI / 3) {
			GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(100, 100), "20 GET BALL", COLOR_RED);
			static_get_task.player.pos = ball.Pos();
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(static_get_task));
		}
		else {
			GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(100, 100), "20 AVOID BALL", COLOR_RED);
			static_get_task.player.pos = ball.Pos() + Utils::Polar2Vector(30, -me2Ball.dir());
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(static_get_task));
		}
	}
	else if (me2Ball.mod() < 30) {
		if (abs(Utils::Normalize(me2Ball.dir() - finalAngel)) < Param::Math::PI / 3) {
			GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(100, 100), "30 GET BALL", COLOR_CYAN);
			cout <<"30getball" << endl;
			static_get_task.player.pos = ball.Pos() + Utils::Polar2Vector(15, -finalAngel);
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(static_get_task));
		}
		else {
			GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(100, 100), "30 AVOID BALL", COLOR_CYAN);
			cout << me2Ball.mod() <<" "<< me2Ball.dir()<<" "<< -me2Ball.dir() + Param::Math::PI / 5<< endl;
			static_get_task.player.pos = ball.Pos() + Utils::Polar2Vector(25, -me2Ball.dir() + Param::Math::PI / 2);
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(static_get_task));
		}
	}
	else {
		static_get_task.player.pos = ball.Pos();
		setSubTask(TaskFactoryV2::Instance()->GotoPosition(static_get_task));
	}
	*/
	setSubTask(TaskFactoryV2::Instance()->GotoPosition(static_get_task));
	CStatedTask::plan(pVision);
	return;

}



CPlayerCommand* CStaticGetBallNew::execute(const CVisionModule* pVision)
{
	if (subTask()) {
		return subTask()->execute(pVision);
	}
	return NULL;
}

int CStaticGetBallNew::getStaticDir(const CVisionModule * pVision, int staticDir)
{
	const MobileVisionT& ball = pVision->Ball();
	const int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);

	double ball2MeDir = (me.Pos() - ball.Pos()).dir();
	double finalDir = task().player.angle;
	double tmp2FinalDirDiff = Utils::Normalize(ball2MeDir - finalDir);

	if (!staticDir) staticDir = tmp2FinalDirDiff > 0 ? 1 : -1;
	else {
		if (staticDir == 1) {
			if (tmp2FinalDirDiff < -0.5) staticDir = -1;
		}
		else if (tmp2FinalDirDiff > 0.5) staticDir = 1;
	}

	return staticDir;
}
