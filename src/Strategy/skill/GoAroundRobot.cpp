#include "GoAroundRobot.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include "ControlModel.h"
#include <utils.h>
#include <ControlModel.h>

namespace
{
	//ÅäÖÃ×´Ì¬»ú
	enum TestCircleBall
	{
		GOAROUND = 1,
	};
} 

CGoAroundRobot::CGoAroundRobot()
{

}

void CGoAroundRobot::plan(const CVisionModule* pVision)
{
	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1){
		setState(BEGINNING);
	}

	const int vecNumber = task().executor;
	double faceDir = task().player.angle;
	int circleDir =task().player.rotdir;
	CGeoPoint circleCenter = task().player.pos;

	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	double radius = task().player.rotvel;
	TaskT myTask(task());

	myTask.player.flag = myTask.player.flag | PlayerStatus::DODGE_OUR_DEFENSE_BOX;
	
	int new_state = state();
	int old_state;
	switch (state()) {
		case BEGINNING:		
			new_state = GOAROUND;
			break;
		case GOAROUND:
			
			break;
		default:
			new_state = BEGINNING;
			break;
		}

	setState(new_state);

	if  (GOAROUND == state())
	{
		if (circleDir  ==  0)
		{
			target =circleCenter+ Utils::Polar2Vector(radius,Utils::Normalize((me.Pos() - circleCenter).dir() + Param::Math::PI /6));
		}else
		{
			target =circleCenter+ Utils::Polar2Vector(radius,Utils::Normalize((me.Pos() - circleCenter).dir() - Param::Math::PI /6));
		}
		
		setSubTask(PlayerRole::makeItSimpleGoto(task().executor,target,faceDir,CVector(0,0),0,0));
	}

	_lastCycle = pVision->Cycle();	
	return CStatedTask::plan(pVision);
}
CPlayerCommand* CGoAroundRobot::execute(const CVisionModule* pVision)
{
	if(subTask()) {
		return subTask()->execute(pVision);
	}

	return NULL;
}