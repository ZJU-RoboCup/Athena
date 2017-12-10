#include "DribbleTurnKick.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include <utils.h>
#include <DribbleStatus.h>
#include <RobotSensor.h>
#include <CommandFactory.h>
#include <KickStatus.h>

namespace{	
	enum DribbleTurnKickState {
		Turn =1,
		Kick,
	};
}

CDribbleTurnKick::CDribbleTurnKick()
{
	_lastCycle = 0;
	_directCommand = NULL;
	_turnToKickCouter=0;
	_initDir=0;
}

void CDribbleTurnKick::plan(const CVisionModule* pVision)
{
	_directCommand = NULL;

	const int vecNumber = task().executor;
	const double finalDir = task().player.angle;
	const double turnRotVel =task().player.rotvel;
	const double kickPower=task().player.kickpower;

	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const CVector self2ball = ballPos - me.Pos();
	const CVector ball2self = me.Pos() - ballPos;

	CGeoLine ballMoveingLine(ball.Pos(), ball.Vel().dir());
	CGeoPoint projMe = ballMoveingLine.projection(me.Pos());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + Param::Math::PI);
	const double diffAngle = Utils::Normalize(finalDir-me.Dir());
	int turnCycle=200;


	bool isInfraredOn = RobotSensor::Instance()->IsInfraredOn(vecNumber);
	int flags = task().player.flag | PlayerStatus::DODGE_OUR_DEFENSE_BOX;


	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1
		|| _lastRunner != vecNumber){
		setState(BEGINNING);
		_turnToKickCouter=0;
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
				_turnToKickCouter++;
				if (_turnToKickCouter==turnCycle||fabs(diffAngle)<Param::Math::PI*2.5/180)
				{
					new_state = Kick;
					_turnToKickCouter=0;
				}
				if (!Utils::AngleBetween(me.Dir(),_initDir,finalDir,Param::Math::PI*5/180))
				{
					new_state = BEGINNING;
					_turnToKickCouter=0;
				}
				break;
			case Kick:
				break;
			default:
				new_state = Kick;
				break;
		}
	} while ( old_state != new_state );
	//cout << new_state<<endl;
	setState(new_state);
	DribbleStatus::Instance()->setDribbleCommand(vecNumber,2);
	if (Turn == state()){
		const double changeAngle=Utils::Normalize(finalDir-_initDir);
		double speedW=turnRotVel*Utils::Sign(changeAngle);
		double speedY=-Utils::Sign(speedW)*35;
		double speedX=0;
		CVector localVel(speedX, speedY);
		CVector globalVel = localVel.rotate(me.Dir());
		setSubTask(PlayerRole::makeItRun(vecNumber, globalVel.x(), globalVel.y(), speedW, 0));
	} else if (Kick == state()){
		KickStatus::Instance()->setKick(vecNumber,kickPower);
		setSubTask(PlayerRole::makeItChaseKickV1(vecNumber, finalDir,PlayerStatus::FORCE_KICK));
	} 


	_lastCycle = pVision->Cycle();
	_lastRunner = vecNumber;
	CStatedTask::plan(pVision);
}

CPlayerCommand* CDribbleTurnKick::execute(const CVisionModule* pVision)
{
	if( _directCommand ){
		return _directCommand;
	} else if (subTask())
	{
		return subTask()->execute(pVision);
	}
	return NULL;
}
