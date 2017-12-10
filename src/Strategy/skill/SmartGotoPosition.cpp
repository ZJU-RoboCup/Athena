#include "SmartGotoPosition.h"
#include <PathPlanner.h>
#include <VisionModule.h>
#include "skill/Factory.h"
#include <utils.h>
#include <tinyxml/ParamReader.h>
#include <TaskMediator.h>
//#include <LeavePenaltyArea.h>
#include "PlayInterface.h"
#include <GDebugEngine.h>
#include <RobotCapability.h>
#include <CommandFactory.h>
#include <TimeCounter.h>
#include <ControlModel.h>
#include <iostream>


/************************************************************************/
/*                                                                      */
/************************************************************************/
namespace{
	/// 调试开关
	bool VERBOSE_MODE = false;
	bool DRAW_TARGET = false;

	/// 路径规划
	double NO_SMART_DIST = 5;
	double NO_SMART_DIST_IN_THEIR_DEFENCE_BOX = 5;
	double DO_NOT_AVOID_VEHICLE_EXTRA = Param::Field::MAX_PLAYER;
	double PATH_VALID_DIST = 2; // 小于此距离便认为路径点不可用
	int PATH_PLAN_TYPE = 0;  /* RRT = 0,GEO = 1,ASTAR = 2,STUPID = 3, BANGBANG = 4, PFAD = 5,DELAUNAY = 6,NEWGEO = 7,*/

	//int REPlanCount = 3;
	//int PATH_PLAN_CYCLE[Param::Field::MAX_PLAYER+1] = {-20,-20,-20,-20,-20,-20,-20};
	//CGeoPoint PATH_PLAN_RECORD_POINT[Param::Field::MAX_PLAYER+1] = {CGeoPoint(0,0),CGeoPoint(0,0),CGeoPoint(0,0),CGeoPoint(0,0),CGeoPoint(0,0),CGeoPoint(0,0),CGeoPoint(0,0)};

	/// 避碰规划
	bool DO_CHECK = false;		// 控制是否进行碰撞检测
	int TRAJECTORY_METHORD = 4;	// CMU_TRAJ = 1,ZERO_FINAL = 2,	ZERO_TRAP = 3,	NONE_TRAP = 4
	int MaxCheckSearchNum = 0;
	double CheckSearchStep = Param::Field::MAX_PLAYER_SIZE*1.5;
	int TRAP_COLLISION_BUFFER = 0;
	int TRAP_STEP = 6;
	double NeedCheckDist = 100;
	double MoveCritialSpeed = 50;
	double FuzzyStep = 2.5;
	double FuzzyBaseSpeed = 150;

	bool DRAW_TRAJ = false;		//Draw
	bool DRAW_PREDICT_POS = false;
	bool DRAW_TMP_ADDED = false;

	/// 
}
using namespace Param::Vehicle::V2;


/************************************************************************/
/*                       CSmartGotoPositionV2                           */
/************************************************************************/
/// 构造函数 ： 参数初始化
CSmartGotoPositionV2::CSmartGotoPositionV2()
{
	{
		DECLARE_PARAM_READER_BEGIN(CSmartGotoPositionV2)
		READ_PARAM(NO_SMART_DIST)
			READ_PARAM(PATH_VALID_DIST)
			READ_PARAM(DRAW_TARGET)
			READ_PARAM(PATH_PLAN_TYPE)
			READ_PARAM(DO_CHECK)
		DECLARE_PARAM_READER_END
	}
	{
		DECLARE_PARAM_READER_BEGIN(CGotoPositionV2)
			READ_PARAM(TRAJECTORY_METHORD)
		DECLARE_PARAM_READER_END
	}	
}

/// 输出流 ： 调试显示
void CSmartGotoPositionV2::toStream(std::ostream& os) const
{
	os << "Smart going to " << task().player.pos;
}

/// 规划接口
void CSmartGotoPositionV2::plan(const CVisionModule* pVision)
{
	/************************************************************************/
	/* 任务参数解析                                                         */
	/************************************************************************/
	playerFlag = task().player.flag;
	const bool rec = task().player.needdribble;
	const int vecNumber = task().executor;
	const PlayerVisionT& self = pVision->OurPlayer(vecNumber);
	const CGeoPoint& myPos = pVision->OurPlayer(vecNumber).Pos();
	const CVector&myVel = pVision->OurPlayer(vecNumber).Vel();
	const CGeoPoint finalTargetPos = task().player.pos;
	GDebugEngine::Instance()->gui_debug_x(finalTargetPos, COLOR_RED);
	

	/************************************************************************/
	/* 任务参数解析                                                         */
	/************************************************************************/
	CGeoPoint middlePoint =  myPos + (task().player.pos - myPos) * 0.2; // 初始点		
	/// 路径规划
	//int buffer = Param::Field::BALL_SIZE;
	int buffer = 2.5;
	PlanType play_type = (PlanType)PATH_PLAN_TYPE;
	if (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie())) {
		buffer = -6;
		play_type = RRT;
	}
	//CUsecTimer timer;
	//timer.start();
	CPathPlanner planner(pVision,task(), Param::Vehicle::V2::PLAYER_SIZE + buffer, play_type);
	//timer.stop();
	//printf("Cost Time: %f\n", timer.time() / 1000.0);

	if (planner.getPath().size() > 0) { // 规划成功
		CGeoPoint plannedPoint(*(planner.getPath().begin()));
		if (plannedPoint.dist(myPos) >= (Param::Vehicle::V2::PLAYER_SIZE * 2 + 2) || plannedPoint.dist(finalTargetPos) <= Param::Vehicle::V2::PLAYER_SIZE * 0.5) {
				middlePoint = plannedPoint;
		}			
	}		
	if (rec) DribbleStatus::Instance()->setDribbleCommand(vecNumber,2);
	TaskT newTask(task());
	newTask.player.pos = middlePoint;
	setSubTask(TaskFactoryV2::Instance()->GotoPosition(newTask));
	CPlayerTask::plan(pVision);
	return ;
}
//
///// 轨迹碰撞检查
//void CSmartGotoPositionV2::doTrajCheck(const CVisionModule* pVision,int playerNum,const PlayerPoseT& middlePose,PlayerPoseT& nextStep)
//{
//	//player
//	const PlayerVisionT& player = pVision->OurPlayer(playerNum);
//
//	//motion-control
//	//1.param read
//	const CRobotCapability* robotCap = RobotCapFactory::Instance()->getRobotCap(pVision->Side(), playerNum);
//	double MaxSpeed = robotCap->maxSpeed(0);		//speed
//	if( playerFlag & PlayerStatus::SPECIFY_SPEED )			
//		MaxSpeed = task().player.max_speed;
//	double MaxAcceleration = robotCap->maxAcceleration(CVector(0,0),0);	//acceleration
//	if( playerFlag & PlayerStatus::SPECIFY_ACCELERATION )
//		MaxAcceleration = task().player.max_acceleration;
//	double MaxDeceleration = MaxAcceleration;				//deceleration
//	
//	double MaxAngularSpeed = robotCap->maxAngularSpeed();	//ang-speed
//	if( playerFlag & PlayerStatus::SPECIFY_ROTATE_SPEED )
//		MaxAngularSpeed = task().player.max_rotate_speed;
//	double MaxAngularAcceleration = robotCap->maxAngularAcceleration();	//ang-acceleration
//	if( playerFlag & PlayerStatus::SPECIFY_ROTATE_ACCELERATION )
//		MaxAngularAcceleration = task().player.max_rotate_acceleration;	//ang-acceleration
//	double MaxAngularDeceleration = MaxAngularAcceleration;
//	
//	if( playerFlag & PlayerStatus::SLOWLY )
//	{
//		MaxSpeed *= 0.6;
//	}
//	//std::cout<<MaxSpeed<<"\t"<<MaxAcceleration<<"\t"<<MaxDeceleration<<"\t"<<MaxAngularSpeed<<"\t"<<MaxAngularAcceleration<<"\t"<<MaxAngularDeceleration<<std::endl;
//
//	//2.generate
//	PlayerPoseT start;				//start
//	start.SetPos(player.Pos());
//	start.SetVel(player.Vel());
//	start.SetDir(player.Dir());
//	start.SetRotVel(player.RotVel());
//	PlayerPoseT final;				//final
//	final.SetPos(middlePose.Pos());
//	final.SetVel(middlePose.Vel());
//	final.SetDir(middlePose.Dir());
//	final.SetRotVel(0.0);
//	PlayerCapabilityT capability;	//capability
//	capability.maxSpeed = MaxSpeed;
//	capability.maxAccel = MaxAcceleration;
//	capability.maxDec = MaxDeceleration;
//	capability.maxAngularSpeed = MaxAngularSpeed;
//	capability.maxAngularAccel = MaxAngularAcceleration;
//	capability.maxAngularDec = MaxAngularDeceleration;
//
//	//根据 TRAJECTORY_METHORD 选择对应的控制算法
//	CControlModel control;
//	switch ( TRAJECTORY_METHORD )
//	{
//	case CMU_TRAJ:
//		control.makeCmTrajectory(start,final,capability);				//没有完整轨迹返回 TODO		这个还比较纠结，还没整明白
//		break;
//	case ZERO_FINAL:
//		control.makeZeroFinalVelocityPath(start, final, capability);	//没有完整轨迹返回 TODO		可以很方便通过改写返回
//		break;
//	case ZERO_TRAP:
//		control.makeTrapezoidalVelocityPath(start,final,capability);
//		break;
//	case NONE_TRAP:
//		control.makeNoneTrapezoidalVelocityPath(start, final, capability);
//		break;
//	default:
//		control.makeNoneTrapezoidalVelocityPath(start, final, capability);
//		break;
//	}  		
//	nextStep = control.getNextStep();
//
//	//do check : iterate
//	current_trajectory_safe = true;
//	collidePos = CGeoPoint(0.0,0.0);
//	collideVel = CVector(0,0);
//	collideRot = 0.0;
//	collideNum = 0;
//	vector< vector<double> > nonepathList = control.getFullPath();
//	int index = 0;
//	for( vector< vector<double> >::const_iterator iter = nonepathList.begin();
//			iter != nonepathList.end() && index < Param::Vision::FRAME_RATE ; iter++,index++)
//	{
//		CGeoPoint realPoint = CGeoPoint( (*iter)[1]+player.Pos().x(),(*iter)[2]+player.Pos().y());
//
//		if(DRAW_TRAJ)
//			GDebugEngine::Instance()->gui_debug_x(realPoint,COLOR_CYAN);
//
//		bool isCollide = false;
//		for(int i=1;i<=2*Param::Field::MAX_PLAYER;i++)
//		{
//			//not me
//			if( i == playerNum )
//				continue;
//
//			//... ... Higher Priority : not added yet
//			//... ...
//
//			bool isOnField = (i<=Param::Field::MAX_PLAYER)?pVision->OurPlayer(i).Valid():
//								pVision->TheirPlayer(i-Param::Field::MAX_PLAYER).Valid();
//			//on field
//			if( !isOnField )
//				continue;
//
//			CGeoPoint currentPos = (i<=Param::Field::MAX_PLAYER)?pVision->OurPlayer(i).Pos():
//									pVision->TheirPlayer(i-Param::Field::MAX_PLAYER).Pos();
//			CVector currentVel = (i<=Param::Field::MAX_PLAYER)?pVision->OurPlayer(i).Vel():
//									pVision->TheirPlayer(i-Param::Field::MAX_PLAYER).Vel();
//
//			//in some distance
//			if( player.Pos().dist(currentPos) > NeedCheckDist )
//				continue;
//
//			//consider dynamic
//			if( index <= 1 )
//				currentPos = currentPos + currentVel*((index+1)*1.0/Param::Vision::FRAME_RATE);
//			else
//				currentPos = currentPos + currentVel*(1.0*((index-1)*TRAP_STEP+2)/Param::Vision::FRAME_RATE);
//
//			//fuzzy about speed
//			double SpeedBuffer = 0.0;
//			if( currentVel.mod2() > MoveCritialSpeed*MoveCritialSpeed )
//				SpeedBuffer = index * FuzzyStep * currentVel.mod() / FuzzyBaseSpeed;
//
//			double collideCheckBufferDist = Param::Vehicle::V2::PLAYER_SIZE*2 + TRAP_COLLISION_BUFFER + SpeedBuffer;
//
//			if( DRAW_PREDICT_POS)
//			{
//				if( i <= Param::Field::MAX_PLAYER )	//only ourside
//					GDebugEngine::Instance()->gui_debug_arc(currentPos,collideCheckBufferDist,0,360,COLOR_ORANGE);
//			}
//
//			if( realPoint.dist(currentPos) <= collideCheckBufferDist )
//			{
//				isCollide = true;
//				collidePos = realPoint;
//				collideVel = CVector((*iter)[4],(*iter)[5]);
//				collideRot = (*iter)[6];
//				collideNum = i;
//				break;							
//			}
//		}
//
//		if( isCollide )
//		{
//			current_trajectory_safe = false;
//			break;
//		}
//	}
//
//	return ;
//}

/// 生成避障跑位点
CGeoPoint CSmartGotoPositionV2::generateObstacleAvoidanceTmpPoint(const CVisionModule* pVision,int playerNum,const CGeoPoint& middlePoint)
{
	CGeoPoint collideVehiclePos = (collideNum <= Param::Field::MAX_PLAYER)?pVision->OurPlayer(collideNum).Pos():
									pVision->TheirPlayer(collideNum-Param::Field::MAX_PLAYER).Pos();
	CVector collideVehicleVel = (collideNum <= Param::Field::MAX_PLAYER)?pVision->OurPlayer(collideNum).Vel():
									pVision->TheirPlayer(collideNum-Param::Field::MAX_PLAYER).Vel();

	CGeoPoint selfPos = pVision->OurPlayer(playerNum).Pos();


	//若已经相撞
	if( selfPos.dist(collideVehiclePos) < 2*Param::Vehicle::V2::PLAYER_SIZE )
	{
		double avoid_dir = (selfPos - collideVehiclePos).dir();
		if( collideVehicleVel.mod() >= MoveCritialSpeed )
			return selfPos + Utils::Polar2Vector(100,Utils::Normalize(avoid_dir));
		else
			return generateObstacleAvoidanceStatic(pVision,playerNum,middlePoint);
	}
	else
	{
		if( collideVehicleVel.mod() >= MoveCritialSpeed )
			return generateObstacleAvoidanceDynamic(pVision,playerNum,middlePoint);
		else
			return generateObstacleAvoidanceStatic(pVision,playerNum,middlePoint);
	}	
}

/// 生成静态避障跑位点
CGeoPoint CSmartGotoPositionV2::generateObstacleAvoidanceStatic(const CVisionModule* pVision,int playerNum,const CGeoPoint& middlePoint)
{
	CGeoPoint collideVehiclePos = (collideNum <= Param::Field::MAX_PLAYER)?pVision->OurPlayer(collideNum).Pos():
									pVision->TheirPlayer(collideNum-Param::Field::MAX_PLAYER).Pos();
	CVector collideVehicleVel = (collideNum <= Param::Field::MAX_PLAYER)?pVision->OurPlayer(collideNum).Vel():
									pVision->TheirPlayer(collideNum-Param::Field::MAX_PLAYER).Vel();

	//设定偏移起始点以及偏移方向
	double offsideDir = 0.0;
	CGeoPoint tmpAddPoint = collidePos;

	CVector collidePos2collideVehiclePos = collidePos - collideVehiclePos;
	CGeoLine self2tmpPoint = CGeoLine(pVision->OurPlayer(playerNum).Pos(),middlePoint);
	CGeoPoint projPoint = self2tmpPoint.projection(collideVehiclePos);

	offsideDir = Utils::Normalize((projPoint-collideVehiclePos).dir());
	tmpAddPoint = tmpAddPoint + Utils::Polar2Vector(CheckSearchStep,offsideDir);

	return tmpAddPoint;
}

/// 生成动态避障跑位点
CGeoPoint CSmartGotoPositionV2::generateObstacleAvoidanceDynamic(const CVisionModule* pVision,int playerNum,const CGeoPoint& middlePoint)
{
	CGeoPoint collideVehiclePos = (collideNum <= Param::Field::MAX_PLAYER)?pVision->OurPlayer(collideNum).Pos():
									pVision->TheirPlayer(collideNum-Param::Field::MAX_PLAYER).Pos();
	CVector collideVehicleVel = (collideNum <= Param::Field::MAX_PLAYER)?pVision->OurPlayer(collideNum).Vel():
									pVision->TheirPlayer(collideNum-Param::Field::MAX_PLAYER).Vel();

	//设定偏移起始点以及偏移方向
	double offsideDir = 0.0;
	CGeoPoint tmpAddPoint = collidePos;

	CVector collidePos2collideVehiclePos = collidePos - collideVehiclePos;
	CGeoLine self2tmpPoint = CGeoLine(pVision->OurPlayer(playerNum).Pos(),middlePoint);
	CGeoPoint projPoint = self2tmpPoint.projection(collideVehiclePos);
	CGeoSegment collideVehicleSeg = CGeoSegment(collideVehiclePos,collideVehiclePos+Utils::Polar2Vector(1000,collideVehicleVel.dir()));

	CGeoPoint projectSelf = collideVehicleSeg.projection(pVision->OurPlayer(playerNum).Pos());
	bool NeedEmergencyStop = false;
	double dist = 0.0;
	if( collideVehicleSeg.IsPointOnLineOnSegment(projectSelf) )
	{
		dist = projectSelf.dist(pVision->OurPlayer(playerNum).Pos());
		if(  dist > Param::Vehicle::V2::PLAYER_SIZE*2 )
			NeedEmergencyStop = true;
	}

	CVector self2collodePos = collidePos - pVision->OurPlayer(playerNum).Pos();
	offsideDir = self2collodePos.dir();

	if( NeedEmergencyStop )	//急停
	{
		if( dist < 100 )
			tmpAddPoint = pVision->OurPlayer(playerNum).Pos() + Utils::Polar2Vector(0,offsideDir);
		else
			tmpAddPoint = pVision->OurPlayer(playerNum).Pos() + self2collodePos*0.5;

	}
	else
	{
		offsideDir = Utils::Normalize((projPoint-collideVehiclePos).dir());
		tmpAddPoint = collidePos + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_SIZE*2,offsideDir);
	}

	return tmpAddPoint;
}