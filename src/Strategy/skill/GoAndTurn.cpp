#include "GoAndTurn.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include "ControlModel.h"
#include "KickStatus.h"
#include <utils.h>
#include <ControlModel.h>

namespace
{
	//ÅäÖÃ×´Ì¬»ú
	enum TestCircleBall
	{
		GOAROUND = 1,
		ADJUST,
		KICK,
	};
}

CGoAndTurn::CGoAndTurn()
{
	angeDiff_Use = Param::Math::PI * 2;
	 R = Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + Param::Field::BALL_SIZE + 12;
	  _lastCycle = 0;
	  CircleCounter = 0.0;
}

void CGoAndTurn::plan(const CVisionModule* pVision)
{
	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1){
		setState(BEGINNING);
		count = 0;
	}

	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const int vecNumber = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const CVector self2ball = ballPos - me.Pos();
	const CVector ball2self = me.Pos() - ballPos;
	double faceDir= Utils::Normalize(( ball.Pos() - me.Pos()).dir());

	TaskT myTask(task());

	myTask.player.flag = myTask.player.flag | PlayerStatus::DODGE_OUR_DEFENSE_BOX;
	
	int new_state = state();
	int old_state;
	switch (state()) {
		case BEGINNING:		
			new_state = GOAROUND;
			break;
		case GOAROUND:
			/*cout << "ccc:" << CircleCounter<<endl;
			if (CircleCounter > 300 && abs(Utils::Normalize( (ball.Pos() - me.Pos()).dir() - (CGeoPoint(Param::Field::PITCH_LENGTH / 2.0,0) - ball.Pos()).dir()) )< Param::Math::PI / 72)
			{
				target = ball.Pos() + Utils::Polar2Vector(15,Utils::Normalize(CVector(ball.Pos()-CGeoPoint(Param::Field::PITCH_LENGTH / 2.0,0)).dir()));
				new_state = ADJUST;
			}else 
			{
				CircleCounter ++;
			}*/
			break;
		case  ADJUST:
			if (Utils::Normalize( (ball.Pos() - me.Pos()).dir() - (CGeoPoint(Param::Field::PITCH_LENGTH / 2.0,0) - ball.Pos()).dir())< Param::Math::PI / 72 && (me.Pos().dist(target) < 5))
			{
				new_state = KICK;
			}
		case KICK: 
			CircleCounter = 0;
			break;

		default:
			new_state = BEGINNING;
			break;
		}

	setState(new_state);

	if  (GOAROUND == state())
	{
	
		
	//	double bbb = (me.Pos() - ball.Pos()).dir();
		if (ball.Pos().y() < 0)
		{
			faceDir = faceDir + Param::Math::PI /3.7;
			target =ball.Pos() + Utils::Polar2Vector(20,Utils::Normalize((me.Pos() - ball.Pos()).dir() + Param::Math::PI /5));
		}else
		{
			faceDir = faceDir - Param::Math::PI /3.7;
			target =ball.Pos() + Utils::Polar2Vector(20,Utils::Normalize((me.Pos() - ball.Pos()).dir() - Param::Math::PI /5));
		}
		
		setSubTask(PlayerRole::makeItSimpleGoto(task().executor,target,faceDir,CVector(0,0),0,0));
	}else if (ADJUST == state())
	{
		double faceDir_2Goal = Utils::Normalize((  CGeoPoint(Param::Field::PITCH_LENGTH / 2.0,0) - ball.Pos()).dir());
		setSubTask(PlayerRole::makeItSimpleGoto(task().executor,target,faceDir_2Goal,CVector(0,0),0,0));
	}
	else if (KICK == state())
	{
		KickStatus::Instance()->setKick(task().executor,200);
		setSubTask(PlayerRole::makeItNoneTrajGetBall(task().executor,CVector(CGeoPoint(Param::Field::PITCH_LENGTH / 2,0) - ball.Pos()).dir(),CVector(0,0),0,0));
	}

	_lastCycle = pVision->Cycle();	
	return CStatedTask::plan(pVision);


}
CPlayerCommand* CGoAndTurn::execute(const CVisionModule* pVision)
{
	if(subTask()) {
		return subTask()->execute(pVision);
	}

	return NULL;
}