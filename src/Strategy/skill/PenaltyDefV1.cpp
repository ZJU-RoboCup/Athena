#include "PenaltyDefV1.h"
#include "skill/Factory.h"
#include <VisionModule.h>
#include <PlayerCommand.h>
#include <CommandFactory.h>
#include <ControlModel.h>
#include "BallStatus.h"
#include <BestPlayer.h>
#include <robot_power.h>
#include "RobotCapability.h"
#include "GDebugEngine.h"
#include "GoaliePosV1.h"

namespace
{
	bool VERBOSE_MODE = false;
	bool use_new_method = false;
}
CPenaltyDefV1::CPenaltyDefV1()
{}
void CPenaltyDefV1::plan(const CVisionModule* pVision)
{
	static bool is_first = true;
	if (is_first == true && VERBOSE_MODE)
	{
		cout<<"get into CPenaltyDefV1 !"<<endl;
		is_first = false;
	}
	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1 )
	{
		setState(BEGINNING);
	}
	const MobileVisionT& ball = pVision->Ball();
	const int runner = task().executor;
	const int flag = task().player.flag;
	const PlayerVisionT& self = pVision->OurPlayer(runner);
	CGeoPoint pen_sht_pos;
	if (use_new_method == true)
	{
		pen_sht_pos = GoaliePosV1::Instance()->GetPenaltyShootPosV2(pVision);
	}
	else
	{
		pen_sht_pos = GoaliePosV1::Instance()->GetPenaltyShootPos(pVision);
	}
	const double face_dir = Utils::Normalize((ball.Pos() - self.Pos()).dir());
	
	TaskT new_task;
	new_task.executor = runner;
	new_task.player.pos = pen_sht_pos;
	new_task.player.angle = face_dir;
	new_task.player.max_acceleration = 1400;
	setSubTask(TaskFactoryV2::Instance()->GotoPosition(new_task));
	//setSubTask(PlayerRole::makeItSimpleGoto(runner, pen_sht_pos, face_dir, CVector(0 ,0), 0, flag));
	
	_lastCycle = pVision->Cycle();
	CStatedTask::plan(pVision);
}
CPlayerCommand* CPenaltyDefV1::execute(const CVisionModule* pVision)
{
	if( subTask() ){
		return subTask()->execute(pVision);
	}
	if( _directCommand ){
		return _directCommand;
	}
	return 0;
}