#include "ProtectTurn.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include <utils.h>
#include <param.h>
#include <DribbleStatus.h>
#include <RobotSensor.h>
#include <CommandFactory.h>
#include <KickStatus.h>
using namespace std;
namespace{	
	enum DribbleTurnKickState {
		Turn =1,
		Kick,
	};
}

CProtectTurn::CProtectTurn()
{
	_lastCycle = 0;
	_directCommand = NULL;
}


CGeoPoint calculate_central_point(PlayerVisionT me,double r,boolean rotation_direction){
	//cout<<"dir:"<<me.Dir()<<endl;
	//cout<<"x:"<<me.Pos().x()<<endl<<"y:"<<me.Pos().y()<<endl;
	const double x0 = me.Pos().x();
	const double y0 = me.Pos().y();
	double dir0 = me.Dir();
	double dir1 = Utils::Normalize( dir0 + rotation_direction*Param::Math::PI / 2 );//存储圆心的方向角度
	double dx = r/cos(dir1);
	double dy = r/sin(dir1);
	return CGeoPoint(x0+dx,y0+dy);
}

CVector vel_resolve(double v,double dir){
	double x = v * cos(dir);
	double y = v * sin(dir);
	return CVector(x,y);
}

void CProtectTurn::plan(const CVisionModule* pVision)
{
	_directCommand = NULL;
	const int vecNumber = task().executor;
	const double kickPower=task().player.kickpower;
	const double finalDir=task().player.angle;
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	
	const CVector self2ball = ballPos - me.Pos();
	const CVector ball2self = me.Pos() - ballPos;


	//CGeoPoint Central = calculate_central_point(me,r,rotation_direction);
	//cout<<"X:"<<Central.x()<<endl<<"Y:"<<Central.y()<<endl;

	CGeoLine ballMoveingLine(ball.Pos(), ball.Vel().dir());
	CGeoPoint projMe = ballMoveingLine.projection(me.Pos());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + Param::Math::PI);
	int turnCycle=200;


	bool isInfraredOn = RobotSensor::Instance()->IsInfraredOn(vecNumber);//红外是否被触发


	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1|| _lastRunner != vecNumber){
		setState(BEGINNING);
	}

	int new_state = state();
	int old_state;
	do{
		old_state = new_state;
		switch ( old_state )
		{
			case BEGINNING:
				{
					_initDir =me.Dir();
					new_state = Turn;
				}
				break;
			case Turn:
				{	
					if (abs(me.Dir())<Param::Math::PI/12){
						new_state = Kick;
					}
					break;
				}
			case Kick:
					break;			
			default:
				new_state = Kick;
				break;
		}
	} while ( old_state != new_state );
	setState(new_state);
	DribbleStatus::Instance()->setDribbleCommand(vecNumber,2);
	if (Turn == state()){

		boolean rotation_direction = 1;//转动方向，1为顺时针，-1为逆时针
		double t = 2;//0.65;//圆周半周期 单位秒 T=4
		double r = 40;//圆周半径 单位厘米 
		double speedW = Param::Math::PI/t;//弧度/s
		double v = speedW*r;//线速度 v = wr

		CVector globalVel = vel_resolve(v,me.Dir());
		setSubTask(PlayerRole::makeItRun(vecNumber, globalVel.x(), globalVel.y(), speedW, 0));
	} else if (Kick == state()){

		double shootdx = 450-me.Pos().x();
		double shootdy = abs(me.Pos().y());
		double shootDir = atan2(shootdy,shootdx);
		cout<<"shoot:"<<shootDir-finalDir<<endl;
		KickStatus::Instance()->setKick(vecNumber,kickPower);
		setSubTask(PlayerRole::makeItReceivePass(vecNumber, 3.14));
	} 


	_lastCycle = pVision->Cycle();
	_lastRunner = vecNumber;
	CStatedTask::plan(pVision);
}

CPlayerCommand* CProtectTurn::execute(const CVisionModule* pVision)
{
	if( _directCommand ){
		return _directCommand;
	} else if (subTask())
	{
		return subTask()->execute(pVision);
	}
	return NULL;
}
