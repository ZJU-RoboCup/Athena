#include "PlayerCommandV2.h"
#include <utils.h>
#include <sstream>
#include "CommandSender.h"
#include <Simulator.h>
#include <robot_power.h>
#include <RobotCapability.h>
#include "PlayInterface.h"
#include "TaskMediator.h"

namespace{
	const bool SPEED_MODE = true;
	inline int merge_command_type(int type, unsigned char dribble)
	{
		if( dribble > DRIBBLE_DISABLED){
			type |= 0x08;
		}
		return type;
	}
	const double RSPEED_DRIBBLEHIGH = 1;	// 当旋转速度大于此值使用

	const double ACTION_ERROR = 1.0; // 执行误差
}

/************************************************************************/
/*                      CPlayerSpeed                                    */
/************************************************************************/
CPlayerSpeedV2::CPlayerSpeedV2(int number, double xspeed, double yspeed, double rspeed, unsigned char dribble)
	: CPlayerCommand(number,dribble)
	,_rspeed(rspeed)
{
	const CRobotCapability* robotCap = RobotCapFactory::Instance()->getRobotCap(1, 1);
	CVector velocity(xspeed, yspeed);

	// 把输出指令限制在小车的运动能力之内 added by qxz 08.06.07
	if (number == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie())) {
		if (velocity.mod() > robotCap->maxSpeedGoalie(0))
			velocity = Utils::Polar2Vector(robotCap->maxSpeedGoalie(0), velocity.dir());
	} else if (TaskMediator::Instance()->leftBack() != 0 && number == TaskMediator::Instance()->leftBack()
		|| TaskMediator::Instance()->rightBack() != 0 && number == TaskMediator::Instance()->rightBack()
		|| TaskMediator::Instance()->singleBack() != 0 && number == TaskMediator::Instance()->singleBack()
		|| TaskMediator::Instance()->sideBack() != 0 && number == TaskMediator::Instance()->sideBack()) {
			if (velocity.mod() > robotCap->maxSpeedBack(0))
				velocity = Utils::Polar2Vector(robotCap->maxSpeedBack(0), velocity.dir());
	} else {
		if (velocity.mod() > robotCap->maxSpeed(0))
			velocity = Utils::Polar2Vector(robotCap->maxSpeed(0), velocity.dir());
	}

	_xspeed = velocity.x();
	_yspeed = velocity.y();
}
void CPlayerSpeedV2::execute(bool isSimulation)
{
	const double speed2 = xspeed() * xspeed() + yspeed() * yspeed(); // 速度的平方
	if( speed2 > 0 && speed2 < 5 && std::abs(rspeed()) > 0.5 ){
		if(isSimulation)
			CSimulator::instance()->sendCommand(number(), merge_command_type(CTSpeed, dribble()), 0, 0, Utils::Rad2Deg(rspeed()));
		else{
			CommandSender::Instance()->setSpeed(number(), 0, 0, ACTION_ERROR*rspeed());
		}
	}else if( std::abs(rspeed()) < 0.01 ){
		if(isSimulation)
			CSimulator::instance()->sendCommand(number(), merge_command_type(CTSpeed, dribble()), xspeed(), yspeed(), 0);
		else{
			CommandSender::Instance()->setSpeed(number(), ACTION_ERROR*xspeed(), ACTION_ERROR*yspeed(), 0);
		}
	}else{
		if(isSimulation)
			CSimulator::instance()->sendCommand(number(), merge_command_type(CTSpeed, dribble()), xspeed(), yspeed(), Utils::Rad2Deg(rspeed()));
		else{
			CommandSender::Instance()->setSpeed(number(), ACTION_ERROR*xspeed(), ACTION_ERROR*yspeed(), ACTION_ERROR*rspeed());	
		}
	}
	if(!isSimulation){
		CommandSender::Instance()->setDribble(number(), dribble());
		//std::cout << "PlayCommand dribblePower: " << static_cast<int> (dribble()) << std::endl;
		CommandSender::Instance()->setKick(number(), 0, 0, 0);
	}
}
// 2015/7/11
void CPlayerSpeedV2::execute(bool isSimulation, int realNum)
{
	const double speed2 = xspeed() * xspeed() + yspeed() * yspeed(); // 速度的平方
	if( speed2 > 0 && speed2 < 5 && std::abs(rspeed()) > 0.5 ){
		if(isSimulation)
			CSimulator::instance()->sendCommand(number(), merge_command_type(CTSpeed, dribble()), 0, 0, Utils::Rad2Deg(rspeed()));
		else{
			CommandSender::Instance()->setSpeed(number(), realNum, 0, 0, ACTION_ERROR*rspeed());
		}
	}else if( std::abs(rspeed()) < 0.01 ){
		if(isSimulation)
			CSimulator::instance()->sendCommand(number(), merge_command_type(CTSpeed, dribble()), xspeed(), yspeed(), 0);
		else{
			CommandSender::Instance()->setSpeed(number(), realNum, ACTION_ERROR*xspeed(), ACTION_ERROR*yspeed(), 0);
		}
	}else{
		if(isSimulation)
			CSimulator::instance()->sendCommand(number(), merge_command_type(CTSpeed, dribble()), xspeed(), yspeed(), Utils::Rad2Deg(rspeed()));
		else{
			CommandSender::Instance()->setSpeed(number(), realNum, ACTION_ERROR*xspeed(), ACTION_ERROR*yspeed(), ACTION_ERROR*rspeed());	
		}
	}
	if(!isSimulation){
		CommandSender::Instance()->setDribble(number(), dribble());
		//std::cout << "PlayCommand dribblePower: " << static_cast<int> (dribble()) << std::endl;
		CommandSender::Instance()->setKick(number(), 0, 0, 0);
	}
}
void CPlayerSpeedV2::toStream(std::ostream& os) const
{
	os << xspeed() << "\t" << yspeed() << "\t" << rspeed();
}
CVector CPlayerSpeedV2::getAffectedVel() const
{
	return CVector(xspeed(), yspeed());
}
double CPlayerSpeedV2::getAffectedRotateSpeed() const
{
	return rspeed();
}

/************************************************************************/
/*                          Kick                                        */
/************************************************************************/
void CPlayerKickV2::execute(bool isSimulation)
{
	if(isSimulation)
		CSimulator::instance()->sendCommand(number(), merge_command_type(CTKick, dribble()), _normal, _chip, _pass);
	else{
		unsigned char dribblePower = dribble();
		// 发kick指令控球关小
		/*if (dribblePower > 0)
			dribblePower = DRIBBLE_LOWEST;
		CommandSender::Instance()->setDribble(number(), dribblePower);*/
		CommandSender::Instance()->setKick(number(), _normal, _chip, _pass);
	}
}
void CPlayerKickV2::toStream(std::ostream& os) const
{
	os << number();
	if( _normal ){
		if( _pass ){
			os << " pass " << _pass;
		}else{
			os << " kick " << _normal;
		}
	}else{
		os << " chip kick " << _chip;
	}
}

/************************************************************************/
/*                         Gyro(陀螺仪)                                 */
/************************************************************************/
void CPlayerGyroV2::execute(bool isSimulation)
{
	if(isSimulation){
		CSimulator::instance()->sendCommand(number(), merge_command_type(CTArc, dribble()), radius(), Utils::Rad2Deg(angle()), rspeed());
	} else{
		CommandSender::Instance()->setDribble(number(), dribble());
		CommandSender::Instance()->setGyro(number(), dribble(), angle(), radius(), rspeed());
	}
}
void CPlayerGyroV2::toStream(std::ostream& os) const
{
	os << "rotNum "<< number()<<"radius "<<_radius<<"angle "<<_angle;
}