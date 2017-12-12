#include "InterceptBallV6.h"
#include "skill/Factory.h"
#include <CMmotion.h>
#include <tinyxml/ParamReader.h>
#include <iostream>
#include "KickStatus.h"
#include "DribbleStatus.h"
#include <RobotSensor.h>
std::string CParamReader::_paramFileName = "zeus2005";
namespace {
	int MAX_CNT_FOR_TURN = 6;
	double RESPONSE_TIME = 0.4;
	double FRICTION;
}

CInterceptBallV6::CInterceptBallV6():waitPoint(0, 0)
{
	DECLARE_PARAM_READER_BEGIN(FieldParam)
	READ_PARAM(FRICTION)
	DECLARE_PARAM_READER_END
}


CInterceptBallV6::~CInterceptBallV6()
{
}

void CInterceptBallV6::plan(const CVisionModule * pVision)
{
	const MobileVisionT& ball = pVision->Ball();
	const int robotNum = task().executor;
	const double finalAngel = task().player.angle;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const int playerFlag = task().player.flag;


	CGeoLine ballLine(ball.RawPos(), ball.Vel().dir());
	TaskT chase_kick_task(task());
	double ballArriveTime = 0;
	double meArriveTime = 9999;
	int testMinFrame = 5;//最少帧数
	CGeoPoint testPoint(ball.RawPos().x(), ball.RawPos().y());
	CVector me2Ball = ball.RawPos() - me.RawPos();
	CGeoPoint ballLineProjection = ballLine.projection(me.RawPos());
	CVector ball2Projection = ballLineProjection - ball.RawPos();
	CVector projection2Me = me.RawPos() - ballLineProjection;


	double isSensored = RobotSensor::Instance()->IsInfoValid(robotNum) && RobotSensor::Instance()->IsInfraredOn(robotNum);	//是否有检测到红外
	double maxArriveTime = 5;//车最多移动时间
	double testBallLength = 0;//车移动距离
	double ballAcc = FRICTION;//球减速度
	double testVel = ball.Vel().mod();


	if (me2Ball.mod()<100) {					//距车1m，发送吸球指令
		DribbleStatus::Instance()->setDribbleCommand(robotNum, 3);
	}
	/*
	if (me2Ball.mod() < 15) {
		cnt++;
		needDribble = true;
	}
	else {
		needDribble = false;
		cnt = 0;
	}*/
	if (me2Ball.mod() < 11 && abs(Utils::Normalize(ball.Vel().dir() - me2Ball.dir())) < Param::Math::PI / 4) {//已经吸球
		if (abs(me.Dir() - finalAngel) < 3 * Param::Math::PI / 180) {			//need shoot  ball2Goal.dir())
			cnt = 0;
			needDribble = false;
			//For kick status
			//KickStatus::Instance()->setKick(robotNum, power);
		}
		else {												//need turn off
			int  CLOCKWISE = -1;	//-1 顺时针
			if (Utils::Normalize(me.Dir()) < Utils::Normalize(finalAngel) && Utils::Normalize(me.Dir()) + Param::Math::PI > Utils::Normalize(finalAngel))
				CLOCKWISE = 1;
			double newdir = me.Dir() + CLOCKWISE * Param::Math::PI / 10;
			chase_kick_task.player.pos = CGeoPoint(ball.RawPos().x() - me2Ball.mod()*std::cos(newdir), ball.RawPos().y() - me2Ball.mod()*std::sin(newdir));
			double omega;
			if (abs(me.Dir() - finalAngel) < Param::Math::PI / 6) {
				omega = Param::Math::PI / 3 * 2;
				chase_kick_task.player.speed_x = 30;
			}
			else {
				omega = Param::Math::PI*1.5;
				chase_kick_task.player.speed_x = 40;
			}
			chase_kick_task.player.speed_y = -CLOCKWISE*omega*me2Ball.mod();
			chase_kick_task.player.rotate_speed = CLOCKWISE*omega;
			setSubTask(TaskFactoryV2::Instance()->OpenSpeed(chase_kick_task));
		}
	}
	else if (abs(ball.Vel().mod()) < 30) {				//球近似静止
			if (abs(Utils::Normalize(me2Ball.dir() - me.Dir())) > Param::Math::PI / 3 * 2 && me2Ball.mod() < 30) {
				chase_kick_task.player.pos = ball.RawPos() + Utils::Polar2Vector(30, me2Ball.dir() + Param::Math::PI / 2);
			}
			else {
				chase_kick_task.player.pos = ball.RawPos();
			}
			chase_kick_task.player.angle = me2Ball.dir();
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(chase_kick_task));
		}
		else if (me.RawPos().dist(ballLineProjection) < 23 && me2Ball.mod()<100 &&
		abs(Utils::Normalize(ball2Projection.dir()-ball.Vel().dir()))<0.1) {
			if (abs(Utils::Normalize(me2Ball.dir() - ball.Vel().dir())) > Param::Math::PI / 3 * 2)
			{
				chase_kick_task.player.pos = ballLineProjection;
			}
			else
			{
				chase_kick_task.player.pos = ballLineProjection + ball.Vel() * 2.5;
			}
			chase_kick_task.player.angle =  me2Ball.dir();
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(chase_kick_task));
		}
		else {							//计算接球点
			do {
				ballArriveTime += testMinFrame * 1.0 / 60.0;
				testVel = ball.Vel().mod() - ballAcc*ballArriveTime;
				testBallLength = (ball.Vel().mod() + testVel)*ballArriveTime / 2;//(ball.Vel().mod2() - testVel*testVel) / (2 * ballAcc);
				testPoint.setX(ball.RawPos().x() + testBallLength * std::cos(ball.Vel().dir()));
				testPoint.setY(ball.RawPos().y() + testBallLength * std::sin(ball.Vel().dir()));
				meArriveTime = predictedTime(me, testPoint);
			} while (IsInField(testPoint) && (meArriveTime + RESPONSE_TIME) > ballArriveTime && meArriveTime < maxArriveTime);


			CVector testpoint2Ball = ball.RawPos() - testPoint;//CGeoPoint(450, 0)- testPoint;
			chase_kick_task.player.angle = testpoint2Ball.dir();
			lastAngle = testpoint2Ball.dir();

			if (abs(Utils::Normalize(me2Ball.dir() - ball.Vel().dir())) < Param::Math::PI / 2 && me2Ball.mod() <= 40)
			{
				chase_kick_task.player.pos = testPoint + (projection2Me / projection2Me.mod() * 30);
			}
			else
			{
				chase_kick_task.player.pos = testPoint;
			}
			setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(chase_kick_task));
		}
	CStatedTask::plan(pVision);
	return;
}


CPlayerCommand* CInterceptBallV6::execute(const CVisionModule* pVision)
{
	if (subTask()) {
		return subTask()->execute(pVision);
	}
	return NULL;
}

bool CInterceptBallV6::IsInField(const CGeoPoint p)
{
	return (p.x() > -Param::Field::PITCH_LENGTH / 2 && p.x() < Param::Field::PITCH_LENGTH / 2 &&
		p.y() > -Param::Field::PITCH_WIDTH / 2 && p.y() < Param::Field::PITCH_WIDTH / 2);
}
