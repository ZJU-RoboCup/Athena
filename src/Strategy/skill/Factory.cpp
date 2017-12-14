#include "Factory.h"
#include <TaskMediator.h>

#include <misc_types.h>
#include <WorldModel.h>
#include "CommandFactory.h"
#include "GDebugEngine.h"

/************************************************************************/
/*								concrete skill							*/
/************************************************************************/
//1.atom skills(TODO)
#include "GotoPosition.h"
#include "SmartGotoPosition.h"
#include "GoAroundBall.h"

//2.basic skills(TODO)
#include "StopRobot.h"
#include "Speed.h"
#include "OpenSpeed.h"
#include "InterceptBallV3.h"
#include "CrazyPush.h"
#include "CircleAndPass.h"
#include "GoAndTurnKick.h"
#include "GoAndTurnKickV3.h"
#include "GoAndTurnKickV4.h"
#include "GoAroundRobot.h"
#include "GoAndTurn.h"
#include "ChaseKickV1.h"
#include "ChaseKickV2.h"
#include "TouchKick.h"
#include "PenaltyDefV1.h"
#include "PenaltyDefV2.h"
#include "PenaltyDef2017V1.h"
#include "PenaltyDef2017V2.h"
#include "TestCircleBall.h"
#include "AdvanceBallV1.h"
#include "AdvanceBallV2.h"
#include "Tandem.h"
#include "WaitTouch.h"
#include "WaitTouch2016.h"
#include "StaticGetBallNew.h"
#include "GetBallV3.h"
#include "GetBallV4.h"
#include "SlowGetBall.h"
#include "TimeDelayTest.h"
#include "Marking.h"
#include "Goalie2013.h"
#include "Goalie2015.h"
#include "Goalie2015Hefei.h"
#include "Goalie2015V2.h"
#include "PidToPosition.h"
#include "InterceptBall.h"
#include "ShootBall.h"
#include "ShootBallV2.h"
#include "PassBall.h"
#include "ReceivePass.h"
#include "PenaltyKick2013.h"
#include "PenaltyKick2014.h"
#include "PenaltyKick2017V1.h"
#include "PenaltyKick2017V2.h"
#include "PenaltyGoalie2012.h"
#include "ProtectBall.h"
#include "ForceStartRush.h"
#include "GoAvoidShootLine.h"
#include "DribbleTurn.h"
#include "DriftKick.h"
#include "MarkingTouch.h"
#include "MarkingFront.h"
#include "DribbleTurnKick.h"
#include "InterceptBallV4.h"
#include "InterceptBallV2.h"
#include "InterceptBallV5.h"
#include "InterceptBallV6.h"
#include "ProtectTurn.h"
#include "InterceptTouch.h"

/************************************************************************/
/*                      TaskFactoryV2									*/
/* 请注意:																*/
/* 上层（CStatedScriptPlay) 通过调用 CBasicPlay 中的 setTask() 设定任务	*/
/* 并将每个策略执行小车对应的任务存放在 TaskMediator 里面				*/
/* 执行（ActionModule）通过任务的 execute() 接口生成任务小车对应的指令	*/
/************************************************************************/

//////////////////////////////////////////////////////////////////////////
// an template used as a task pool for storing tasks for executing player
template < class CTaskType >
CPlayerTask* CTaskFactoryV2::MakeTask(const TaskT& task )
{
	static CTaskType taskPool[Param::Field::MAX_PLAYER + 1];
	taskPool[task.executor].reset(task);
	return &taskPool[task.executor];
}

//////////////////////////////////////////////////////////////////////////
// current now debugged skill for game
CPlayerTask* CTaskFactoryV2::GotoPosition(const TaskT& task) {
	return MakeTask< CGotoPositionV2 >(task);
}

CPlayerTask* CTaskFactoryV2::SmartGotoPosition(const TaskT& task) {
	return MakeTask< CSmartGotoPositionV2 >(task);
}

CPlayerTask* CTaskFactoryV2::GoAvoidShootLine(const TaskT& task){
	return MakeTask< CGoAvoidShootLine>(task);
}

CPlayerTask* CTaskFactoryV2::StopRobot(const TaskT& task) {
	return MakeTask< CStopRobotV2 >(task);
}

CPlayerTask	* CTaskFactoryV2::Speed(const TaskT& task) {
	return MakeTask< CSpeed >(task);
}

CPlayerTask	* CTaskFactoryV2::OpenSpeed(const TaskT& task) {
	return MakeTask< COpenSpeed >(task);
}

CPlayerTask* CTaskFactoryV2::StaticGetBall(const TaskT& task) {
	return MakeTask< CStaticGetBallNew >(task);
}

CPlayerTask* CTaskFactoryV2::NoneTrajGetBall(const TaskT& task) {
	return MakeTask< CGetBallV3 >(task);
}

CPlayerTask* CTaskFactoryV2::NoneTrajGetBallNew(const TaskT& task) {
	return MakeTask< CGetBallV4 >(task);
}

CPlayerTask* CTaskFactoryV2::GetBallV4(const TaskT& task) {
	return MakeTask< CGetBallV4 >(task);
}

CPlayerTask* CTaskFactoryV2::ForceStartRush(const TaskT& task) {
	return MakeTask< CForceStartRush >(task);
}

CPlayerTask* CTaskFactoryV2::ChaseKickV1(const TaskT& task) {
	return MakeTask< CChaseKickV1 >(task);
}

CPlayerTask* CTaskFactoryV2::ChaseKickV2(const TaskT& task){
	return MakeTask< CChaseKickV2>(task);
}

CPlayerTask* CTaskFactoryV2::ProtectBall(const TaskT& task){
	return MakeTask< CProtectBall>(task);
}

CPlayerTask* CTaskFactoryV2::TouchKick(const TaskT& task) {
	return MakeTask< CTouchKick >(task);
}

CPlayerTask* CTaskFactoryV2::InterceptBallV2(const TaskT& task) {
	return MakeTask< CInterceptBallV2>(task);
}
CPlayerTask* CTaskFactoryV2::InterceptBallV3(const TaskT& task) {
	return MakeTask< CInterceptBallV3>(task);
}

CPlayerTask* CTaskFactoryV2::MarkingTouch(const TaskT& task) {
	return MakeTask< CMarkingTouch >(task);
}

CPlayerTask* CTaskFactoryV2::MarkingFront(const TaskT& task) {
	return MakeTask< CMarkingFront >(task);
}

CPlayerTask* CTaskFactoryV2::ShootBall(const TaskT& task) {
	return MakeTask< CShootBall >(task);
}

CPlayerTask* CTaskFactoryV2::ShootBallV2(const TaskT& task) {
	return MakeTask< CShootBallV2 >(task);
}

CPlayerTask* CTaskFactoryV2::DriftKick(const TaskT& task) {
	return MakeTask< CDriftKick >(task);
}


CPlayerTask* CTaskFactoryV2::CarzyPush(const TaskT& task){
	return MakeTask<CCrazyPush>(task);
}
CPlayerTask* CTaskFactoryV2::GoAroundBall(const TaskT& task) {
	return MakeTask< CGoAroundBall >(task);
}
CPlayerTask* CTaskFactoryV2::GoAroundRobot(const TaskT& task){
	return MakeTask< CGoAroundRobot >(task);
}

CPlayerTask* CTaskFactoryV2::PenaltyDefV1(const TaskT& task) {
	return MakeTask< CPenaltyDefV1 >(task);
}

CPlayerTask* CTaskFactoryV2::PenaltyDefV2(const TaskT& task) {
	return MakeTask< CPenaltyDefV2 >(task);
}

CPlayerTask* CTaskFactoryV2::PenaltyDef2017V1(const TaskT& task) {
	return MakeTask< CPenaltyDef2017V1 >(task);
}

CPlayerTask* CTaskFactoryV2::PenaltyDef2017V2(const TaskT& task) {
	return MakeTask< CPenaltyDef2017V2 >(task);
}

CPlayerTask* CTaskFactoryV2::AdvanceBallV1(const TaskT& task) {
	return MakeTask< CAdvanceBallV1 >(task);
}

CPlayerTask* CTaskFactoryV2::AdvanceBallV2(const TaskT& task) {
	return MakeTask< CAdvanceBallV2 >(task);
}

CPlayerTask* CTaskFactoryV2::Tandem(const TaskT& task) 
{
	return MakeTask< CTandem >(task);
}

CPlayerTask* CTaskFactoryV2::GoAndTurnKick(const TaskT& task) {
	return MakeTask< CGoAndTurnKick >(task);
}

CPlayerTask* CTaskFactoryV2::CircleAndPass(const TaskT& task) {
	return MakeTask< CCircleAndPass >(task);
}


//CPlayerTask* CTaskFactoryV2::GoAndTurnKickV2(const TaskT& task) {
//	return MakeTask< CGoAndTurnKickV2 >(task);
//}
CPlayerTask* CTaskFactoryV2::GoAndTurnKickV3(const TaskT& task) {
	return MakeTask< CGoAndTurnKickV3 >(task);
}

CPlayerTask* CTaskFactoryV2::GoAndTurnKickV4(const TaskT& task) {
	return MakeTask< CGoAndTurnKickV4 >(task);
}

CPlayerTask* CTaskFactoryV2::GoAndTurn(const TaskT& task) {
	return MakeTask< CGoAndTurn >(task);
}
CPlayerTask* CTaskFactoryV2::SlowGetBall(const TaskT& task) {
	return MakeTask< CSlowGetBall >(task);
}

CPlayerTask* CTaskFactoryV2::timeDelayTest(const TaskT& task) {
	return MakeTask< CTimeDelayTest >(task);
}

CPlayerTask* CTaskFactoryV2::Marking(const TaskT& task) {
	return MakeTask< CMarking >(task);
}

CPlayerTask * CTaskFactoryV2::TestCircleBall(const TaskT& task){
	return MakeTask<CTestCircleBall>(task);
}


CPlayerTask* CTaskFactoryV2::PassBall(const TaskT& task) {
	return MakeTask< CPassBall >(task);
}

CPlayerTask* CTaskFactoryV2::ReceivePass(const TaskT& task) {
	return MakeTask< CReceivePass >(task);
}
CPlayerTask* CTaskFactoryV2::PenaltyGoalie(const TaskT& task){
	return MakeTask< CPenaltyGoalie2012 >(task);
}

CPlayerTask* CTaskFactoryV2::Goalie2013(const TaskT& task){
	return MakeTask< CGoalie2013 >(task);

}

CPlayerTask* CTaskFactoryV2::Goalie2015(const TaskT& task){
	return MakeTask< CGoalie2015Hefei >(task);

}

CPlayerTask* CTaskFactoryV2::Goalie2015V2(const TaskT& task){
	return MakeTask< CGoalie2015V2 >(task);

}

CPlayerTask* CTaskFactoryV2::PidToPosition(const TaskT& task){
	return MakeTask< CPidToPosition >(task);
}

CPlayerTask* CTaskFactoryV2::InterceptBall(const TaskT& task)
{
	return MakeTask< CInterceptBall >(task);
}

CPlayerTask* CTaskFactoryV2::InterceptBallV4(const TaskT& task)
{
	return MakeTask< CInterceptBallV4 >(task);
}

CPlayerTask* CTaskFactoryV2::InterceptBallV5(const TaskT& task)
{
	return MakeTask< CInterceptBallV5 >(task);
}

CPlayerTask* CTaskFactoryV2::InterceptBallV6(const TaskT& task)
{
	return MakeTask< CInterceptBallV6 >(task);
}

CPlayerTask* CTaskFactoryV2::WaitTouch(const TaskT& task){
	return MakeTask<CWaitTouch>(task);
}

CPlayerTask* CTaskFactoryV2::WaitTouch2016(const TaskT& task){
	return MakeTask<CWaitTouch2016>(task);
}

CPlayerTask* CTaskFactoryV2::PenaltyKick2013(const TaskT& task){
	return MakeTask<CPenaltyKick2013>(task);
}

CPlayerTask* CTaskFactoryV2::PenaltyKick2014(const TaskT& task){
	return MakeTask<CPenaltyKick2014>(task);
}

CPlayerTask* CTaskFactoryV2::PenaltyKick2017V1(const TaskT& task) {
	return MakeTask<CPenaltyKick2017V1>(task);
}

CPlayerTask* CTaskFactoryV2::PenaltyKick2017V2(const TaskT& task) {
	return MakeTask<CPenaltyKick2017V2>(task);
}

CPlayerTask* CTaskFactoryV2::DribbleTurn(const TaskT& task){
	return MakeTask<CDribbleTurn>(task);
}

CPlayerTask* CTaskFactoryV2::DribbleTurnKick(const TaskT& task){
	return MakeTask<CDribbleTurnKick>(task);
}

CPlayerTask* CTaskFactoryV2::ProtectTurn(const TaskT& task){
	return MakeTask<CProtectTurn>(task);
}

CPlayerTask* CTaskFactoryV2::InterceptTouch(const TaskT& task) {
	return MakeTask<CInterceptTouch>(task);
}

//////////////////////////////////////////////////////////////////////////
// define the namespace used to provide interface for task calling
namespace PlayerRole {
	CPlayerTask* makeItStop(const int num, const int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->StopRobot(playerTask);
	}
	CPlayerTask* makeItGoto(const int num, const CGeoPoint& target, const double dir, const int flags, const int sender)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.pos = target;
        playerTask.player.vel = CVector(0.0,0.0);
		playerTask.player.rotvel = 0;
		playerTask.player.angle = dir;
		playerTask.player.flag = flags;
		playerTask.ball.Sender = sender;
		return TaskFactoryV2::Instance()->SmartGotoPosition(playerTask);
	}
	CPlayerTask* makeItGoto(const int num, const CGeoPoint& target, const double dir, const CVector& vel, const double rotvel, const int flags, const int sender)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.pos = target;
		playerTask.player.angle = dir;
		playerTask.player.vel = vel;
		playerTask.player.rotvel = rotvel;
		playerTask.player.flag = flags;
		playerTask.ball.Sender = sender;
		return TaskFactoryV2::Instance()->SmartGotoPosition(playerTask);
	}

	CPlayerTask* makeItSimpleGoto(const int num, const CGeoPoint& target, const double dir, const int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.pos = target;
		playerTask.player.vel = CVector(0.0,0.0);
		playerTask.player.rotvel = 0;
		playerTask.player.angle = dir;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->GotoPosition(playerTask);
	}

	CPlayerTask* makeItSimpleGoto(const int num, const CGeoPoint& target, const double dir, const CVector& vel, const double rotvel, const int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.pos = target;
		playerTask.player.angle = dir;
		playerTask.player.vel = vel;
		playerTask.player.rotvel = rotvel;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->GotoPosition(playerTask);
	}

	CPlayerTask* makeItGoAvoidShootLine(const int num, const CGeoPoint& target, const double dir, const CVector& vel, const double rotvel, const int flags,const int sender)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.pos = target;
		playerTask.player.angle = dir;
		playerTask.player.vel = vel;
		playerTask.player.rotvel = rotvel;
		playerTask.player.flag = flags;
		playerTask.ball.Sender = sender;
		return TaskFactoryV2::Instance()->GoAvoidShootLine(playerTask);
	}

	CPlayerTask* makeItCrazyPush(const int num, double faceDir)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = faceDir;
		return TaskFactoryV2::Instance()->CarzyPush(playerTask);
	}
	CPlayerTask* makeItRun(const int num, const double speedX, const double speedY, const double rotSpeed, int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.speed_x = speedX;
		playerTask.player.speed_y = speedY;
		playerTask.player.rotate_speed = rotSpeed;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->Speed(playerTask);
	}

	CPlayerTask* makeItRunLocalVersion(const int num, const double speedX, const double speedY, const double rotSpeed)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.speed_x = speedX;
		playerTask.player.speed_y = speedY;
		playerTask.player.rotate_speed = rotSpeed;
		return TaskFactoryV2::Instance()->OpenSpeed(playerTask);
	}

	CPlayerTask* makeItStaticGetBall(const int num, const double dir, CVector finalVel, int flags, double StopDist, CTRL_METHOD mode)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = dir;
		playerTask.player.vel = finalVel;
		playerTask.player.flag = flags;
		playerTask.player.rotvel = StopDist;
		playerTask.player.specified_ctrl_method = mode;
		return TaskFactoryV2::Instance()->StaticGetBall(playerTask);
	}

	CPlayerTask* makeItNoneTrajGetBall(const int num,const double dir,CVector finalVel,int flags,double StopDist,CTRL_METHOD mode)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = dir;
		playerTask.player.vel = finalVel;
		playerTask.player.flag = flags;
		playerTask.player.rotvel = StopDist;
		playerTask.player.specified_ctrl_method = mode;
		return TaskFactoryV2::Instance()->NoneTrajGetBall(playerTask);
	}

	CPlayerTask* makeItNoneTrajGetBallNew(const int num, const double dir, CVector finalVel, int flags, double StopDist, CTRL_METHOD mode)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = dir;
		playerTask.player.vel = finalVel;
		playerTask.player.flag = flags;
		playerTask.player.rotvel = StopDist;
		playerTask.player.specified_ctrl_method = mode;
		return TaskFactoryV2::Instance()->NoneTrajGetBallNew(playerTask);
	}

	CPlayerTask* makeItGoAndTurnKickV4(const int num,const double dir,CVector finalVel,int flags,double StopDist,CTRL_METHOD mode)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = dir;
		playerTask.player.vel = finalVel;
		playerTask.player.flag = flags;
		playerTask.player.rotvel = StopDist;
		playerTask.player.specified_ctrl_method = mode;
		return TaskFactoryV2::Instance()->GoAndTurnKickV4(playerTask);
	}
	CPlayerTask* makeItForceStartRush(const int num,double faceDir,int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = faceDir;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->ForceStartRush(playerTask);
	}
	CPlayerTask* makeItChaseKickV1(const int num, double faceDir, int flags,bool isChip)
	//重载 add by gty 17-3-26
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = faceDir;
		playerTask.player.flag = flags;
		playerTask.player.ischipkick = isChip;
		return TaskFactoryV2::Instance()->ChaseKickV1(playerTask);
	}
	CPlayerTask* makeItChaseKickV1(const int num,double faceDir,int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = faceDir;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->ChaseKickV1(playerTask);
	}

	CPlayerTask* makeItChaseKickV2(const int num,double faceDir,int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = faceDir;
		//playerTask.player.ischipkick = isChip;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->ChaseKickV2(playerTask);
	}

	CPlayerTask* makeItChaseKickV3(const int num, double faceDir, int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = faceDir;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->NoneTrajGetBallNew(playerTask);
	}

	CPlayerTask* makeItProtectBall(const int num,const int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->ProtectBall(playerTask);
	}

	CPlayerTask* makeItTouchKick(const int num,double kickDir, bool isPass, int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = kickDir;
		playerTask.player.ispass = isPass;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->TouchKick(playerTask);
	}

	CPlayerTask* makeItInterKickV2(const int num, const double dir, const int flags, const bool isChip)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = dir;
		playerTask.player.flag = flags;
		playerTask.player.ischipkick = isChip;
		

		//		playerTask.player.pos = target;
		return TaskFactoryV2::Instance()->InterceptBallV2(playerTask);
	}


	CPlayerTask* makeItInterKickV3(const int num, const double dir, const int flags,const bool kickway)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = dir;
		playerTask.player.flag = flags;
		playerTask.player.ischipkick = kickway;

		//		playerTask.player.pos = target;
		return TaskFactoryV2::Instance()->InterceptBallV3(playerTask);
	}

	CPlayerTask* makeItInterKickV4(const int num, const double dir, const bool kickway, const double ipower, const int flags,const bool needKick,const bool needDribble)        //由C++里面开射门的InterceptBall
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = dir;
		playerTask.player.flag = flags;
		playerTask.player.ischipkick = kickway;
		playerTask.player.kickpower = ipower;
		playerTask.player.chipkickpower = ipower;
		playerTask.player.needkick = needKick;
		playerTask.player.needdribble = needDribble;
		//		playerTask.player.pos = target;
		return TaskFactoryV2::Instance()->InterceptBallV4(playerTask);
	}

	CPlayerTask* makeItInterKickV5(const int num, const double dir, const int flags, const bool kickway)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = dir;
		playerTask.player.flag = flags;
		playerTask.player.ischipkick = kickway;

		return TaskFactoryV2::Instance()->InterceptBallV5(playerTask);
	}

	CPlayerTask* makeItInterKickV6(const int num, const double dir, const int flags, const bool kickway)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = dir;
		playerTask.player.flag = flags;
		playerTask.player.ischipkick = kickway;

		return TaskFactoryV2::Instance()->InterceptBallV6(playerTask);
	}

	CPlayerTask* makeItMarkingTouch(const int num,const double kickDir,const CGeoPoint leftUpPos,const CGeoPoint rightDownPos,const int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = kickDir;
		playerTask.player.flag = flags;
		playerTask.ball.pos=leftUpPos;
		playerTask.player.pos=rightDownPos;
		return TaskFactoryV2::Instance()->MarkingTouch(playerTask);
	}

	CPlayerTask* makeItMarkingFront(const int num,const int markNum,const double kickDir,const int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.ball.Sender=markNum;
		playerTask.player.angle = kickDir;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->MarkingFront(playerTask);
	}

	CPlayerTask* makeItDriftKick(const int num,double faceDir,int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = faceDir;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->DriftKick(playerTask);
	}

	CPlayerTask* makeItAdvanceBallV1(const int num,const int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->AdvanceBallV1(playerTask);
	}
	CPlayerTask* makeItAdvanceBallV2(const int num, const int flags, const int tendemNum)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.flag = flags;
		playerTask.ball.receiver = tendemNum;
		return TaskFactoryV2::Instance()->AdvanceBallV2(playerTask);
	}
	CPlayerTask* makeItTandem(const int num, const int robber, const int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.ball.receiver = robber;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->Tandem(playerTask);
	}

	CPlayerTask* makeItGoAndTurnKick(const int num ,const double targetdir,const int itype, const int power)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = targetdir;
		playerTask.player.flag = itype;
		playerTask.player.chipkickpower = power;
		return TaskFactoryV2::Instance()->GoAndTurnKick(playerTask);
	}

	CPlayerTask* makeItGetBallV4(const int num, const double targetdir, const int flag)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.flag = flag;
		playerTask.player.angle = targetdir;
		return TaskFactoryV2::Instance()->GetBallV4(playerTask);
	}

	CPlayerTask* makeItCircleAndPass(const int num, const double targetdir, const int itype, const int power)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = targetdir;
		playerTask.player.flag = itype;
		playerTask.player.chipkickpower = power;
		return TaskFactoryV2::Instance()->CircleAndPass(playerTask);
	}

	CPlayerTask* makeItTestCircleBall(const int num ,const double targetdir,const double kickprecision,const int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = targetdir;
		playerTask.player.flag = flags;
		playerTask.player.kickprecision = kickprecision;
		return TaskFactoryV2::Instance()->TestCircleBall(playerTask);
	}
	CPlayerTask* makeItGoAndTurnKickV3(const int num ,const double targetdir,int circleNum,const double fixAngle,const double maxAcc,const int radius,const int numPerCir,const double gotoPre,const double gotoDist,const double adjustPre,const double kickprecision,const int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = targetdir;
		playerTask.player.flag = flags;
		playerTask.ball.angle = circleNum;
		playerTask.player.kickprecision = kickprecision;
		playerTask.player.rotate_speed = fixAngle;
		playerTask.player.max_acceleration = maxAcc;
		playerTask.player.rotvel = radius;
		playerTask.player.chipkickpower = numPerCir;
		playerTask.player.kickpower = gotoPre;
		playerTask.player.max_deceleration = gotoDist;
		playerTask.player.speed_x = adjustPre;
		return TaskFactoryV2::Instance()->GoAndTurnKickV3(playerTask);
	}
	CPlayerTask* makeItGoAndTurn(const int num ,const double targetdir,const double kickprecision,const int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = targetdir;
		playerTask.player.flag = flags;
		playerTask.player.kickprecision = kickprecision;
		return TaskFactoryV2::Instance()->GoAndTurn(playerTask);
	}
	CPlayerTask* makeItGoAroundRobot(const int num,const double faceDir,const CGeoPoint circleCenter,int circleDir,double radius, int flag)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = faceDir;
		playerTask.player.rotdir = circleDir;
		playerTask.player.pos = circleCenter;
		playerTask.player.rotvel = radius;
		return TaskFactoryV2::Instance()->GoAroundRobot(playerTask);
	}
	CPlayerTask* makeItSlowGetBall(const int num, const double dir, const int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = dir;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->SlowGetBall(playerTask);
	}
	CPlayerTask* makeItTimeDelayTest(const int num, const int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->timeDelayTest(playerTask);
	}
	CPlayerTask* makeItMarkEnemy(const int num,const int priority,const bool front, const int flags,const CGeoPoint pos,const double dir)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.ball.Sender = priority;
		playerTask.ball.front = front;
		playerTask.player.flag = flags;
		playerTask.player.pos = pos;
		playerTask.player.angle = dir;
		return TaskFactoryV2::Instance()->Marking(playerTask);
	}
	CPlayerTask* makeItShootBall(const int num, const double dir, const bool ischipkick, const double precision, const double kp, const double cp, const int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.ischipkick = ischipkick;
		playerTask.player.angle = dir;
		playerTask.player.kickprecision = precision;
		playerTask.player.kickpower = kp;
		playerTask.player.chipkickpower = cp;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->ShootBall(playerTask);
	}
	CPlayerTask* makeItShootBallV2(const int num, const double dir, const int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = dir;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->ShootBallV2(playerTask);
	}
	CPlayerTask* makeItReceivePass(const int num, const double dir, const int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = dir;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->ReceivePass(playerTask);
	}
	CPlayerTask* makeItPenaltyGoalie(const int num, const int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->PenaltyDefV1(playerTask);
	//	return TaskFactoryV2::Instance()->PenaltyGoalie(playerTask);
	}
	CPlayerTask* makeItPenaltyGoalieV2(const int num, const int flags) {
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->PenaltyDefV2(playerTask);
	}

	CPlayerTask* makeItPenaltyGoalie2017V1(const int num, const int flags)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->PenaltyDef2017V1(playerTask);
		//	return TaskFactoryV2::Instance()->PenaltyGoalie(playerTask);
	}
	CPlayerTask* makeItPenaltyGoalie2017V2(const int num, const int flags) {
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->PenaltyDef2017V2(playerTask);
	}

	CPlayerTask* makeItGoalie2013(const int num,const int flags){
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->Goalie2013(playerTask);
	}

	CPlayerTask* makeItGoalie2015(const int num, const int flags){
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->Goalie2015(playerTask);
	}

	CPlayerTask* makeItGoalie2015V2(const int num, const int flags){
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.flag = flags;
		return TaskFactoryV2::Instance()->Goalie2015V2(playerTask);
	}

	CPlayerTask* makeItPidToPosition(const int num, const CGeoPoint& target,const double angle){
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.pos = target;
		playerTask.player.angle = angle;
		return TaskFactoryV2::Instance()->PidToPosition(playerTask);
	}

	CPlayerTask* makeItInterceptBall(const int num, const double add_angle)
	{
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = add_angle;
		return TaskFactoryV2::Instance()->InterceptBall(playerTask);
	}

	CPlayerTask* makeItWaitTouch(const int num,const CGeoPoint& target, const double dir,const double angle,const int flag /* = 0 */){
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.ball.pos = target;
		playerTask.ball.angle = angle;
		playerTask.player.angle=dir;
		playerTask.player.flag=flag;
		return TaskFactoryV2::Instance()->WaitTouch(playerTask);
	}

	CPlayerTask* makeItWaitTouch2016(const int num, const CGeoPoint& target,const double dir, const int flag /* = 0 */){
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.ball.pos = target;
		playerTask.ball.angle = dir;
		playerTask.player.flag = flag;
		return TaskFactoryV2::Instance()->WaitTouch2016(playerTask);
	}

	CPlayerTask* makeItPenaltyKick2013(const int num, const int flag /* = 0 */){
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.flag = flag;
		return TaskFactoryV2::Instance()->PenaltyKick2013(playerTask);
	}

	CPlayerTask* makeItPenaltyKick2014(const int num, const int flag /* = 0 */){
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.flag = flag;
		return TaskFactoryV2::Instance()->PenaltyKick2014(playerTask);
	}

	CPlayerTask* makeItPenaltyKick2017V1(const int num, const int flag /* = 0 */) {
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.flag = flag;
		return TaskFactoryV2::Instance()->PenaltyKick2017V1(playerTask);
	}

	CPlayerTask* makeItPenaltyKick2017V2(const int num, const int flag /* = 0 */) {
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.flag = flag;
		return TaskFactoryV2::Instance()->PenaltyKick2017V2(playerTask);
	}

	CPlayerTask* makeItDribbleTurn(const int num, const double finalDir, const double pre) {
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = finalDir;
		playerTask.player.speed_x = pre;
		return TaskFactoryV2::Instance()->DribbleTurn(playerTask);
	}

	CPlayerTask* makeItDribbleTurnKick(const int num, const double finalDir, const double turnRotVel,const double kickPower) {
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = finalDir;
		playerTask.player.rotvel=turnRotVel;
		playerTask.player.kickpower=kickPower;
		return TaskFactoryV2::Instance()->DribbleTurnKick(playerTask);
	}

	CPlayerTask* makeItProtectTurn(const int num, const double finalDir, const double turnRotVel,const double kickPower) {
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.angle = finalDir;
		playerTask.player.rotvel=turnRotVel;
		playerTask.player.kickpower=kickPower;
		return TaskFactoryV2::Instance()->ProtectTurn(playerTask);
	}

	CPlayerTask* makeItInterceptTouch(const int num, const CGeoPoint& waitPos, const double touchDir, const double power, const double buffer, const bool useChip,const bool testMode) {
		static TaskT playerTask;
		playerTask.executor = num;
		playerTask.player.pos = waitPos;
		playerTask.player.angle = touchDir;
		playerTask.player.kickpower = power;
		playerTask.player.chipkickpower = buffer;  // 截球时后退的距离
		playerTask.player.ischipkick = useChip;
		playerTask.ball.front = testMode; // testMode use for testing compensation. !!!!!!!!!!
		return TaskFactoryV2::Instance()->InterceptTouch(playerTask);
	}
}