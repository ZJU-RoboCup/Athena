#include "WaitTouch.h"
#include "skill/Factory.h"
#include "GDebugEngine.h"
#include "BallSpeedModel.h"
#include "BallStatus.h"
#include "WaitKickPos.h"
#include "DribbleStatus.h"
#include "KickStatus.h"

namespace{
	int MAX_CNT_FOR_SHOOT = 2;
	const bool verBos = false;
}

CWaitTouch::CWaitTouch() 
{
	last_cycle = 0;
	cur_cnt = 0;
	need_shoot = false;
}

void CWaitTouch::plan(const CVisionModule* pVision) 
{
	///> 1.任务参数及图像信息
	if (pVision->Cycle() - last_cycle  > 6) {
		need_shoot = false;
	}

	int runner = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(runner);
	const MobileVisionT& ball = pVision->Ball();
	double kick_dir;

	TaskT wait_kick_task(task());
	wait_kick_task.player.flag = wait_kick_task.player.flag | PlayerStatus::DODGE_OUR_DEFENSE_BOX;
	kick_dir	= wait_kick_task.player.angle;
	CVector car_2_ball = ball.Pos() - me.Pos();
	CVector car_2_goal = CGeoPoint(Param::Field::PITCH_LENGTH / 2.0, 0) - me.Pos();
	if (car_2_ball.mod() < 20 && ball.Pos().x() > 0) {
		//KickStatus::Instance()->setKick(runner, 790);
		cout << "bothshoot!" << endl;
		KickStatus::Instance()->setBothKick(runner,790,180);
	}
	if (!need_shoot) {
		if (pVision->Ball().Vel().mod() < 100 && car_2_ball.mod() < 40/* && fabs(Utils::Normalize(car_2_ball.dir() - car_2_goal.dir())) < Param::Math::PI / 12*/) {
			cur_cnt++;
			if (cur_cnt > MAX_CNT_FOR_SHOOT) {
				cur_cnt = 0;
				need_shoot = true;
			}
		}
		WaitKickPos::Instance()->GenerateWaitKickPos(wait_kick_task.ball.pos,wait_kick_task.ball.angle,runner,kick_dir);
		wait_kick_task.player.pos = WaitKickPos::Instance()->getKickPos(runner);
		if(verBos) GDebugEngine::Instance()->gui_debug_msg(wait_kick_task.player.pos, "here", COLOR_CYAN);
		setSubTask(TaskFactoryV2::Instance()->GotoPosition(wait_kick_task));
	} else{
		setSubTask(PlayerRole::makeItShootBall(runner,kick_dir,false,Param::Math::PI/18, 900,0,wait_kick_task.player.flag));
		if(verBos) GDebugEngine::Instance()->gui_debug_msg(wait_kick_task.player.pos, "touch", COLOR_CYAN);
	}
	last_cycle = pVision->Cycle();
	CStatedTask::plan(pVision);
}

CPlayerCommand* CWaitTouch::execute(const CVisionModule* pVision)
{
	if (subTask()) {
		return subTask()->execute(pVision);
	}
	return NULL;
}