#include "GetBallV4.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include <utils.h>
#include <DribbleStatus.h>
#include "BallSpeedModel.h"
#include <RobotSensor.h>
#include <KickStatus.h>
#include "PlayInterface.h"
#include "TaskMediator.h"
#include "CMmotion.h"

namespace {
	const double newVehicleBuffer = 0.6;               // 小嘴巴机器人PLAYER_FRONT_TO_CENTER补偿
	const double nearBallRadius = 20;                  // 小车半径+球半径+2~3视频误差裕度,判定是否需要AVOIDBALL的半径
	const double ball2myheadLimit = 3.5;                 // 小嘴巴车嘴长7cm，球半径2cm，想要稳定拿住球需要(7-2)/2=2.5cm 再加1.0的余值
	const double directGetBallDist = 35;               // 直接冲上去拿球的距离
	const double transverseBallSpeed = 20;             // 对拿球产生影响的最低横向球速 ori：30
	const double directGetBallDirLimit = Param::Math::PI / 4;
	const double getBallDist = 100;

	bool Verbose = true;                              // 调试模式

	bool needAvoidBall;
	bool canGetBall;
	bool canForwardShoot;
	bool canShoot;
	int fraredOn;
	int fraredOff;
	int FRICTION;

	const int CHASEBALL = 0;
	const int INTERBALL = 1;
	const int STATICBALL = 2;

	int MAX_CNT_FOR_TURN = 6;
	double RESPONSE_TIME = 0.3;
	int testMinFrame = 5;//最少帧数
	const int maxFrared = 100;
}

CGetBallV4::CGetBallV4()
{
	DECLARE_PARAM_READER_BEGIN(FieldParam)
		READ_PARAM(FRICTION)
	DECLARE_PARAM_READER_END
	_lastCycle = 0;
	canForwardShoot = 0;
	canShoot = 0;
	getBallMode = STATICBALL;
}

void CGetBallV4::plan(const CVisionModule* pVision)
{
	if (pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1) {
		setState(BEGINNING);
		needAvoidBall = false;
		canGetBall = false;
	}

	//////////////////////////////////////////////////////////////////////////
	//视觉初步处理
	const MobileVisionT& ball = pVision->Ball();
	const int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const double StopDist = task().player.rotvel;
	double isSensored = RobotSensor::Instance()->IsInfoValid(robotNum) && RobotSensor::Instance()->IsInfraredOn(robotNum);	//是否有检测到红外

	const CGeoPoint myHead = me.Pos() + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + newVehicleBuffer + StopDist, me.Dir());
	const CVector me2Ball = ball.Pos() - me.Pos();
	const CVector ball2Me = me.Pos() - ball.Pos();
	const CVector head2Ball = ball.Pos() - myHead;
	CGeoLine ballLine(ball.Pos(), ball.Vel().dir());
	CVector ball2Goal = CGeoPoint(450, 0) - ball.Pos();
	CVector me2Goal = CGeoPoint(450, 0) - me.Pos();
	CGeoPoint ballLineProjection = ballLine.projection(me.Pos());
	CVector ball2Projection = ballLineProjection - ball.Pos();
	double finalDir = task().player.angle;
	const double me2BallDirDiff = Utils::Normalize(me2Ball.dir() - finalDir);

	CGeoPoint ballStopPos = ball.Pos() + Utils::Polar2Vector(ball.Vel().mod2() / FRICTION, ball.Vel().dir());

	// 球在车正前方对球预测时间的影响，考虑红外
	bool frared = RobotSensor::Instance()->IsInfraredOn(robotNum);
	//带有小缓存功能的红外
	if (frared) {
		fraredOn = fraredOn >= maxFrared ? maxFrared : fraredOn + 1;
		fraredOff = 0;
	}
	else {
		fraredOn = 0;
		fraredOff = fraredOff >= maxFrared ? maxFrared : fraredOff + 1;
	}

	judgeMode(pVision);

	const CGeoLine myHeadLine = CGeoLine(myHead, Utils::Normalize(me.Dir() + Param::Math::PI / 2.0));
	const CGeoPoint ball2myheadLine_ProjPoint = myHeadLine.projection(ballStopPos);
	bool isInNearBallCircle = me.Pos().dist(ball.Pos()) < nearBallRadius;            //是否在AVOIDBALL小圈之内
	bool isInDirectGetBallCircle = me.Pos().dist(ballStopPos) < directGetBallDist;    //是否在直接冲上去拿球距离之内
	bool isGetBallDirReached = fabs(me2BallDirDiff) < directGetBallDirLimit;
	canGetBall = isInDirectGetBallCircle && isGetBallDirReached;     //重要布尔量:是否能直接上前拿球
	bool fraredGetball = fraredOn > 10;

	if (!canGetBall && me2Ball.mod() < 30 && !fraredGetball) needAvoidBall = true;
	else needAvoidBall = false;
	
	//////////////////////////////////////////////////////////////////////////
	//拿球Task初始化
	TaskT getballTask(task());
	getballTask.player.rotvel = 0.0;
	getballTask.player.flag |= PlayerStatus::DODGE_BALL;

	//////////////////////////////////////////////////////////////////////////
	//决策执行

	//getBallMode = STATICBALL;
	// chase kick
	if (getBallMode == CHASEBALL) {
		if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(160, -150), "CHASE", COLOR_CYAN);
		canShoot = canShootBall(pVision);
		canForwardShoot = judgeShootMode(pVision);

		//else {
			if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, -150), "FIXING", COLOR_CYAN);
			if (canForwardShoot) {
				if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(80, -150), "forward", COLOR_CYAN);
				getballTask.player.pos = getBallPredictPos(pVision) + Utils::Polar2Vector(20, Utils::Normalize(finalDir - Param::Math::PI));
				getballTask.player.vel = ball.Vel() + Utils::Polar2Vector(100, ball.Vel().dir());
			}
			else {
				if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(80, -150), "horizon", COLOR_CYAN);
				getballTask.player.pos = getBallPredictPos(pVision) + Utils::Polar2Vector(20, Utils::Normalize(finalDir - Param::Math::PI));
				getballTask.player.vel = ball.Vel();
				KickDirection::Instance()->GenerateShootDir(robotNum, getBallPredictPos(pVision));
			}
		//}
	}

	// inter get ball
	if (getBallMode == INTERBALL) {
		double ballArriveTime = 0;
		double meArriveTime = 9999;
		CGeoPoint testPoint(ball.Pos().x(), ball.Pos().y());
		CVector projection2Me = me.Pos() - ballLineProjection;

		double maxArriveTime = 5;//车最多移动时间
		double testBallLength = 0;//车移动距离
		double testVel = ball.Vel().mod();
		if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(160, -150), "INTER", COLOR_CYAN);
		static double lastAngle = 0;

		if (me.Pos().dist(ballLineProjection) < 23 && me2Ball.mod()<100 &&
			abs(Utils::Normalize(ball2Projection.dir()) - Utils::Normalize(ball.Vel().dir()))<0.1) {	//需要截球
			if (abs(Utils::Normalize(me2Ball.dir() - ball.Vel().dir())) > Param::Math::PI / 3 * 2)	//可以直接在投影点截球
			{
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, -150), "Waiting Ball", COLOR_CYAN);
				getballTask.player.pos = ballLineProjection;
			}
			else {																					//不能在投影点截球
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, -150), "Catching Ball", COLOR_CYAN);
				getballTask.player.pos = ballLineProjection + ball.Vel() * 2;
			}
			getballTask.player.angle = lastAngle;
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(getballTask));
			//return;
		}
		else {				//计算截球点
			do {
				ballArriveTime += testMinFrame * 1.0 / 60.0;
				testVel = ball.Vel().mod() - FRICTION * ballArriveTime;
				testBallLength = (ball.Vel().mod() + testVel) * ballArriveTime / 2;
				testPoint.setX(ball.Pos().x() + testBallLength * std::cos(ball.Vel().dir()));
				testPoint.setY(ball.Pos().y() + testBallLength * std::sin(ball.Vel().dir()));
				meArriveTime = predictedTime(me, testPoint);
			} while (IsInField(testPoint) && (meArriveTime + RESPONSE_TIME) > ballArriveTime && meArriveTime < maxArriveTime);
			CVector testpoint2Ball = ball.Pos() - testPoint;
			getballTask.player.angle = testpoint2Ball.dir();
			lastAngle = testpoint2Ball.dir();

			if (abs(Utils::Normalize(me2Ball.dir() - ball.Vel().dir())) < Param::Math::PI / 2 && me2Ball.mod() <= 40)	//避障
			{
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, -150), "Avoid ball", COLOR_CYAN);
				getballTask.player.pos = testPoint + (projection2Me / projection2Me.mod() * 30);
			}
			else
			{
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, -150), "Get ball", COLOR_CYAN);
				getballTask.player.pos = testPoint;
			}
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(getballTask));
		}
	}

	// static get ball
	if (getBallMode == STATICBALL) {
		if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(160, -150), "STATIC", COLOR_CYAN);
		static int staticDir;
		staticDir = getStaticDir(pVision, staticDir);
		if (needAvoidBall) {
			if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, -150), "AVOID BALL", COLOR_CYAN);
			if (fabs(me2BallDirDiff) > Param::Math::PI / 3) {
				if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(80, -150), "beside ball", COLOR_CYAN);
				double avoidDir = Utils::Normalize(ball2Me.dir() + staticDir * Param::Math::PI / 4);
				getballTask.player.pos = ball.Pos() + Utils::Polar2Vector(30, avoidDir);
			}
			else {
				if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(80, -150), "behind ball", COLOR_CYAN);
				double directDist = Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + newVehicleBuffer + Param::Field::BALL_SIZE + StopDist - 2.5;
				getballTask.player.pos = ball.Pos() + Utils::Polar2Vector(directDist + 5, Utils::Normalize(finalDir - Param::Math::PI));
				if (fabs(me2BallDirDiff) < 0.2)
					getballTask.player.pos = ball.Pos() + Utils::Polar2Vector(directDist, Utils::Normalize(finalDir - Param::Math::PI));
			}
		} 
		else {
			if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, -150), "GO TO", COLOR_CYAN);
			if (fabs(me2BallDirDiff) > Param::Math::PI / 2) {
				if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(80, -150), "in front of ball", COLOR_CYAN);
				double gotoDir = Utils::Normalize(finalDir + staticDir * Param::Math::PI * 3 / 5);
				getballTask.player.pos = ball.Pos() + Utils::Polar2Vector(30, gotoDir);
				getballTask.player.vel = Utils::Polar2Vector(100, (getballTask.player.pos - me.Pos()).dir());
			}
			else {
				if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(80, -150), "behind ball", COLOR_CYAN);
				double directDist = Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + newVehicleBuffer + Param::Field::BALL_SIZE + StopDist - 2.5;
				getballTask.player.pos = ball.Pos() + Utils::Polar2Vector(directDist, Utils::Normalize(finalDir - Param::Math::PI));
			}
		}
	}

	if (canShoot) {
		if (Verbose) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, 150), "SHOOT", COLOR_CYAN);
		getballTask.player.pos = getBallPredictPos(pVision);
		getballTask.player.vel = getShootVel(pVision);
	}

		// 防止守门员往球门里冲
	if (robotNum == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie())) {
		if (getballTask.player.pos.x() < -Param::Field::PITCH_LENGTH / 2 + Param::Vehicle::V2::PLAYER_SIZE)
			getballTask.player.pos.setX(-Param::Field::PITCH_LENGTH / 2 + Param::Vehicle::V2::PLAYER_SIZE);
		if (pVision->OurPlayer(robotNum).X() < -Param::Field::PITCH_LENGTH / 2 + 2) {
			getballTask.player.pos.setX(-Param::Field::PITCH_LENGTH / 2 + Param::Vehicle::V2::PLAYER_SIZE);
			getballTask.player.pos.setY(pVision->OurPlayer(robotNum).Y());
		}
	}

	if (getBallMode == STATICBALL)
		setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(getballTask));
	else setSubTask(TaskFactoryV2::Instance()->GotoPosition(getballTask));

	_lastCycle = pVision->Cycle();
	CStatedTask::plan(pVision);
}

CPlayerCommand* CGetBallV4::execute(const CVisionModule* pVision)
{
	if (subTask()) {
		return subTask()->execute(pVision);
	}

	return NULL;
}

int CGetBallV4::getStaticDir(const CVisionModule* pVision, int staticDir) {
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

void CGetBallV4::judgeMode(const CVisionModule * pVision)
{
	const MobileVisionT& ball = pVision->Ball();
	const int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	double finalDir = task().player.angle;

	CVector me2Ball = ball.Pos() - me.Pos();
	double ballVel2FInalDirDiff = Utils::Normalize(finalDir - ball.Vel().dir());
	double ball2MeDirDiff = Utils::Normalize(me.Dir() - me2Ball.dir());
	bool isBallMoving2Final = fabs(ballVel2FInalDirDiff) < Param::Math::PI * 3 / 5;
	bool meInFrontOfBall = fabs(ball2MeDirDiff) > Param::Math::PI / 2;
	bool ballMovingOurSide = ball.Vel().x() < 0;

	if (canShoot) {
		getBallMode = CHASEBALL;
		return;
	}

	if (meInFrontOfBall && ballMovingOurSide) {
		getBallMode = INTERBALL;
		return;
	}

	if (ball.Vel().mod() < 50) getBallMode = STATICBALL;
	else if (isBallMoving2Final) getBallMode = CHASEBALL;
	else getBallMode = INTERBALL;
}

bool CGetBallV4::canShootBall(const CVisionModule * pVision)
{
	CGeoPoint leftGoal = CGeoPoint(Param::Field::PITCH_LENGTH / 2, -Param::Field::GOAL_WIDTH / 2);   // 450, -50
	CGeoPoint rightGoal = CGeoPoint(Param::Field::PITCH_LENGTH / 2, Param::Field::GOAL_WIDTH / 2);   // 450, 50
	const MobileVisionT& ball = pVision->Ball();
	const int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);

	CVector ball2LeftGoal = leftGoal - ball.Pos();
	CVector ball2RightGoal = rightGoal - ball.Pos();
	CVector me2LeftGoal = leftGoal - me.Pos();
	CVector me2RightGoal = rightGoal - me.Pos();
	CVector me2Ball = ball.Pos() - me.Pos();
	CVector ball2Me = me.Pos() - ball.Pos();

	//bool meFaceGoal = me.Dir() < me2RightGoal.dir() && me.Dir() > me2LeftGoal.dir();  // 朝向球门

	double chasePrecise = 0.15;
	if (me.Pos().x() > 0) chasePrecise = ball.Vel().mod() < 80 ? 0.1 : 0.15;
	else chasePrecise = 0.1;

	bool meFaceFinalDir = fabs(Utils::Normalize(me.Dir() - task().player.angle)) < chasePrecise;  // 与task规划方向一致

	double finalDir = task().player.angle;
	double tmp2FinalDirDiff = Utils::Normalize(me2Ball.dir() - finalDir);
	bool behindBall = fabs(tmp2FinalDirDiff) < Param::Math::PI / 2;  // 在球后面
	bool inGetBallCircle = me2Ball.mod() < getBallDist;  // 离球在getball距离内
	bool para2BallVelLine = (ball.Vel().mod() && me2Ball.mod() > getBallDist / 2) ? (fabs(Utils::Normalize(me.Vel().dir() - ball.Vel().dir())) < 0.2) : 1;  // 与球速线平行

	bool canGetBall = behindBall && inGetBallCircle && para2BallVelLine;  // 可以冲上去拿球

	return /*meFaceGoal &&*/ meFaceFinalDir && canGetBall;
}

bool CGetBallV4::judgeShootMode(const CVisionModule * pVision)
{
	const MobileVisionT& ball = pVision->Ball();
	const int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	double finalDir = task().player.angle;
	double ballVel2FinalDiff = Utils::Normalize(ball.Vel().dir() - finalDir);

	bool shootMode = fabs(ballVel2FinalDiff) < 0.5;
	return shootMode;
}

CVector CGetBallV4::getShootVel(const CVisionModule * pVision)
{
	const MobileVisionT& ball = pVision->Ball();
	const int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);

	CVector finalVel = Utils::Polar2Vector(ball.Vel().mod() + 100, task().player.angle);

	return finalVel;
}

CGeoPoint CGetBallV4::getBallPredictPos(const CVisionModule * pVision)
{
	const MobileVisionT& ball = pVision->Ball();
	const int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);

	CGeoPoint finalPoint = ball.Pos();
	CVector me2Ball = ball.Pos() - me.Pos();
	bool meBehindBall = fabs(Utils::Normalize(me2Ball.dir() - ball.Vel().dir())) < Param::Math::PI * 2 / 5;
	double me2BallTime = predictedTime(me, ball.Pos());
	if (meBehindBall) {
		finalPoint = ball.Pos() + Utils::Polar2Vector(ball.Vel().mod() * me2BallTime, ball.Vel().dir());
	}
	else {
		finalPoint = ball.Pos() + Utils::Polar2Vector(ball.Vel().mod() * me2BallTime / 3, ball.Vel().dir());
	}

	return finalPoint;
}

bool CGetBallV4::IsInField(const CGeoPoint p)
{
	return (p.x() > -Param::Field::PITCH_LENGTH / 2 && p.x() < Param::Field::PITCH_LENGTH / 2 &&
		p.y() > -Param::Field::PITCH_WIDTH / 2 && p.y() < Param::Field::PITCH_WIDTH / 2);
}
