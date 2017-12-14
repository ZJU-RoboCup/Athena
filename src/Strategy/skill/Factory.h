#ifndef _TASK_FACTORY_V2_H_
#define _TASK_FACTORY_V2_H_

#include <singleton.h>
#include <misc_types.h>
#include "param.h"
/************************************************************************/
/*                         CTaskFactoryV2                               */
/************************************************************************/

class CPlayerTask;
class CTaskFactoryV2{
public:
	CPlayerTask* GotoPosition(const TaskT& task);
	CPlayerTask* SmartGotoPosition(const TaskT& task);
	CPlayerTask* GoAvoidShootLine(const TaskT& task);
	CPlayerTask* InterceptBallV2(const TaskT& task);
	CPlayerTask* InterceptBallV3(const TaskT& task);
	CPlayerTask* InterceptBallV4(const TaskT& task);
	CPlayerTask* InterceptBallV5(const TaskT& task);
	CPlayerTask* InterceptBallV6(const TaskT& task);
	CPlayerTask* StopRobot(const TaskT& task);
	CPlayerTask* Speed(const TaskT& task); // by zc
	CPlayerTask* OpenSpeed(const TaskT& task);
	CPlayerTask* Marking(const TaskT& task);
	CPlayerTask* NoneTrajGetBall(const TaskT& task);
	CPlayerTask* NoneTrajGetBallNew(const TaskT& task);
	CPlayerTask* ForceStartRush(const TaskT& task);	
	CPlayerTask* ChaseKickV1(const TaskT& task);
	CPlayerTask* ChaseKickV2(const TaskT& task);
	CPlayerTask* ProtectBall(const TaskT& task);
	CPlayerTask* TouchKick(const TaskT& task);
	CPlayerTask* MarkingTouch(const TaskT& task);
	CPlayerTask* MarkingFront(const TaskT& task);
	CPlayerTask* GoAroundBall(const TaskT& task);
	CPlayerTask* PenaltyDefV1(const TaskT& task);
	CPlayerTask* PenaltyDefV2(const TaskT& task);
	CPlayerTask* PenaltyDef2017V1(const TaskT& task);
	CPlayerTask* PenaltyDef2017V2(const TaskT& task);
	

	CPlayerTask* AdvanceBallV1(const TaskT& task);
	CPlayerTask* AdvanceBallV2(const TaskT& task);
	CPlayerTask* Tandem(const TaskT& task);
	CPlayerTask* GoAndTurnKick(const TaskT& task);
	CPlayerTask* StaticGetBall(const TaskT& task);
	CPlayerTask* GetBallV4(const TaskT& task);
	CPlayerTask* CircleAndPass(const TaskT& task);
	CPlayerTask* GoAndTurnKickV3(const TaskT& task);
	CPlayerTask* GoAndTurnKickV4(const TaskT& task);

	CPlayerTask* GoAndTurn(const TaskT& task);
	CPlayerTask* GoAroundRobot(const TaskT& task);
	CPlayerTask* SlowGetBall(const TaskT& task);

	CPlayerTask* timeDelayTest(const TaskT& task);
	CPlayerTask* TestCircleBall(const TaskT& task);
	CPlayerTask* CarzyPush(const  TaskT& task);

	CPlayerTask* ShootBall(const TaskT& task);
	CPlayerTask* ShootBallV2(const TaskT& task);
	CPlayerTask* PassBall(const TaskT& task);
	CPlayerTask* ReceivePass(const TaskT& task);
	CPlayerTask* PenaltyGoalie(const TaskT& task);
	CPlayerTask* Goalie2013(const TaskT& task);
	CPlayerTask* Goalie2015(const TaskT& task);
	CPlayerTask* Goalie2015V2(const TaskT& task);
	CPlayerTask* PidToPosition(const TaskT& task);
	CPlayerTask* InterceptBall(const TaskT& task);

	CPlayerTask* WaitTouch(const TaskT& task);
	CPlayerTask* WaitTouch2016(const TaskT& task);

	CPlayerTask* PenaltyKick2013(const TaskT& task);
	CPlayerTask* PenaltyKick2014(const TaskT& task);
	CPlayerTask* PenaltyKick2017V1(const TaskT& task);
	CPlayerTask* PenaltyKick2017V2(const TaskT& task);

	CPlayerTask* DribbleTurn(const TaskT& task);
	CPlayerTask* DriftKick(const TaskT& task);
	CPlayerTask* DribbleTurnKick(const TaskT& task);
	CPlayerTask* ProtectTurn(const TaskT& task);
	CPlayerTask* InterceptTouch(const TaskT& task);
protected:
	template < class CTaskType >
	CPlayerTask* MakeTask( const TaskT& task );
};
typedef NormalSingleton<CTaskFactoryV2> TaskFactoryV2;
/************************************************************************/
/*                 一些包裹(wrap)函数, 方便调用tasks                    */
/************************************************************************/

//! 调用skill的接口
namespace PlayerRole{
	CPlayerTask* makeItStop(const int num, const int flags = 0);
	CPlayerTask* makeItGoto(const int num, const CGeoPoint& target, const double dir, const int flags = 0, const int sender = 0);
	CPlayerTask* makeItGoto(const int num, const CGeoPoint& target, const double dir, const CVector& vel, const double rotvel, const int flags = 0, const int sender = 0);
	CPlayerTask* makeItSimpleGoto(const int num, const CGeoPoint& target, const double dir, const int flags = 0);
	CPlayerTask* makeItSimpleGoto(const int num, const CGeoPoint& target, const double dir, const CVector& vel, const double rotvel, const int flags=0);
	CPlayerTask* makeItGoAvoidShootLine(const int num, const CGeoPoint& target, const double dir, const CVector& vel, const double rotvel, const int flags = 0,const int sender = 0);
	CPlayerTask* makeItInterKickV2(const int num, const double dir, const int flags, const bool isChip = false);
	CPlayerTask* makeItInterKickV3(const int num, const double dir, const int flags, const bool kickway = 0);
	CPlayerTask* makeItInterKickV5(const int num, const double dir, const int flags, const bool kickway = 0);
	CPlayerTask* makeItInterKickV6(const int num, const double dir, const int flags, const bool kickway = 0);
	CPlayerTask* makeItInterKickV4(const int num, const double dir , const bool kickway = 0, const double ipower = 9999, const int flags = 0, const bool needKick = true, const bool needDribble = false);
	CPlayerTask* makeItRun(const int num, const double speedX, const double speedY, const double rotSpeed, int flags = 0); // 直接发速度,by zc
	CPlayerTask* makeItRunLocalVersion(const int num, const double speedX, const double speedY, const double rotSpeed);  // 发开环的小车坐标系速度
	CPlayerTask* makeItCrazyPush(const int num, double faceDir);
	CPlayerTask* makeItNoneTrajGetBall(const int num,const double dir,CVector finalVel = CVector(0.0,0.0),int flags = 0,double StopDist = -2, CTRL_METHOD mode = CMU_TRAJ);
	CPlayerTask* makeItNoneTrajGetBallNew(const int num, const double dir, CVector finalVel = CVector(0.0, 0.0), int flags = 0, double StopDist = -2, CTRL_METHOD mode = CMU_TRAJ);
	CPlayerTask* makeItGoAndTurnKickV4(const int num,const double dir,CVector finalVel = CVector(0.0,0.0),int flags = 0,double StopDist = -2, CTRL_METHOD mode = CMU_TRAJ);
	CPlayerTask* makeItForceStartRush(const int num,double faceDir,int flags = 0);
	CPlayerTask* makeItChaseKickV1(const int num, double faceDir, int flags = 0);
	CPlayerTask* makeItChaseKickV1(const int num, double faceDir, int flags,bool isChip);//重载 add by gty 17-3-26
	CPlayerTask* makeItChaseKickV2(const int num, double faceDir, int flags = 0);
	CPlayerTask* makeItChaseKickV3(const int num, double faceDir, int flags = 0);
	CPlayerTask* makeItDriftKick(const int num,double faceDir,int flags = 0);
	CPlayerTask* makeItProtectBall(const int num,int flags=0);
	CPlayerTask* makeItTouchKick(const int num,double kickDir, bool isPass = false, int flags=0);
	CPlayerTask* makeItMarkingTouch(const int num,const double kickDir,const CGeoPoint leftUpPos,const CGeoPoint rightDownPos,const int flags);
	CPlayerTask* makeItMarkingFront(const int num,const int markNum,const double kickDir,const int flags);
	CPlayerTask* makeItAdvanceBallV1(const int num, const int flags = 0);
	CPlayerTask* makeItAdvanceBallV2(const int num, const int flags = 0,  const int tendemNum = 0);
	CPlayerTask* makeItTandem(const int num, const int robber = 1, const int flags = 0);
	CPlayerTask* makeItGoAndTurnKick(const int num, const double targetdir, const int itype, const int power);
	CPlayerTask* makeItStaticGetBall(const int num, const double dir, CVector finalVel, int flags, double StopDist, CTRL_METHOD mode);
	CPlayerTask* makeItGetBallV4(const int num, const int flag = 0);
	CPlayerTask* makeItCircleAndPass(const int num, const double targetdir, const int itype, const int power);
	CPlayerTask* makeItGoAndTurnKickV3(const int num ,const double targetdir,int circleNum,const double fixAngle,const double maxAcc,const int radius,const int numPerCir,const double gotoPre,const double gotoDist,const double adjustPre,const double kickprecision = Param::Math::PI*5/180,const int flags = 0);

	CPlayerTask* makeItGoAndTurn(const int num ,const double targetdir,const double kickprecision = Param::Math::PI*5/180,const int flags = 0);
	CPlayerTask* makeItTestCircleBall(const int num ,const double targetdir,const double kickprecision = Param::Math::PI*5/180,const int flags = 0);
	CPlayerTask* makeItGoAroundRobot(const int num ,const double faceDir,const CGeoPoint circleCenter,int circleDir, double radius = 25, int flags = 0);
	CPlayerTask* makeItSlowGetBall(const int num, const double dir, const int flags = 0);
	CPlayerTask* makeItTimeDelayTest(const int num, const int flags = 0);
	CPlayerTask* makeItMarkEnemy(const int num,const int priority,const bool front = false,const int flags = 0,const CGeoPoint pos = CGeoPoint(1000,1000),const double dir=999);
	CPlayerTask* makeItShootBall(const int num, const double dir, const bool ischipkick, const double precision, const double kp, const double cp, const int flags = 0);
	CPlayerTask* makeItShootBallV2(const int num, const double dir, const int flags = 0);
	CPlayerTask* makeItReceivePass(const int num, const double dir, const int flags = 0);
	CPlayerTask* makeItPenaltyGoalie(const int num, const int flags = 0);
	CPlayerTask* makeItPenaltyGoalieV2(const int num, const int flags = 0);
	CPlayerTask* makeItPenaltyGoalie2017V1(const int num, const int flags = 0);
	CPlayerTask* makeItPenaltyGoalie2017V2(const int num, const int flags = 0);
	CPlayerTask* makeItGoalie2013(const int num,const int flag = 0);
	CPlayerTask* makeItGoalie2015(const int num, const int flag = 0);
	CPlayerTask* makeItGoalie2015V2(const int num, const int flag = 0);
	CPlayerTask* makeItPidToPosition(const int num, const CGeoPoint& target, const double angle);
	CPlayerTask* makeItInterceptBall(const int num, const double add_angle);
	CPlayerTask* makeItWaitTouch(const int num,const CGeoPoint& target, const double dir,const double angle,const int flag = 0);
	CPlayerTask* makeItWaitTouch2016(const int num, const CGeoPoint& target, const double angle, const int flag = 0);
	CPlayerTask* makeItPenaltyKick2013(const int num, const int flag = 0);
	CPlayerTask* makeItPenaltyKick2014(const int num, const int flag = 0);
	CPlayerTask* makeItPenaltyKick2017V1(const int num, const int flag = 0);
	CPlayerTask* makeItPenaltyKick2017V2(const int num, const int flag = 0);
	CPlayerTask* makeItDribbleTurn(const int num, const double finalDir, const double pre = Param::Math::PI / 90);
	CPlayerTask* makeItDribbleTurnKick(const int num, const double finalDir, const double turnRotVel,const double kickPower);
	CPlayerTask* makeItProtectTurn(const int num, const double finalDir, const double turnRotVel,const double kickPower);
	CPlayerTask* makeItInterceptTouch(const int num, const CGeoPoint& waitPos, const double touchDir, const double power, const double buffer, const bool useChip,const bool);
}

#endif // _TASK_FACTORY_V2_H_
