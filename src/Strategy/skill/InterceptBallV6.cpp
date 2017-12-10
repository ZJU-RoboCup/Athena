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
	double RESPONSE_TIME = 0.3;
}

CInterceptBallV6::CInterceptBallV6()
{
}


CInterceptBallV6::~CInterceptBallV6()
{
}

void CInterceptBallV6::plan(const CVisionModule * pVision)
{
	double FRICTION;
	PARAM_READER->readParams();
	DECLARE_PARAM_READER_BEGIN(FieldParam)
	READ_PARAM(FRICTION)
		DECLARE_PARAM_READER_END

	const MobileVisionT& ball = pVision->Ball();
	const int robotNum = task().executor;
	const double power = 500;//task().player.kickpower;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const int playerFlag = task().player.flag;


	//CGeoSegment ballLine(CGeoPoint(ball.RawPos().x() - 9999 * ball.VelX(), ball.RawPos().y() - 9999 * ball.VelY()),
	//	CGeoPoint(ball.RawPos().x() + 9999 * ball.VelX(), ball.RawPos().y() + 9999 * ball.VelY()));
	//CGeoSegment me2Goal(me.RawPos(), CGeoPoint(450, 0));
	//bool onDiffSide = ballLine.IsSegmentsIntersect(me2Goal);//判断车、门是否在异侧
	CGeoLine ballLine(ball.RawPos(), ball.Vel().dir());
	TaskT chase_kick_task(task());
	double ballArriveTime = 0;
	double meArriveTime = 9999;
	int testMinFrame = 5;//最少帧数
	CGeoPoint testPoint(ball.RawPos().x(), ball.RawPos().y());
	CVector me2Ball = ball.RawPos() - me.RawPos();
	CVector ball2Goal = CGeoPoint(450, 0) - ball.RawPos();
	CVector me2Goal = CGeoPoint(450, 0) - me.RawPos();
	CGeoPoint ballLineProjection = ballLine.projection(me.RawPos());
	CVector ball2Projection = ballLineProjection - ball.RawPos();

	double isSensored = RobotSensor::Instance()->IsInfoValid(robotNum) && RobotSensor::Instance()->IsInfraredOn(robotNum);	//是否有检测到红外
	double maxArriveTime = 5;//车最多移动时间
	double testBallLength = 0;//车移动距离
	double ballAcc = FRICTION;//球减速度
	double testVel = ball.Vel().mod();

	/*
	double acca = 2;
	if (!needDribble) chase_kick_task.player.speed_y = 20;
	else chase_kick_task.player.rotate_speed -= acca;
	if (last_speed == 20) needDribble = true;
	if (last_speed == 0) needDribble = false;
	setSubTask(TaskFactoryV2::Instance()->OpenSpeed(chase_kick_task));
	return;
	*/
	/*
	std::cout << "sensor:" <<robotNum<<"\t"<< RobotSensor::Instance()->IsInfoValid(robotNum) << "\t" << RobotSensor::Instance()->IsInfraredOn(robotNum) << std::endl;
	chase_kick_task.player.speed_y = 1;
	setSubTask(TaskFactoryV2::Instance()->OpenSpeed(chase_kick_task));
	return;
	*/
	if (needDribble) {					//发送吸球指令
		DribbleStatus::Instance()->setDribbleCommand(robotNum, 3);
		//chase_kick_task.player.flag |= PlayerStatus::DRIBBLING;
	}
	if (me2Ball.mod() < 15) {
		cnt++;
		needDribble = true;
		//std::cout << " cnt=" << cnt << std::endl;
	}
	else {
		needDribble = false;
		cnt = 0;
	}
	if (cnt > MAX_CNT_FOR_TURN && me2Ball.mod() < 11) {//&& isSensored
		if (abs(me.Dir() - me2Goal.dir()) < Param::Math::PI / 180) {			//need shoot  ball2Goal.dir())
			cnt = 0;
			std::cout << "shoot me.dir=\t" << me2Goal.dir() << "\nball2goal\t" << ball2Goal.dir() << std::endl;
			needDribble = false;
			KickStatus::Instance()->setKick(robotNum, power);
			return;
		}
		else {												//need turn off
			//std::cout << abs(me.Dir() - ball2Goal.dir()) << endl;
			int  CLOCKWISE = -1;	//-1 顺时针
			if (Utils::Normalize(me.Dir()) < Utils::Normalize(ball2Goal.dir()) && Utils::Normalize(me.Dir()) + Param::Math::PI > Utils::Normalize(ball2Goal.dir()))
				CLOCKWISE = 1;
			double newdir = me.Dir() + CLOCKWISE * Param::Math::PI / 10;
			chase_kick_task.player.pos = CGeoPoint(ball.RawPos().x() - me2Ball.mod()*std::cos(newdir), ball.RawPos().y() - me2Ball.mod()*std::sin(newdir));
			/*CVector newpos2Ball = ball.RawPos() - chase_kick_task.player.pos;
			chase_kick_task.player.angle = newpos2Ball.dir();//me2Ball.dir();
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(chase_kick_task));*/
			double omega;
			if (abs(me.Dir() - me2Goal.dir()) < Param::Math::PI / 6) {
				omega = Param::Math::PI/2;
				chase_kick_task.player.speed_x = 20;
			}
			else {
				omega = Param::Math::PI;
				chase_kick_task.player.speed_x = 40;
			}
			chase_kick_task.player.speed_y = -CLOCKWISE*omega*me2Ball.mod();
			chase_kick_task.player.rotate_speed = CLOCKWISE*omega;
			//std::cout << "open speed!!" << std::endl;
			setSubTask(TaskFactoryV2::Instance()->OpenSpeed(chase_kick_task));
			return;
		}
	}

	if (abs(testVel) < 30) {				//球近似静止
		chase_kick_task.player.pos = ball.RawPos();
		chase_kick_task.player.angle = me2Ball.dir();
		setSubTask(TaskFactoryV2::Instance()->GotoPosition(chase_kick_task));
		return;
	}
	if (me.RawPos().dist(ballLineProjection) < 10 && me2Ball.mod()<50
		&& abs(Utils::Normalize(ball2Projection.dir()) - Utils::Normalize(ball.Vel().dir()))<0.1) {
		std::cout << "in wait :" << me.RawPos().dist(ballLineProjection) << std::endl;
		chase_kick_task.player.pos = ballLineProjection;
		chase_kick_task.player.angle = me2Ball.dir();
		setSubTask(TaskFactoryV2::Instance()->GotoPosition(chase_kick_task));
		return;
	}

	/*if (me.RawPos().dist(ballLineProjection) < 20 //&& me2Ball.mod() < 40
	&& 	abs(Utils::Normalize(ball2Projection.dir()) - Utils::Normalize(ball.Vel().dir())) - Param::Math::PI < 0.2) {
	std::cout << "in chase:" << me.RawPos().dist(ballLineProjection) << std::endl;
	/*chase_kick_task.player.pos = CGeoPoint(ball.RawPos().x() + ball.VelX() / 2, ball.RawPos().y() + ball.VelY() / 2);//ballLineProjection;
	chase_kick_task.player.angle = me2Ball.dir();
	setSubTask(TaskFactoryV2::Instance()->GotoPosition(chase_kick_task));

	chase_kick_task.player.speed_x = 2;
	setSubTask(TaskFactoryV2::Instance()->OpenSpeed(chase_kick_task));
	return;
	}*/


	do {
		ballArriveTime += testMinFrame * 1.0 / 60.0;
		testVel = ball.Vel().mod() - ballAcc*ballArriveTime;
		testBallLength = (ball.Vel().mod() + testVel)*ballArriveTime / 2;//(ball.Vel().mod2() - testVel*testVel) / (2 * ballAcc);
		testPoint.setX(ball.RawPos().x() + testBallLength * std::cos(ball.Vel().dir()));
		testPoint.setY(ball.RawPos().y() + testBallLength * std::sin(ball.Vel().dir()));
		meArriveTime = predictedTime(me, testPoint);
		//std::cout << ballAcc<<"ball Arrive@" << ballArriveTime << "\tme Arrive @" << meArriveTime << std::endl;
	} while (IsInField(testPoint) && (meArriveTime + RESPONSE_TIME) > ballArriveTime && meArriveTime < maxArriveTime);


	CVector testpoint2Ball = ball.RawPos() - testPoint;//CGeoPoint(450, 0)- testPoint;
	if (me2Ball.mod()>100 ) 
	chase_kick_task.player.angle = testpoint2Ball.dir();
	else 
	chase_kick_task.player.angle = me2Ball.dir();
	chase_kick_task.player.pos = testPoint;//CGeoPoint(testPoint.x() - 8 * testpoint2Goal.x() / testpoint2Goal.mod(), testPoint.y() - 8 * testpoint2Goal.y() / testpoint2Goal.mod());

	setSubTask(TaskFactoryV2::Instance()->GotoPosition(chase_kick_task));

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
