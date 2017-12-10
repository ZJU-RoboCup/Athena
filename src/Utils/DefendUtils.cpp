#include "defendutils.h"
#include <param.h>
#include <iostream>
#include <cmath>
#include <sstream>
#include "WorldModel.h"
#include "TaskMediator.h"
#include <GDebugEngine.h>
#include "BestPlayer.h"
#include "BallSpeedModel.h"
#include "Global.h"
#include "CornerAreaPos.h"
#include "IndirectDefender.h"
#include "DefaultPos.h"
#include "PlayInterface.h"
#include "tinyxml/ParamReader.h"

namespace DefendUtils{
	double GoalieFrontBuffer = 0.5;
	double GoalBuffer = 2;
	double PLAYERSIZE = Param::Vehicle::V2::PLAYER_SIZE * 0.8; //本篇所用的player size  0.5
	CGeoPoint RGOAL_LEFT_POS = CGeoPoint(Param::Field::PITCH_LENGTH / 2, -Param::Field::GOAL_WIDTH / 2 - GoalBuffer);
	CGeoPoint RGOAL_RIGHT_POS = CGeoPoint(Param::Field::PITCH_LENGTH / 2, Param::Field::GOAL_WIDTH / 2 + GoalBuffer);
	CGeoPoint RPENALTY_LEFT_POS = CGeoPoint(Param::Field::PITCH_LENGTH / 2, -Param::Field::PENALTY_AREA_WIDTH / 2);
	CGeoPoint RPENALTY_RIGHT_POS = CGeoPoint(Param::Field::PITCH_LENGTH / 2, Param::Field::PENALTY_AREA_WIDTH / 2);
	CGeoPoint RGOAL_CENTRE_POS = CGeoPoint(Param::Field::PITCH_LENGTH / 2,0);
	CGeoPoint GOAL_CENTRE_POS = CGeoPoint(-Param::Field::PITCH_LENGTH / 2,0);
	CGeoPoint GOAL_LEFT_POS = CGeoPoint(-Param::Field::PITCH_LENGTH / 2, -Param::Field::GOAL_WIDTH / 2);
	CGeoPoint GOAL_RIGHT_POS = CGeoPoint(-Param::Field::PITCH_LENGTH / 2, Param::Field::GOAL_WIDTH / 2);
	CGeoPoint RLEFT = CGeoPoint(-Param::Field::PITCH_LENGTH / 2, Param::Field::PITCH_WIDTH / 2);
	CGeoPoint RRIGHT = CGeoPoint(-Param::Field::PITCH_LENGTH / 2, Param::Field::PITCH_WIDTH / 2);

	//一些计算用的中间点
	double AVOID_PENALTY_BUFFER = Param::Vehicle::V2::PLAYER_SIZE;//ori:7
	double PEN_RADIUS = Param::Field::PENALTY_AREA_R + Param::Vehicle::V2::PLAYER_SIZE + AVOID_PENALTY_BUFFER;
	double PEN_DEPTH = Param::Field::PENALTY_AREA_DEPTH + Param::Vehicle::V2::PLAYER_SIZE + AVOID_PENALTY_BUFFER;
	//防守队员 站位的 门前线段两边的两个极限点
	CGeoPoint RCENTER_LEFT = CGeoPoint(Param::Field::PITCH_LENGTH/2 - PEN_DEPTH,-Param::Field::PENALTY_AREA_L/2);
	CGeoPoint RCENTER_RIGHT = CGeoPoint(Param::Field::PITCH_LENGTH/2 - PEN_DEPTH,Param::Field::PENALTY_AREA_L/2);
	//底线两端的两个极限点
	CGeoPoint RBOTTOM_LEFT = CGeoPoint(Param::Field::PITCH_LENGTH/2,-PEN_RADIUS - Param::Field::PENALTY_AREA_L/2);
	CGeoPoint RBOTTOM_RIGHT = CGeoPoint(Param::Field::PITCH_LENGTH/2,PEN_RADIUS + Param::Field::PENALTY_AREA_L/2);
	//门前线段映射到底线的两个点
	CGeoPoint RCENTER2BOTTOM_LEFT = CGeoPoint(Param::Field::PITCH_LENGTH/2,-Param::Field::PENALTY_AREA_L/2);
	CGeoPoint RCENTER2BOTTOM_RIGHT = CGeoPoint(Param::Field::PITCH_LENGTH/2,Param::Field::PENALTY_AREA_L/2);

	const double BALL_SHOOT_DIR_BUFFER = Param::Math::PI * 10 / 180;		//朝向判断的余量，只有球射出时使用
	const double ENEMY_FACE_BUFFER = Param::Math::PI * 25 / 180;		//判断敌人是否面对我方球门的余量
	const double ENEMY_BALL_DIST_BUFFER = 20;                           //防守朝向的距离判据
	const double PENALTY_BUFFER = 12;									//禁区缓冲
	const double ENEMY_PASS_SPEED = 300;								//判断敌人传球速度限
	const double GOALIE_SELF_DIST = 25;								//后卫在离任务点该距离以外时，守门员孤军作战

	//后卫所在规划线
	CGeoLine RD_CENTER_SEGMENT = CGeoLine(RCENTER_LEFT,RCENTER_RIGHT);
	CGeoCirlce RD_CIR_LEFT = CGeoCirlce(RCENTER2BOTTOM_LEFT,PEN_RADIUS);
	CGeoCirlce RD_CIR_RIGHT = CGeoCirlce(RCENTER2BOTTOM_RIGHT,PEN_RADIUS);

	//middle及后卫（椭圆规划线）所在规划线原点
	CGeoPoint CENTERPOINT = CGeoPoint(Param::Field::PITCH_LENGTH/2,0);

	//后卫椭圆方式规划线
	const double AVOIDBUFFER = Param::Vehicle::V2::PLAYER_SIZE;

  // 1. 普通后卫
	CGeoEllipse RD_ELLIPSE  = CGeoEllipse(CENTERPOINT, Param::Field::PENALTY_AREA_DEPTH + 10 + AVOIDBUFFER, (Param::Field::PENALTY_AREA_WIDTH + 50 + AVOIDBUFFER*2)/2);

  // 2. 球在禁区内时 后卫
  CGeoEllipse RD_ELLIPSE1 = CGeoEllipse(CENTERPOINT, Param::Field::PENALTY_AREA_DEPTH + 10 + AVOIDBUFFER, (Param::Field::PENALTY_AREA_WIDTH + 50 + AVOIDBUFFER*2)/2);

  // 3. 弃用
	CGeoEllipse RD_ELLIPSE2 = CGeoEllipse(CENTERPOINT, Param::Field::PENALTY_AREA_DEPTH + 15 + AVOIDBUFFER, (Param::Field::PENALTY_AREA_WIDTH + 30 + AVOIDBUFFER*2)/2);

  // 4. 后腰所用椭圆
	CGeoEllipse RD_ELLIPSE3 = CGeoEllipse(CENTERPOINT, Param::Field::PENALTY_AREA_DEPTH*1.7 + AVOIDBUFFER, (Param::Field::PENALTY_AREA_WIDTH*1.5 + AVOIDBUFFER*2)/2);

  // 5. 球在禁区内时 后腰
	CGeoEllipse RD_ELLIPSE4 = CGeoEllipse(CENTERPOINT, Param::Field::PENALTY_AREA_DEPTH*2.5 + AVOIDBUFFER, (Param::Field::PENALTY_AREA_WIDTH*1.6 + AVOIDBUFFER*2)/2);

	//calcPenaltyLine规划线
  CGeoPoint DCENTERPOINT = CGeoPoint(-Param::Field::PITCH_LENGTH/2,0);
	CGeoEllipse D_ELLIPSE = CGeoEllipse(DCENTERPOINT,Param::Field::PENALTY_AREA_DEPTH+5 +AVOIDBUFFER,(Param::Field::PENALTY_AREA_WIDTH+8+AVOIDBUFFER*2)/2);
	//判断是否需要进入penaltycleaner
	CGeoEllipse D_ELLIPSE1 =  CGeoEllipse(DCENTERPOINT,Param::Field::PENALTY_AREA_DEPTH+38 +AVOIDBUFFER,(Param::Field::PENALTY_AREA_WIDTH+68+AVOIDBUFFER*2)/2);

	// 后卫相关临界值
	CGeoPoint LEFTBACK_CRITICAL_POINT = CGeoPoint(-(Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_DEPTH - Param::Vehicle::V2::PLAYER_SIZE), -(Param::Field::PITCH_WIDTH/2 - Param::Field::MAX_PLAYER_SIZE*2));
	CGeoPoint RIGHTBACK_CRITICAL_POINT = CGeoPoint(-(Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_DEPTH - Param::Vehicle::V2::PLAYER_SIZE), Param::Field::PITCH_WIDTH/2 - Param::Field::MAX_PLAYER_SIZE*2);
	const double SIDEBACK_DEFEND_CRITICAL_X = -Param::Field::PITCH_LENGTH / 6;
	const double SIDEBACK_DEFEND_BOUNDARY_DIR_MAX = (RIGHTBACK_CRITICAL_POINT - GOAL_CENTRE_POS).dir();
	const double SIDEBACK_DEFEND_BOUNDARY_DIR_MIN = SIDEBACK_DEFEND_BOUNDARY_DIR_MAX - Param::Math::PI / 5;
	const double SIDEBACK_DEFEND_BOUNDARY_DIR_BUFFER = Param::Math::PI * 5 / 180;
	const double SIDEBACK_SIDEFACTOR_BUFFER_DIR = Param::Math::PI * 25 / 180;
	const double SIDEBACK_BUFFER_DIST = 120;
	const double SIDEBACK_MARKING_DIST_THRESHOLD_UPPER = Param::Field::MAX_PLAYER_SIZE*3.5;
	const double SIDEBACK_MARKING_DIST_THRESHOLD_LOWER = Param::Field::MAX_PLAYER_SIZE*2;
	const double SIDEBACK_MARKING_DIST_THRESHOLD_BUFFER = Param::Vehicle::V2::PLAYER_SIZE;

	bool isPosInOurPenaltyArea(const CGeoPoint& pos){
		return Utils::InOurPenaltyArea(pos,Param::Vehicle::V2::PLAYER_SIZE/4);
	}
	int getPenaltyFronter(){
		int num = 0;
		for (int i=1;i<Param::Field::MAX_PLAYER+1;i++)
		{
			CGeoPoint pos = VisionModule::Instance()->OurPlayer(i).Pos();
			if (Utils::InOurPenaltyArea(pos,60))
			{
				num ++;
			}
		}
		return num;
	}
	CGeoPoint reversePoint(const CGeoPoint& p)
	{
		return CGeoPoint(-1*p.x(),-1*p.y());
	}

	double calcBlockAngle(const CGeoPoint& target,const CGeoPoint& player)
	{
		double dist_target2player = target.dist(player) <= PLAYERSIZE ? (PLAYERSIZE + 0.1) : target.dist(player);
		return fabs(asin(PLAYERSIZE / dist_target2player));
	}

	bool leftCirValid(const CGeoPoint& p)
	{
		return (p.x() < RBOTTOM_LEFT.x()) && (p.x() > RCENTER_LEFT.x())
			&& (p.y() < RCENTER_LEFT.y());
	}

	bool rightCirValid(const CGeoPoint& p)
	{
		return (p.x() < RBOTTOM_RIGHT.x()) && (p.x() > RCENTER_RIGHT.x())
			&& (p.y() > RCENTER_RIGHT.y());
	}

	int getEnemyShooter()
	{
		CVisionModule* pVision = vision;
		int shooter;
		if (pVision->Ball().Vel().mod() < ENEMY_PASS_SPEED)
		{
			shooter =  BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num;
		} else {
			const MobileVisionT ball = pVision->Ball();
			const double ball_speed_dir = ball.Vel().dir();
			double min_dist = Param::Field::PITCH_LENGTH, dist, angle_diff;
			int min_player = 0;
			const double PassAngleDiff = Param::Math::PI / 6;
			for (int i = 1; i <= Param::Field::MAX_PLAYER; i ++)
			{
				if (pVision->TheirPlayer(i).Valid() == true)
				{
					angle_diff = fabs(Utils::Normalize(Utils::Normalize(CVector(pVision->TheirPlayer(i).Pos()
						- ball.Pos()).dir()) - ball_speed_dir));
					dist = pVision->TheirPlayer(i).Pos().dist(ball.Pos());
					if (angle_diff < PassAngleDiff && dist < min_dist)
					{
						min_player = i;
						min_dist = dist;
					}
				}
			}
			if (min_player == 0)
			{
				shooter =  BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num;
			}
			else
			{
				shooter = min_player;
			}
		}
		return shooter;
	}

	CGeoLine getDefenceTargetAndLine(CGeoPoint& RdefenceTarget,double& RdefendDir){
		const CGeoPoint RGOAL_LEFT_CENTER(Param::Field::PITCH_LENGTH/2, -Param::Field::GOAL_WIDTH/2 + Param::Field::MAX_PLAYER_SIZE);
		const CGeoPoint RGOAL_RIGHT_CENTER(Param::Field::PITCH_LENGTH/2, Param::Field::GOAL_WIDTH/4 - Param::Field::MAX_PLAYER_SIZE);

		CVisionModule* pVision = vision;
		CGeoLine RdefenceLine = CGeoLine(CGeoPoint(0,0),0.0);
		//注意：带R(reverse)字头的都是反向后的变量！！！
		const int enemyNum = DefendUtils::getEnemyShooter();
		static int lastShooter;
		//球
		const MobileVisionT& ball = pVision->Ball();
		CGeoPoint RballPos = DefendUtils::reversePoint(ball.Pos());

    // 横传球的预测
    if (std::fabs(ball.Vel().dir()) > Param::Math::PI/3 && ball.Vel().dir() < Param::Math::PI/3*2)
      RballPos = reversePoint(ball.Pos() + Utils::Polar2Vector(ball.Vel().mod()/20, ball.Vel().dir()));
    //GDebugEngine::Instance()->gui_debug_arc(reversePoint(RballPos), 5, 0, 360, COLOR_WHITE);

		CVector RballVel = Utils::Polar2Vector(ball.Vel().mod(),Utils::Normalize(ball.Vel().dir() + Param::Math::PI));
		double RballVelMod = ball.Vel().mod();
		double RballVelDir = RballVel.dir();
		//球门连线
		CVector Rball2LeftGoal = RGOAL_LEFT_POS - RballPos;
		CVector Rball2RightGoal = RGOAL_RIGHT_POS - RballPos;
		CVector Rball2GoalCenter = RGOAL_CENTRE_POS - RballPos;
		double Rball2LeftDir = Rball2LeftGoal.dir();
		double Rball2RightDir = Rball2RightGoal.dir();
		double Rball2GoalCenterDir = Rball2GoalCenter.dir();
		double left2centreAngle = fabs(Utils::Normalize(Rball2LeftDir - Rball2GoalCenterDir));
		double right2centreAngle = fabs(Utils::Normalize(Rball2RightDir - Rball2GoalCenterDir));
		double Rball2enemyDir = Utils::Normalize((pVision->Ball().Pos() - pVision->TheirPlayer(enemyNum).Pos()).dir()+Param::Math::PI);

		static bool todefenddirection = false;
		if (true == DefenceInfo::Instance()->getOppPlayerByNum(enemyNum)->isTheRole("RReceiver")){
			todefenddirection = true;
		}
		if (enemyNum != lastShooter && todefenddirection ==true){
			todefenddirection =false;
		}
		///朝向线：判断1 球是否已经射出
		bool ballSpeed = (RballVelMod >= 30);
		bool outOfShooter = !(DefenceInfo::Instance()->getBallTaken());
		bool ballDirLimit = Utils::InBetween(RballVelDir,Rball2LeftDir - BALL_SHOOT_DIR_BUFFER,Rball2RightDir + BALL_SHOOT_DIR_BUFFER);
		bool ballShot = ballSpeed && outOfShooter && ballDirLimit;

		//或者球刚踢出，用位置进行判断
		bool enemyHasShot = pVision->TheirPlayer(enemyNum).Pos().dist(ball.Pos())<40
			&& Utils::InBetween(Rball2enemyDir,Rball2LeftDir - BALL_SHOOT_DIR_BUFFER,Rball2RightDir + BALL_SHOOT_DIR_BUFFER)
			&& ballSpeed && outOfShooter && ball.VelX() <0;
		ballShot = ballShot || enemyHasShot;
		if (pVision->TheirPlayer(enemyNum).Valid())
		{
			const PlayerVisionT& enemy = pVision->TheirPlayer(enemyNum);
			CGeoPoint RenemyPos = DefendUtils::reversePoint(enemy.Pos());
			double RenemyDir = Utils::Normalize(enemy.Dir() + Param::Math::PI);

			///朝向线：判断2 是否防朝向
			bool defenceEnemy = false;  //防守朝向的判据
			double defendRenemyDir = 0;  //防守的朝向

			//判断是否对方掌握球:暂时用法，以后这段话删去***********!!!!!!!!!!!!!!!!
			const int ourFastPlayer = BestPlayer::Instance()->ourFastestPlayerToBallList().at(0).num;
			const PlayerVisionT& me = pVision->OurPlayer(ourFastPlayer);
			bool enemyHasBall = enemy.Pos().dist(ball.Pos()) < me.Pos().dist(ball.Pos()) ? true : false;
			//TODO 以后开启下面这句话*****************************!!!!!!!!!!!!!!
			//bool enemyHasBall = BestPlayer::Instance()->oppWithBall();

			//对方射手和球连线
			CVector Renemy2ball = RballPos - RenemyPos;
			CVector Rball2enemy = RenemyPos - RballPos;
			double Renemy2ballDir = Renemy2ball.dir();
			bool Renemy2ballDirAdapt = Utils::InBetween(Renemy2ballDir,Rball2LeftDir - ENEMY_FACE_BUFFER,Rball2RightDir + ENEMY_FACE_BUFFER);
			bool Renemy2ballDistAdapt = RballPos.dist(RenemyPos) < ENEMY_BALL_DIST_BUFFER;
			defenceEnemy = enemyHasBall && Renemy2ballDirAdapt && Renemy2ballDistAdapt&& !todefenddirection;
			//判断对手是否传射配合
			bool enemyPass = RballVelMod > ENEMY_PASS_SPEED && fabs(Utils::Normalize(RballVelDir - Rball2enemy.dir())) < Param::Math::PI / 9.0;
			//cout<<"defenceEnemy is "<<defenceEnemy<<" "<<ball.Vel().mod()<<endl;
			if (defenceEnemy)//动态调整朝向
			{
				if (Utils::InBetween(RenemyDir,Rball2LeftDir - ENEMY_FACE_BUFFER,Rball2LeftDir))
				{
					double Renemy2leftEdge = fabs(Utils::Normalize(RenemyDir - Rball2LeftDir));
					defendRenemyDir = Rball2GoalCenterDir + left2centreAngle * (Renemy2leftEdge / ENEMY_FACE_BUFFER - 1);
					//cout<<"defence 11111111111"<<endl;
				} else if (Utils::InBetween(RenemyDir,Rball2LeftDir,Rball2RightDir))
				{
					defendRenemyDir = RenemyDir;
					//cout<<"defence 2222222222"<<endl;
				} else if (Utils::InBetween(RenemyDir,Rball2RightDir,Rball2RightDir + ENEMY_FACE_BUFFER))
				{
					double Renemy2rightEdge = fabs(Utils::Normalize(RenemyDir - Rball2RightDir));
					defendRenemyDir = Rball2GoalCenterDir - right2centreAngle * (Renemy2rightEdge / ENEMY_FACE_BUFFER - 1);
					//cout<<"defence 3333333333 "<<defendRenemyDir/Param::Math::PI*180<<endl;
				}else{
					defendRenemyDir = Rball2GoalCenterDir;
				}
			}
			if (enemyPass){
				if (Utils::AngleBetween(RenemyDir,(RGOAL_LEFT_POS - RenemyPos).dir(),(RGOAL_RIGHT_POS - RenemyPos).dir(),0)){
					defendRenemyDir = RenemyDir;
					//cout<<"pass 1111111"<<endl;
				}else if(Utils::InBetween(RenemyDir,(RGOAL_LEFT_POS - RenemyPos).dir()-ENEMY_FACE_BUFFER,(RGOAL_LEFT_POS - RenemyPos).dir())){
					defendRenemyDir =(RGOAL_LEFT_POS - RenemyPos).dir();
					//cout<<"pass 2222222"<<endl;
				}else if (Utils::InBetween(RenemyDir,(RGOAL_RIGHT_POS - RenemyPos).dir(),(RGOAL_RIGHT_POS - RenemyPos).dir()+ENEMY_FACE_BUFFER)){
					defendRenemyDir = (RGOAL_RIGHT_POS - RenemyPos).dir();
					//cout<<"pass 3333333"<<endl;
				}else{
					defendRenemyDir = (RGOAL_CENTRE_POS - RenemyPos).dir();
				}
			}
			defenceEnemy = defenceEnemy||enemyPass;
			//由判断条件执行跳转
			//cout<<ballShooted<<"  "<<defenceEnemy<<endl;
			if (ballShot)//此处不 && ball.Valid()，只要看到一帧就要坚定地走过去
			{
				RdefenceLine = CGeoLine(RballPos,RballVelDir); //防球速线
				RdefenceTarget= reversePoint(pVision->Ball().RawPos());
				RdefendDir = RballVelDir;
				//cout<<vision->Cycle()<<" ball shooted"<<endl;
			} else if (defenceEnemy)
			{
				if (ball.Valid() && RballVelMod < ENEMY_PASS_SPEED + 20)
				{
					RdefenceTarget = RballPos;
					RdefendDir = defendRenemyDir;
					RdefenceLine = CGeoLine(RballPos,defendRenemyDir);      //看到球以球为主防朝向
					//cout<<"11111: "<<defendRenemyDir<<endl;
				} else {
					RdefenceLine = CGeoLine(RenemyPos,defendRenemyDir);	//看不到球或者球速过大以持球人为主
					RdefenceTarget = RenemyPos;
					RdefendDir = defendRenemyDir;
					//cout<<"22222: "<<defendRenemyDir<<endl;
				}
				if (enemyPass){
					CGeoPoint RBasePoint = RGOAL_CENTRE_POS;
					double vely;
					if (pVision->Ball().VelY()>400){
						vely = 400;
					}else if (pVision->Ball().VelY()<-400){
						vely = -400;
					}else{
						vely = pVision->Ball().VelY();
					}
					RBasePoint = RBasePoint + Utils::Polar2Vector((-0.04)*vely,Param::Math::PI/2);
					RdefendDir = ( RBasePoint - RballPos).dir();
					RdefenceTarget = RballPos;
					RdefenceLine = CGeoLine(RballPos,RBasePoint);
					//cout<<"enemypass"<<endl;
				}
			}else{
				RdefenceLine = CGeoLine(RballPos,RGOAL_CENTRE_POS);//防球——门中心连线
				RdefenceTarget = RballPos;
				RdefendDir = CVector(RGOAL_CENTRE_POS - RballPos).dir();

				// 球往一侧走时防守点偏向那一侧
				const CGeoLine RBallVelLine(RballPos, RballVelDir);
				const CGeoPoint RLeftCorner = CGeoPoint(Param::Field::PITCH_LENGTH/2, -Param::Field::PITCH_WIDTH/2);
				const CGeoPoint RRightCorner = CGeoPoint(Param::Field::PITCH_LENGTH/2, Param::Field::PITCH_WIDTH/2);
				const CGeoLine RBaseLine(RLeftCorner, RRightCorner);
				const CGeoLineLineIntersection intersection(RBallVelLine, RBaseLine);
				if (intersection.Intersectant() == true) {
					const CGeoPoint& point = intersection.IntersectPoint();
					if (point.y() < Param::Field::PITCH_WIDTH/2 && point.y() >= 0 && RballVel.x() > 0 && RballVel.y() > 0
							|| point.y() > -Param::Field::PITCH_WIDTH/2 && point.y() <= 0 && RballVel.x() > 0 && RballVel.y() < 0) {
						if (RballPos.y() > -Param::Vehicle::V2::PLAYER_SIZE && RballVel.y() > 0) {
							RdefenceLine = CGeoLine(RballPos, RGOAL_RIGHT_CENTER);
							RdefendDir = (RGOAL_RIGHT_CENTER - RballPos).dir();
						} else if (RballPos.y() < Param::Vehicle::V2::PLAYER_SIZE && RballVel.y() < 0) {
							RdefenceLine = CGeoLine(RballPos, RGOAL_LEFT_CENTER);
							RdefendDir = (RGOAL_LEFT_CENTER - RballPos).dir();
						}
					}
				}
			}
		} else {
			if (ballShot)
			{
				RdefenceLine = CGeoLine(RballPos,RballVelDir);
				RdefenceTarget = reversePoint(pVision->Ball().RawPos());
				RdefendDir = RballVelDir;
				//cout<<"ball is shooting"<<endl;
			} else
			{
				RdefenceLine = CGeoLine(RballPos,RGOAL_CENTRE_POS);
				RdefenceTarget = RballPos;
				RdefendDir = (RGOAL_CENTRE_POS - RballPos).dir();

				// 球往一侧走时防守点偏向那一侧
				const CGeoLine RBallVelLine(RballPos, RballVelDir);
				const CGeoPoint RLeftCorner = CGeoPoint(Param::Field::PITCH_LENGTH/2, -Param::Field::PITCH_WIDTH/2);
				const CGeoPoint RRightCorner = CGeoPoint(Param::Field::PITCH_LENGTH/2, Param::Field::PITCH_WIDTH/2);
				const CGeoLine RBaseLine(RLeftCorner, RRightCorner);
				const CGeoLineLineIntersection intersection(RBallVelLine, RBaseLine);
				if (intersection.Intersectant() == true) {
					const CGeoPoint& point = intersection.IntersectPoint();
					if (point.y() < Param::Field::PITCH_WIDTH/2 && point.y() >= 0 && RballVel.x() > 0 && RballVel.y() > 0
							|| point.y() > -Param::Field::PITCH_WIDTH/2 && point.y() <= 0 && RballVel.x() > 0 && RballVel.y() < 0) {
						if (RballPos.y() > -Param::Vehicle::V2::PLAYER_SIZE && RballVel.y() > 0) {
							RdefenceLine = CGeoLine(RballPos, RGOAL_RIGHT_CENTER);
							RdefendDir = (RGOAL_RIGHT_CENTER - RballPos).dir();
						} else if (RballPos.y() < Param::Vehicle::V2::PLAYER_SIZE && RballVel.y() < 0) {
							RdefenceLine = CGeoLine(RballPos, RGOAL_LEFT_CENTER);
							RdefendDir = (RGOAL_LEFT_CENTER - RballPos).dir();
						}
					}
				}
			}
		}
		//补球速慢时突然被打一脚的bug
		double RenemyDir = Utils::Normalize(pVision->TheirPlayer(enemyNum).Dir() + Param::Math::PI);
		double diffDir = fabs(Utils::Normalize(ball.Vel().dir() - (ball.Pos() - pVision->TheirPlayer(enemyNum).Pos()).dir()));
    double enemyToBallDist = ball.Pos().dist(pVision->TheirPlayer(enemyNum).Pos());
		if (Utils::InOurPenaltyArea(ball.Pos(), 0) == false
				&& enemyToBallDist < 30
				&& ball.Vel().mod() < 200
				&& Utils::InBetween(RballVelDir, Rball2LeftDir, Rball2RightDir) == false
				&& (diffDir < Param::Math::PI/6 || Utils::InBetween(RenemyDir, Rball2LeftDir, Rball2RightDir))) {
			RdefenceLine = CGeoLine(RballPos,RGOAL_CENTRE_POS);
			RdefenceTarget = RballPos;
			RdefendDir = CVector(RGOAL_CENTRE_POS - RballPos).dir();
			//cout<<"in here"<<endl;
		}
		CGeoLineLineIntersection pointA = CGeoLineLineIntersection(RdefenceLine, CGeoLine(CGeoPoint(Param::Field::PITCH_LENGTH / 2.0, -Param::Field::PITCH_WIDTH / 2.0), CGeoPoint(Param::Field::PITCH_LENGTH / 2.0, 0)));
		CGeoPoint pointAA = pointA.IntersectPoint();
		GDebugEngine::Instance()->gui_debug_x(reversePoint(RdefenceTarget), COLOR_PURPLE);
		lastShooter = enemyNum;
		return RdefenceLine;
	}

	bool getBallShooted(){
		const CVisionModule* pVision = vision;
		const MobileVisionT& ball = pVision->Ball();
		const int enemyNum = DefendUtils::getEnemyShooter();
		CGeoPoint RballPos = DefendUtils::reversePoint(ball.Pos());
		CVector Rball2LeftGoal = RGOAL_LEFT_POS - RballPos;
		CVector Rball2RightGoal = RGOAL_RIGHT_POS - RballPos;
		CVector Rball2GoalCenter = RGOAL_CENTRE_POS - RballPos;
		double Rball2LeftDir = Rball2LeftGoal.dir();
		double Rball2RightDir = Rball2RightGoal.dir();
		double Rball2GoalCenterDir = Rball2GoalCenter.dir();
		CVector RballVel = Utils::Polar2Vector(ball.Vel().mod(),Utils::Normalize(ball.Vel().dir() + Param::Math::PI));
		double RballVelMod = ball.Vel().mod();
		double RballVelDir = RballVel.dir();
		double Rball2enemyDir = Utils::Normalize((pVision->Ball().Pos() - pVision->TheirPlayer(enemyNum).Pos()).dir()+Param::Math::PI);
		///朝向线：判断球是否已经射出
		bool ballSpeed = RballVelMod >= 30;
		bool outOfShooter = !(DefenceInfo::Instance()->getBallTaken());
		bool ballDirLimit = Utils::InBetween(RballVelDir,Rball2LeftDir - BALL_SHOOT_DIR_BUFFER,Rball2RightDir + BALL_SHOOT_DIR_BUFFER);
		bool ballShooted = ballSpeed && outOfShooter && ballDirLimit;
		//或者球刚踢出，用位置进行判断
		bool enemyShooted = pVision->TheirPlayer(enemyNum).Pos().dist(ball.Pos())<40 &&
			Utils::InBetween(Rball2enemyDir,Rball2LeftDir - BALL_SHOOT_DIR_BUFFER,Rball2RightDir + BALL_SHOOT_DIR_BUFFER)
			&& ballSpeed && outOfShooter && ball.VelX() <0;
		ballShooted = ballShooted||enemyShooted;
		return ballShooted;
	}

	bool getEnemyPass(){
		const CVisionModule* pVision = vision;
		const MobileVisionT& ball = pVision->Ball();
		const int enemyNum = DefendUtils::getEnemyShooter();
		CGeoPoint RballPos = DefendUtils::reversePoint(ball.Pos());
		CVector RballVel = Utils::Polar2Vector(ball.Vel().mod(),Utils::Normalize(ball.Vel().dir() + Param::Math::PI));
		double RballVelMod = ball.Vel().mod();
		double RballVelDir = RballVel.dir();
		bool enemyPass = false;
		if (pVision->TheirPlayer(enemyNum).Valid()){
			const PlayerVisionT& enemy = pVision->TheirPlayer(enemyNum);
			CGeoPoint RenemyPos = DefendUtils::reversePoint(enemy.Pos());
			CVector Rball2enemy = RenemyPos - RballPos;
			double Rball2enemyDir = Utils::Normalize((pVision->Ball().Pos() - pVision->TheirPlayer(enemyNum).Pos()).dir()+Param::Math::PI);
			enemyPass = RballVelMod > ENEMY_PASS_SPEED &&
			fabs(Utils::Normalize(RballVelDir - Rball2enemy.dir())) < Param::Math::PI / 9.0;
		}
		return enemyPass;
	}

	CGeoPoint calcDefenderPoint(const CGeoPoint Rtarget,const double Rdir,const posSide Rside){
		CVisionModule* pVision = vision;
		//计算出一个反向的后卫站位点
		CGeoPoint RdefenderPoint;
		int sideFactor;
		if (POS_SIDE_LEFT == Rside)
			sideFactor = -1;
		else if (POS_SIDE_RIGHT == Rside)
			sideFactor = 1;
		else if (POS_SIDE_MIDDLE == Rside)
			sideFactor = 0;
		CVector transVector = Utils::Polar2Vector(PLAYERSIZE,Utils::Normalize(Rdir + sideFactor * Param::Math::PI / 2));
		CGeoPoint RtransPoint = Rtarget + transVector;
		CGeoLine RtargetLine = CGeoLine(RtransPoint,Rdir);  // 用于计算交点的直线，为原传入直线按照要求平移PLAYERSIZE后的直线

		bool Up;
		if (RtransPoint.x() >= RCENTER_LEFT.x())
			Up = true;
		else
			Up = false;

		posSide tpSide;
		if (RtransPoint.y() > 0)
			tpSide = POS_SIDE_RIGHT;
		else
			tpSide = POS_SIDE_LEFT;

		CGeoPoint temp[3];  // 两个临时点,因为交点最多有两个点
		if (Up && POS_SIDE_RIGHT == tpSide)
		{
			int pointCount = 0;
			CGeoLineCircleIntersection intersect = CGeoLineCircleIntersection(RtargetLine,RD_CIR_RIGHT);
			if (intersect.intersectant())
			{
				if (DefendUtils::rightCirValid(intersect.point1()))
				{
					temp[pointCount] = intersect.point1();
					pointCount++;
				}
				if (DefendUtils::rightCirValid(intersect.point2()))
				{
					temp[pointCount] = intersect.point2();
					pointCount++;
				}
				if (0 == pointCount)
				{
					if (Rdir < -Param::Math::PI/2)
					{
						RdefenderPoint = CGeoPoint(RBOTTOM_RIGHT.x() - PLAYERSIZE,RBOTTOM_RIGHT.y());
					} else RdefenderPoint = RCENTER_RIGHT;
				} else if (1 == pointCount)
				{
					RdefenderPoint = temp[0];
				} else { //2 == pointCount
					if (Rdir > 0)
					{
						if (temp[0].y() > temp[1].y())
						{
							RdefenderPoint = temp[1];
						} else RdefenderPoint = temp[0];
					} else {
						if (temp[0].y() > temp[1].y())
						{
							RdefenderPoint = temp[0];
						} else RdefenderPoint = temp[1];
					}
				}
			} else {//没交点
				if (Rdir < -Param::Math::PI/2)
				{
					RdefenderPoint = CGeoPoint(RBOTTOM_RIGHT.x() - PLAYERSIZE,RBOTTOM_RIGHT.y());
				} else RdefenderPoint = RCENTER_RIGHT;
			}
		}
		else if (Up && POS_SIDE_LEFT == tpSide)
		{
			int pointCount = 0;
			CGeoLineCircleIntersection intersect = CGeoLineCircleIntersection(RtargetLine,RD_CIR_LEFT);
			if (intersect.intersectant())
			{
				if (DefendUtils::leftCirValid(intersect.point1()))
				{
					temp[pointCount] = intersect.point1();
					pointCount++;
				}
				if (DefendUtils::leftCirValid(intersect.point2()))
				{
					temp[pointCount] = intersect.point2();
					pointCount++;
				}
				if (0 == pointCount)
				{
					if (Rdir > Param::Math::PI/2)
					{
						RdefenderPoint = CGeoPoint(RBOTTOM_LEFT.x() - PLAYERSIZE,RBOTTOM_LEFT.y());
					} else RdefenderPoint = RCENTER_LEFT;
				} else if (1 == pointCount)
				{
					RdefenderPoint = temp[0];
				} else { //2 == pointCount
					if (Rdir > 0)
					{
						if (temp[0].y() > temp[1].y())
						{
							RdefenderPoint = temp[1];
						} else RdefenderPoint = temp[0];
					} else {
						if (temp[0].y() > temp[1].y())
						{
							RdefenderPoint = temp[0];
						} else RdefenderPoint = temp[1];
					}
				}
			} else {//没交点
				if (Rdir > Param::Math::PI/2)
				{
					RdefenderPoint = CGeoPoint(RBOTTOM_LEFT.x() - PLAYERSIZE,RBOTTOM_LEFT.y());
				} else RdefenderPoint = RCENTER_LEFT;
			}
		}
		else if (!Up && POS_SIDE_RIGHT == tpSide)
		{
			int pointCount = 0;
			CGeoLineCircleIntersection intersect = CGeoLineCircleIntersection(RtargetLine,RD_CIR_RIGHT);
			if (intersect.intersectant())
			{
				if (DefendUtils::rightCirValid(intersect.point1()))
				{
					temp[pointCount] = intersect.point1();
					pointCount++;
				}
				if (DefendUtils::rightCirValid(intersect.point2()))
				{
					temp[pointCount] = intersect.point2();
					pointCount++;
				}
			}
			CGeoLineLineIntersection intersect1 = CGeoLineLineIntersection(RtargetLine,RD_CENTER_SEGMENT);
			if (intersect1.Intersectant())
			{
				CGeoPoint interP = intersect1.IntersectPoint();
				if (interP.y() <= RCENTER_RIGHT.y() && interP.y() >= RCENTER_LEFT.y())
				{
					temp[pointCount] = interP;
					pointCount++;
				}
			}
			double compareDir = CVector(RCENTER_RIGHT - RtransPoint).dir();
			if (0 == pointCount)
			{
				if (Rdir < compareDir)
				{
					RdefenderPoint = RCENTER_LEFT;
				} else RdefenderPoint = CGeoPoint(RBOTTOM_RIGHT.x() - PLAYERSIZE,RBOTTOM_RIGHT.y());
			} else if (1 == pointCount)
			{
				RdefenderPoint = temp[0];
			} else {// pointCount >= 2
				if (temp[0].y() <= temp[1].y())
				{
					RdefenderPoint = temp[0];
				} else RdefenderPoint = temp[1];
			}
		}
		else ///  !Up && POS_SIDE_LEFT == tpSide
		{
			int pointCount = 0;
			CGeoLineCircleIntersection intersect = CGeoLineCircleIntersection(RtargetLine,RD_CIR_LEFT);
			if (intersect.intersectant())
			{
				if (DefendUtils::leftCirValid(intersect.point1()))
				{
					temp[pointCount] = intersect.point1();
					pointCount++;
				}
				if (DefendUtils::leftCirValid(intersect.point2()))
				{
					temp[pointCount] = intersect.point2();
					pointCount++;
				}
			}
			CGeoLineLineIntersection intersect1 = CGeoLineLineIntersection(RtargetLine,RD_CENTER_SEGMENT);
			if (intersect1.Intersectant())
			{
				CGeoPoint interP = intersect1.IntersectPoint();
				if (interP.y() <= RCENTER_RIGHT.y() && interP.y() >= RCENTER_LEFT.y())
				{
					temp[pointCount] = interP;
					pointCount++;
				}
			}
			double compareDir = CVector(RCENTER_LEFT - RtransPoint).dir();
			if (0 == pointCount)
			{
				if (Rdir > compareDir)
				{
					RdefenderPoint = RCENTER_RIGHT;
				} else RdefenderPoint = CGeoPoint(RBOTTOM_LEFT.x() - PLAYERSIZE,RBOTTOM_LEFT.y());
			} else if (1 == pointCount)
			{
				RdefenderPoint = temp[0];
			} else {// pointCount >= 2
				if (temp[0].y() >= temp[1].y())
				{
					RdefenderPoint = temp[0];
				} else RdefenderPoint = temp[1];
			}
		}

		if (false) {
			GDebugEngine::Instance()->gui_debug_arc(reversePoint(RCENTER2BOTTOM_LEFT), PEN_RADIUS, 0, 360, COLOR_RED);
			GDebugEngine::Instance()->gui_debug_arc(reversePoint(RCENTER2BOTTOM_RIGHT), PEN_RADIUS, 0, 360, COLOR_CYAN);
			GDebugEngine::Instance()->gui_debug_line(reversePoint(RCENTER_LEFT), reversePoint(RCENTER_RIGHT), COLOR_WHITE);
		}

		return RdefenderPoint;
	}

	//计算DefPos2013函数
	CGeoPoint calcMiddlePoint(const CGeoPoint Rtarget,const double Rdir,double Rradius){
		CVisionModule* pVision = vision;
		//计算出一个反向的后卫站位点
		CGeoPoint RMiddlePoint;
		CGeoLine targetLine = CGeoLine(Rtarget,Rdir);//用于计算交点的直线，为原传入直线按照要求平移PLAYERSIZE后的直线
		CGeoCirlce Rdefendcircle = CGeoCirlce(CENTERPOINT,Rradius);
		//GDebugEngine::Instance()->gui_debug_line(Rtarget,RGOAL_CENTRE_POS);
		CGeoLineCircleIntersection intersect = CGeoLineCircleIntersection(targetLine,Rdefendcircle);
		if (intersect.intersectant()){
			if(intersect.point1().x()<intersect.point2().x()){
				RMiddlePoint = intersect.point1();
			 }else
				RMiddlePoint = intersect.point2();
		}

		//GDebugEngine::Instance()->gui_debug_x(RMiddlePoint);
		//cout<<"Rtarget is:      "<<Rtarget<<endl;
		//cout<<"RMiddle point is:       "<<RMiddlePoint<<endl;
		return RMiddlePoint;
	}

	CGeoPoint calcDefenderPointV2(const CGeoPoint Rtarget,const double Rdir,const posSide Rside,int mode,double ratio){
		    CVisionModule* pVision = vision;
		    CGeoPoint RdefenderPoint;
		    CGeoPoint Rbasepoint;
			double Rbasedir;
		//计算出一个反向的后卫站位点
		//当ratio为-1是后卫调用点，否则为盯人和防头球调用点
			if (ratio < 0){
				int sideFactor;
				if (POS_SIDE_LEFT == Rside){
					sideFactor = 1;
				} else if (POS_SIDE_RIGHT == Rside){
					sideFactor = -1;
				} else if (POS_SIDE_MIDDLE == Rside){
					sideFactor = 0;
				}
				 CGeoPoint defendbasepoint;
				 Rbasepoint = Rtarget;
				 Rbasedir = Rdir;
				if (Utils::InOurPenaltyArea(pVision->Ball().Pos(),PENALTY_BUFFER) && mode==0){
					int oppnum =  BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num;
					if (/*pVision->TheirPlayer(oppnum).Pos().x()<0 && */oppnum!=0 && vision->TheirPlayer(oppnum).Valid()){
						 if(pVision->TheirPlayer(oppnum).Pos().y()>0 && (pVision->TheirPlayer(oppnum).Pos()-GOAL_CENTRE_POS).dir()>(RIGHTBACK_CRITICAL_POINT-GOAL_CENTRE_POS).dir()){
							 defendbasepoint = RIGHTBACK_CRITICAL_POINT;
							 Rbasepoint = reversePoint(defendbasepoint);
							 Rbasedir = (RGOAL_CENTRE_POS - Rbasepoint).dir();
						 }else if (pVision->TheirPlayer(oppnum).Pos().y()<0 && (pVision->TheirPlayer(oppnum).Pos()-GOAL_CENTRE_POS).dir()<(LEFTBACK_CRITICAL_POINT-GOAL_CENTRE_POS).dir()){
							 defendbasepoint = LEFTBACK_CRITICAL_POINT;
							 Rbasepoint = reversePoint(defendbasepoint);
							 Rbasedir = (RGOAL_CENTRE_POS - Rbasepoint).dir();
						 }else{
							 defendbasepoint = pVision->TheirPlayer(oppnum).Pos();
							 Rbasepoint = reversePoint(defendbasepoint);
							 Rbasedir = (RGOAL_CENTRE_POS - Rbasepoint).dir();
						 }
						 if (!Utils::InOurPenaltyArea(pVision->TheirPlayer(oppnum).Pos(),PENALTY_BUFFER)){
							 if (POS_SIDE_LEFT == Rside){
								Rbasedir = Rbasedir ;
							 }else if(POS_SIDE_RIGHT == Rside){
								 Rbasedir = Rbasedir ;
							 }
						 }else{
							 if (POS_SIDE_LEFT == Rside && defendbasepoint == pVision->TheirPlayer(oppnum).Pos()){
								 Rbasedir = Rbasedir -0.1;
							 }else if(POS_SIDE_RIGHT == Rside && defendbasepoint == pVision->TheirPlayer(oppnum).Pos()){
								 Rbasedir = Rbasedir +0.1;
							 }
						 }
					}
					CVector transVector = Utils::Polar2Vector(PLAYERSIZE,Utils::Normalize(Rbasedir + sideFactor * Param::Math::PI / 2));
					CGeoPoint transPoint = Rbasepoint + transVector;
					CGeoLine targetLine = CGeoLine(transPoint,Rbasedir);
					//GDebugEngine::Instance()->gui_debug_line(reversePoint(transPoint),reversePoint(transPoint+Utils::Polar2Vector(100,Rbasedir)),COLOR_PURPLE);

	                 //球在禁区内，后卫前移
					CGeoLineEllipseIntersection intersect = CGeoLineEllipseIntersection(targetLine,RD_ELLIPSE1);
					if(intersect.intersectant()){
						if(intersect.point1().dist(transPoint)<intersect.point2().dist(transPoint)){
							RdefenderPoint = intersect.point1();
						}else{
							RdefenderPoint = intersect.point2();
						}
					}
				}else{
					CVector transVector = Utils::Polar2Vector(PLAYERSIZE,Utils::Normalize(Rbasedir + sideFactor * Param::Math::PI / 2));
					CGeoPoint transPoint = Rbasepoint + transVector;
					CGeoLine targetLine = CGeoLine(transPoint,Rbasedir);
					CGeoLineEllipseIntersection intersect = CGeoLineEllipseIntersection(targetLine,RD_ELLIPSE);
					//stop模式下车身后移
					if (vision->GetCurrentRefereeMsg()=="gameStop" || vision->GetCurrentRefereeMsg() == " theirIndirectKick" || vision->GetCurrentRefereeMsg() =="theirDirectKick"){
						intersect = CGeoLineEllipseIntersection(targetLine,RD_ELLIPSE2);
					}
					if(intersect.intersectant()){
						if(intersect.point1().dist(transPoint)<intersect.point2().dist(transPoint)){
							RdefenderPoint = intersect.point1();
						}else{
							RdefenderPoint = intersect.point2();
						}
					}else{
						CGeoLine wrongDefenceLine = CGeoLine(reversePoint(transPoint),reversePoint(transPoint+Utils::Polar2Vector(500,Rbasedir)));
						//对球速线抖动造成invalid的处理
						CGeoLineLineIntersection pointA = CGeoLineLineIntersection(wrongDefenceLine,CGeoLine(CGeoPoint(Param::Field::PITCH_LENGTH / 2.0,-Param::Field::PITCH_WIDTH / 2.0),CGeoPoint(Param::Field::PITCH_LENGTH / 2.0,Param::Field::PITCH_WIDTH / 2.0)));
						if(pointA.Intersectant()){
							CGeoPoint pointAA = pointA.IntersectPoint();
							if (pointAA.y()>0){
								targetLine = CGeoLine(RGOAL_RIGHT_POS,transPoint);
							}else{
								targetLine = CGeoLine(RGOAL_LEFT_POS,transPoint);
							}
							intersect = CGeoLineEllipseIntersection(targetLine,RD_ELLIPSE);
							if(intersect.intersectant()){
								if(intersect.point1().dist(transPoint)<intersect.point2().dist(transPoint)){
									RdefenderPoint = intersect.point1();
								}else{
									RdefenderPoint = intersect.point2();
								}
							}else{
								cout<<"defend valid!!"<<endl;
							}
						}
					}
				}
			if (mode == 1){
				CVector transVector = Utils::Polar2Vector(PLAYERSIZE,Utils::Normalize(Rbasedir + sideFactor * Param::Math::PI / 2));
				CGeoPoint transPoint = Rbasepoint + transVector;
				if (RdefenderPoint.x()>280*Param::Field::RATIO){
					//have modified by thj
					if (transPoint.y()<0){
						RdefenderPoint = CGeoPoint(300*Param::Field::RATIO,-144*Param::Field::RATIO);
					}else{
						RdefenderPoint = CGeoPoint(300*Param::Field::RATIO,144*Param::Field::RATIO);
					}
				}
			}
			if (mode == 3){
				if (Utils::InOurPenaltyArea(pVision->Ball().Pos(),PENALTY_BUFFER)){
					int oppnum =  BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num;
					if (oppnum!=0 && vision->TheirPlayer(oppnum).Valid()){
						if(pVision->TheirPlayer(oppnum).Pos().y()>0 && (pVision->TheirPlayer(oppnum).Pos()-GOAL_CENTRE_POS).dir()>(RIGHTBACK_CRITICAL_POINT-GOAL_CENTRE_POS).dir()){
							defendbasepoint = RIGHTBACK_CRITICAL_POINT;
							Rbasepoint = reversePoint(defendbasepoint);
							Rbasedir = (RGOAL_CENTRE_POS - Rbasepoint).dir();
						}else if (pVision->TheirPlayer(oppnum).Pos().y()<0 && (pVision->TheirPlayer(oppnum).Pos()-GOAL_CENTRE_POS).dir()<(LEFTBACK_CRITICAL_POINT-GOAL_CENTRE_POS).dir()){
							defendbasepoint = LEFTBACK_CRITICAL_POINT;
							Rbasepoint = reversePoint(defendbasepoint);
							Rbasedir = (RGOAL_CENTRE_POS - Rbasepoint).dir();
						}else{
							defendbasepoint = pVision->TheirPlayer(oppnum).Pos();
							Rbasepoint = reversePoint(defendbasepoint);
							Rbasedir = (RGOAL_CENTRE_POS - Rbasepoint).dir();
						}
					}
				}
				CGeoLine targetLine = CGeoLine(Rbasepoint,Rbasedir);
				double Rball2goalDir = (RGOAL_CENTRE_POS - Rbasepoint).dir();
				CGeoLine targetLine1 = CGeoLine(Rbasepoint,Rball2goalDir);
				CGeoLineEllipseIntersection intersect = CGeoLineEllipseIntersection(targetLine,RD_ELLIPSE3);
				CGeoLineEllipseIntersection intersect1 = CGeoLineEllipseIntersection(targetLine1,RD_ELLIPSE3);
				if (Utils::InOurPenaltyArea(vision->Ball().Pos(),PENALTY_BUFFER)){
					intersect = CGeoLineEllipseIntersection(targetLine,RD_ELLIPSE4);
					intersect1 = CGeoLineEllipseIntersection(targetLine1,RD_ELLIPSE4);
				}
				//GDebugEngine::Instance()->gui_debug_line(reversePoint(Rbasepoint),reversePoint(Rbasepoint+Utils::Polar2Vector(100,Rbasedir)),COLOR_PURPLE);
				//GDebugEngine::Instance()->gui_debug_line(reversePoint(Rbasepoint),reversePoint(Rbasepoint+Utils::Polar2Vector(100,Rball2goalDir)),COLOR_PURPLE);
				if(intersect.intersectant()){
					if(intersect.point1().dist(Rbasepoint)<intersect.point2().dist(Rbasepoint)){
						RdefenderPoint = intersect.point1();
					}else{
						RdefenderPoint = intersect.point2();
					}
				}else{
					//不相交使用球门之间的相交线
					//cout<<"middle valid!!"<<endl;
					if(intersect1.intersectant()){
						if(intersect1.point1().dist(Rbasepoint)<intersect1.point2().dist(Rbasepoint)){
							RdefenderPoint = intersect1.point1();
						}else{
							RdefenderPoint = intersect1.point2();
						}
					}
				}
			}
		}else{
			double x = Param::Field::PENALTY_AREA_DEPTH +AVOIDBUFFER;
			double y = (Param::Field::PENALTY_AREA_WIDTH +AVOIDBUFFER*2)/2;
			x = x + ratio*10;
			y = y + ratio*10;
			CGeoEllipse RDEFENDELLIPSE = CGeoEllipse(CENTERPOINT,x,y);
			int sideFactor;
			if (POS_SIDE_LEFT == Rside){
				sideFactor = 1;
			} else if (POS_SIDE_RIGHT == Rside){
				sideFactor = -1;
			} else if (POS_SIDE_MIDDLE == Rside){
				sideFactor = 0;
			}
			CVector transVector = Utils::Polar2Vector(PLAYERSIZE,Utils::Normalize(Rdir + sideFactor * Param::Math::PI / 2));
			CGeoPoint transPoint = Rtarget + transVector;
			CGeoLine targetLine = CGeoLine(transPoint,Rdir);
			CGeoLineEllipseIntersection intersect = CGeoLineEllipseIntersection(targetLine,RDEFENDELLIPSE);
			if(intersect.intersectant()){
				if(intersect.point1().dist(transPoint)<intersect.point2().dist(transPoint)){
					RdefenderPoint = intersect.point1();
				}else{
					RdefenderPoint = intersect.point2();
				}
			}else{
				cout<<"marking errorly!!!"<<endl;
			}
		}
		GDebugEngine::Instance()->gui_debug_x(reversePoint(RdefenderPoint),COLOR_PURPLE);
		return RdefenderPoint;
	}

	CGeoPoint calcDefenderPointV3(const CGeoPoint& RTarget, double RDir, posSide RSide, int mode) {
		CVisionModule* pVision = vision;
		CGeoPoint RDefenderPoint;
		CGeoPoint defendbasepoint;
		CGeoPoint RBasepoint = RTarget;
		double Rbasedir = RDir;
		int sideFactor;
		switch (RSide) {
			case POS_SIDE_LEFT:
				sideFactor = 1;
				break;
			case POS_SIDE_RIGHT:
				sideFactor = -1;
				break;
			case POS_SIDE_MIDDLE:
				sideFactor = 0;
				break;
		}
		if (Utils::InOurPenaltyArea(pVision->Ball().Pos(), PENALTY_BUFFER) && mode == 0) {
			int oppnum = BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num;
			if (oppnum != 0 && vision->TheirPlayer(oppnum).Valid()) {
				 if ((pVision->TheirPlayer(oppnum).Pos()-GOAL_CENTRE_POS).dir()>(RIGHTBACK_CRITICAL_POINT-GOAL_CENTRE_POS).dir())
					 defendbasepoint = RIGHTBACK_CRITICAL_POINT;
				 else if ((pVision->TheirPlayer(oppnum).Pos()-GOAL_CENTRE_POS).dir()<(LEFTBACK_CRITICAL_POINT-GOAL_CENTRE_POS).dir())
					 defendbasepoint = LEFTBACK_CRITICAL_POINT;
				 else
					 defendbasepoint = pVision->TheirPlayer(oppnum).Pos();
				 RBasepoint = reversePoint(defendbasepoint);
				 Rbasedir = (RGOAL_CENTRE_POS - RBasepoint).dir();
			}
			CVector transVector = Utils::Polar2Vector(PLAYERSIZE,Utils::Normalize(Rbasedir + sideFactor * Param::Math::PI / 2));
			CGeoPoint transPoint = RBasepoint + transVector;
			CGeoLine targetLine = CGeoLine(transPoint,Rbasedir);

			//球在禁区内，后卫前移
			CGeoLineEllipseIntersection intersect = CGeoLineEllipseIntersection(targetLine, RD_ELLIPSE1);
			if (intersect.intersectant()) {
				if (intersect.point1().dist(transPoint) < intersect.point2().dist(transPoint)) {
					RDefenderPoint = intersect.point1();
				} else {
					RDefenderPoint = intersect.point2();
				}
        //GDebugEngine::Instance()->gui_debug_arc(reversePoint(pVision->Ball().Pos()), 5, 0, 360, COLOR_WHITE);
        //GDebugEngine::Instance()->gui_debug_x(reversePoint(pVision->Ball().Pos()), COLOR_WHITE);
        //cout << pVision->Cycle() << " " << pVision->Ball().X() << " " << pVision->Ball().Y() << endl;
			}
		} else if (mode == 0) {
			CVector transVector = Utils::Polar2Vector(PLAYERSIZE, Utils::Normalize(Rbasedir + sideFactor * Param::Math::PI / 2));
			CGeoPoint transPoint = RBasepoint + transVector;
			CGeoLine targetLine = CGeoLine(transPoint, Rbasedir);
			CGeoLineEllipseIntersection intersect = CGeoLineEllipseIntersection(targetLine, RD_ELLIPSE);

			if (intersect.intersectant()) {
				if (intersect.point1().dist(transPoint) < intersect.point2().dist(transPoint)) {
					RDefenderPoint = intersect.point1();
				} else {
					RDefenderPoint = intersect.point2();
				}
			} else {
				CGeoLine wrongDefenceLine = CGeoLine(reversePoint(transPoint),reversePoint(transPoint+Utils::Polar2Vector(500,Rbasedir)));
				//对球速线抖动造成invalid的处理
				CGeoLineLineIntersection pointA = CGeoLineLineIntersection(wrongDefenceLine,CGeoLine(CGeoPoint(Param::Field::PITCH_LENGTH / 2.0,-Param::Field::PITCH_WIDTH / 2.0),CGeoPoint(Param::Field::PITCH_LENGTH / 2.0,Param::Field::PITCH_WIDTH / 2.0)));
				if(pointA.Intersectant()){
					CGeoPoint pointAA = pointA.IntersectPoint();
					if (pointAA.y()>0){
						targetLine = CGeoLine(RGOAL_RIGHT_POS,transPoint);
					}else{
						targetLine = CGeoLine(RGOAL_LEFT_POS,transPoint);
					}
					intersect = CGeoLineEllipseIntersection(targetLine,RD_ELLIPSE);
					if(intersect.intersectant()){
						if(intersect.point1().dist(transPoint)<intersect.point2().dist(transPoint)){
							RDefenderPoint = intersect.point1();
						}else{
							RDefenderPoint = intersect.point2();
						}
					}else{
						cout<<"defend valid!!"<<endl;
					}
				}
			}
		} else if (mode == 1) {
			CVector transVector = Utils::Polar2Vector(PLAYERSIZE, Utils::Normalize(Rbasedir + sideFactor * Param::Math::PI / 2));
			CGeoPoint transPoint = RBasepoint + transVector;
			CGeoLine targetLine = CGeoLine(transPoint, Rbasedir);
			CGeoLineEllipseIntersection intersect = CGeoLineEllipseIntersection(targetLine, RD_ELLIPSE);

			if (intersect.intersectant()) {
				if (intersect.point1().dist(transPoint) < intersect.point2().dist(transPoint)) {
					RDefenderPoint = intersect.point1();
				} else {
					RDefenderPoint = intersect.point2();
				}
			} else {
				CGeoLine wrongDefenceLine = CGeoLine(reversePoint(transPoint),reversePoint(transPoint+Utils::Polar2Vector(500,Rbasedir)));
				//对球速线抖动造成invalid的处理
				CGeoLineLineIntersection pointA = CGeoLineLineIntersection(wrongDefenceLine, CGeoLine(CGeoPoint(Param::Field::PITCH_LENGTH / 2.0,-Param::Field::PITCH_WIDTH / 2.0),CGeoPoint(Param::Field::PITCH_LENGTH / 2.0,Param::Field::PITCH_WIDTH / 2.0)));
				if(pointA.Intersectant()){
					CGeoPoint pointAA = pointA.IntersectPoint();
					if (pointAA.y()>0){
						targetLine = CGeoLine(RGOAL_RIGHT_POS,transPoint);
					}else{
						targetLine = CGeoLine(RGOAL_LEFT_POS,transPoint);
					}
					intersect = CGeoLineEllipseIntersection(targetLine,RD_ELLIPSE);
					if(intersect.intersectant()){
						if(intersect.point1().dist(transPoint)<intersect.point2().dist(transPoint)){
							RDefenderPoint = intersect.point1();
						}else{
							RDefenderPoint = intersect.point2();
						}
					}else{
						cout<<"defend valid!!"<<endl;
					}
				}
			}

			bool SIDEBACK_MARKING_MODE;
			DECLARE_PARAM_READER_BEGIN(Defence)
				READ_PARAM(SIDEBACK_MARKING_MODE)
			DECLARE_PARAM_READER_END
			if (SIDEBACK_MARKING_MODE == true) {
				static bool markingMode = true;
				bool isQuaterFieldClear = true;
				for (int i = 1; i <= Param::Field::MAX_PLAYER; ++i) {
					if (pVision->TheirPlayer(i).Valid() == true) {
						const PlayerPoseT& player = pVision->TheirPlayer(i);
						if (player.X() < -RTarget.x() && RTarget.dist(reversePoint(player.Pos())) > PLAYERSIZE) {
							isQuaterFieldClear = false;
							break;
						}
					}
				}
				if (isQuaterFieldClear == false) {
					markingMode = false;
				} else {
					double dist = RDefenderPoint.dist(RTarget);
					if (markingMode == false) {
						if (dist < SIDEBACK_MARKING_DIST_THRESHOLD_UPPER - SIDEBACK_MARKING_DIST_THRESHOLD_BUFFER
								&& dist > SIDEBACK_MARKING_DIST_THRESHOLD_LOWER + SIDEBACK_MARKING_DIST_THRESHOLD_BUFFER)
							markingMode = true;
					} else {
						if (dist > SIDEBACK_MARKING_DIST_THRESHOLD_UPPER
							|| dist < SIDEBACK_MARKING_DIST_THRESHOLD_LOWER)
							markingMode = false;
					}
				}
				if (markingMode == true) {
					RDefenderPoint = (RTarget + Utils::Polar2Vector(Param::Field::MAX_PLAYER_SIZE, RDir + Param::Math::PI));
				}
			}
		} else if (mode == 3) {
			if (Utils::InOurPenaltyArea(pVision->Ball().Pos(), PENALTY_BUFFER) == true){
				int oppnum =  BestPlayer::Instance()->theirFastestPlayerToBallList().front().num;
				if (oppnum != 0 && vision->TheirPlayer(oppnum).Valid() == true){
					if(pVision->TheirPlayer(oppnum).Pos().y() > 0
						&& (pVision->TheirPlayer(oppnum).Pos() - GOAL_CENTRE_POS).dir() > (RIGHTBACK_CRITICAL_POINT-GOAL_CENTRE_POS).dir()) {
						defendbasepoint = RIGHTBACK_CRITICAL_POINT;
						RBasepoint = reversePoint(defendbasepoint);
						Rbasedir = (RGOAL_CENTRE_POS - RBasepoint).dir();
					} else if (pVision->TheirPlayer(oppnum).Pos().y() < 0
						&& (pVision->TheirPlayer(oppnum).Pos() - GOAL_CENTRE_POS).dir() < (LEFTBACK_CRITICAL_POINT-GOAL_CENTRE_POS).dir()) {
						defendbasepoint = LEFTBACK_CRITICAL_POINT;
						RBasepoint = reversePoint(defendbasepoint);
						Rbasedir = (RGOAL_CENTRE_POS - RBasepoint).dir();
					} else {
						defendbasepoint = pVision->TheirPlayer(oppnum).Pos();
						RBasepoint = reversePoint(defendbasepoint);
						Rbasedir = (RGOAL_CENTRE_POS - RBasepoint).dir();
					}
				}
			}
			CGeoLine targetLine = CGeoLine(RBasepoint, Rbasedir);
			double Rball2goalDir = (RGOAL_CENTRE_POS - RBasepoint).dir();
			CGeoLine targetLine1 = CGeoLine(RBasepoint,Rball2goalDir);
			CGeoLineEllipseIntersection intersect = CGeoLineEllipseIntersection(targetLine,RD_ELLIPSE3);
			CGeoLineEllipseIntersection intersect1 = CGeoLineEllipseIntersection(targetLine1,RD_ELLIPSE3);
			if (Utils::InOurPenaltyArea(vision->Ball().Pos(), PENALTY_BUFFER) == true){
				intersect = CGeoLineEllipseIntersection(targetLine,RD_ELLIPSE4);
				intersect1 = CGeoLineEllipseIntersection(targetLine1,RD_ELLIPSE4);
			}
			if (intersect.intersectant()) {
				if(intersect.point1().dist(RBasepoint)<intersect.point2().dist(RBasepoint)){
					RDefenderPoint = intersect.point1();
				}else{
					RDefenderPoint = intersect.point2();
				}
			} else {
				//不相交使用球门之间的相交线
				if(intersect1.intersectant()){
					if(intersect1.point1().dist(RBasepoint)<intersect1.point2().dist(RBasepoint)){
						RDefenderPoint = intersect1.point1();
					}else{
						RDefenderPoint = intersect1.point2();
					}
				}
			}
		}
		//GDebugEngine::Instance()->gui_debug_x(reversePoint(RDefenderPoint), COLOR_PURPLE);
		return RDefenderPoint;
	}

	bool isBallShotToTheGoal(){
		const CVisionModule* pVision = vision;
		const MobileVisionT& ball = pVision->Ball();
		const double ballVelDir = ball.Vel().dir();
		const double ballVel = ball.Vel().mod();

		if (ball.Pos().x() > -(Param::Field::PITCH_LENGTH/2 + Param::Field::PENALTY_AREA_DEPTH)
				&& ball.Pos().x() < Param::Field::PENALTY_AREA_DEPTH
				&& ballVel > 50
				&& ball.VelX() < 0) {
			const double RBallVelDir = Utils::Normalize(ballVelDir + Param::Math::PI);
			const CGeoPoint RBallPos = reversePoint(ball.Pos());
			const CGeoLine RBallVelLine(RBallPos, RBallVelDir);
			const CGeoPoint RLeftGoalPost(Param::Field::PITCH_LENGTH/2, -Param::Field::GOAL_WIDTH/2);
			const CGeoPoint RRightGoalPost(Param::Field::PITCH_LENGTH/2, Param::Field::GOAL_WIDTH/2);
			const CGeoLine RBaseLine(RLeftGoalPost, RRightGoalPost);
			const CGeoLineLineIntersection interseciton(RBaseLine, RBallVelLine);
			if (interseciton.Intersectant() == true) {
				const CGeoPoint& point = interseciton.IntersectPoint();
				if (point.y() < RRightGoalPost.y() + Param::Vehicle::V2::PLAYER_SIZE
						&& point.y() > RLeftGoalPost.y() - Param::Vehicle::V2::PLAYER_SIZE)
					return true;
			}
		}
		return false;
	}

	CGeoPoint calcGoaliePointV2(const CGeoPoint Rtarget,const double Rdir,const posSide Rside,CGeoPoint laststoredpoint,int mode){
		double RoriginX = 0;
		CGeoPoint RgoaliePoint;
		CVisionModule* pVision = vision;
		double m,n;
		bool targetvalid = true;
		static bool checkfrontparam = false;
		static bool firstinflag = false;
		static bool up = true;//判断是否向前的标志量
		/*if (BallIsInShootingTarget()){
			cout<<vision->Cycle()<<" ball is shooting"<<endl;
		}*/
		if (firstinflag){
			if(isBallShotToTheGoal() && !checkInDeadArea()){
				checkfrontparam = true;
				firstinflag = false;
				//cout<<"need to be front"<<endl;
			}
		}
		if (!isBallShotToTheGoal() && firstinflag == false){
			firstinflag = true;
			checkfrontparam = false;
		}
		//bool result = checkInDeadArea();
		//cout<<firstinflag<<" "<<checkfrontparam<<" "<<result<<endl;

		//up和down的buffer线
		if (up == true) {
			if (Rtarget.x()>140*Param::Field::RATIO)
				up = false;
		} else if (up == false) {
			if (Rtarget.x()<120*Param::Field::RATIO)
				up = true;
		}
		if (isBallShotToTheGoal() && checkfrontparam){
			up =true;
			//cout<<"shooting"<<endl;
		}
		//如果球在两边就不要向前了！
		if ((Rside == POS_SIDE_LEFT || Rside == POS_SIDE_RIGHT) && mode ==0){
			up = false;
			//cout<<"set Down"<<endl;
		}
		//cout<<vision->Cycle()<<" up param is "<<up<<endl;
		//if (Rtarget.x() <  150*Param::Field::RATIO  || (BallIsInShootingTarget() && checkfrontparam)){
		//	RoriginX = 270*Param::Field::RATIO;
		//	m = 40*Param::Field::RATIO;
		//	n = 80*Param::Field::RATIO;
		//	//靠前的参数
		//	GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(50,0),"front");
		//	up =true;
		//	//cout<<"front"<<endl;
		//} else{
		//	RoriginX = 290*Param::Field::RATIO;
		//	m = 10*Param::Field::RATIO;
		//	n = 45*Param::Field::RATIO;
		//	up = false;
		//	GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(50,0),"back");
		//	//cout<<"back"<<endl;
		//}
		//stop站下面
		if (vision->GetCurrentRefereeMsg()=="gameStop"){
			up = false;
		}
		if (up == true) {
			// 靠前的参数
			RoriginX = 292*Param::Field::RATIO;
			m = 40*Param::Field::RATIO;
			n = 80*Param::Field::RATIO;
			GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(50, 0),"front");
		} else {
			// 靠后的参数
			RoriginX = 312*Param::Field::RATIO;
			m = 10*Param::Field::RATIO;
			n = 45*Param::Field::RATIO;
			GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(50, 0),"back");
		}
		if (TaskMediator::Instance()->leftBack() == 0	&& TaskMediator::Instance()->rightBack() == 0	&& TaskMediator::Instance()->singleBack() == 0 ) {
				// 没有后卫时的参数 靠后
				RoriginX = 312*Param::Field::RATIO;
				m = 10*Param::Field::RATIO;
				n = 45*Param::Field::RATIO;
				up = false;
		}
		CGeoEllipse Rdefendellipse = CGeoEllipse(CGeoPoint(RoriginX,0),m,n);
		CGeoLine ReverseDefenceLine = CGeoLine(Rtarget,Rdir);
		GDebugEngine::Instance()->gui_debug_line(reversePoint(Rtarget),reversePoint(Rtarget + Utils::Polar2Vector(1000,Rdir)),COLOR_WHITE);
		CGeoLineEllipseIntersection intersect = CGeoLineEllipseIntersection(ReverseDefenceLine,Rdefendellipse);
		if (intersect.intersectant()){
			//cout<<Param::Field::GOAL_WIDTH<<endl;
			if(intersect.point1().dist(Rtarget)<intersect.point2().dist(Rtarget)){
				RgoaliePoint = intersect.point1();
			}else{
				RgoaliePoint = intersect.point2();
			}
			if (POS_SIDE_LEFT == Rside || POS_SIDE_RIGHT == Rside){
				if (RgoaliePoint.x() > (RoriginX - GoalieFrontBuffer) || RgoaliePoint.y()>n-5 || RgoaliePoint.y()<-n+5 ){
					targetvalid = false;
				}
			}
		}
		if (!intersect.intersectant() || !targetvalid){
			if (POS_SIDE_LEFT == Rside){
				RgoaliePoint = CGeoPoint(RoriginX-GoalieFrontBuffer,n-5);
			}else if (POS_SIDE_RIGHT == Rside){
				RgoaliePoint = CGeoPoint(RoriginX-GoalieFrontBuffer,-n+5);
			}else if (POS_SIDE_MIDDLE == Rside){
				CGeoLineLineIntersection pointA = CGeoLineLineIntersection(ReverseDefenceLine,CGeoLine(CGeoPoint(Param::Field::PITCH_LENGTH / 2.0,-Param::Field::PITCH_WIDTH / 2.0),CGeoPoint(Param::Field::PITCH_LENGTH / 2.0,Param::Field::PITCH_WIDTH / 2.0)));
				//GDebugEngine::Instance()->gui_debug_line(CGeoPoint(-300,200),CGeoPoint(-300,-200),COLOR_PURPLE);
				if(pointA.Intersectant()){
					CGeoPoint pointAA = pointA.IntersectPoint();
					if (pointAA.y()>0){
						if (up == true){
							ReverseDefenceLine = CGeoLine(RGOAL_RIGHT_POS,Rtarget);
							//向上的补充
							CGeoLine pointLine = CGeoLine(CGeoPoint(RoriginX-20,200),CGeoPoint(RoriginX-20,-200));
							CGeoLineLineIntersection intersectAA = CGeoLineLineIntersection(ReverseDefenceLine,pointLine);
							if (intersectAA.Intersectant()){
								RgoaliePoint = intersectAA.IntersectPoint();
							}else{
								cout<<"goalie invalid"<<endl;
							}
						}else{
							RgoaliePoint =CGeoPoint(RoriginX-GoalieFrontBuffer,n-5);
						}
					}else{
						if(up == true){
							ReverseDefenceLine = CGeoLine(RGOAL_LEFT_POS,Rtarget);
							CGeoLine pointLine = CGeoLine(CGeoPoint(RoriginX - 20,200),CGeoPoint(RoriginX - 20,-200));
							CGeoLineLineIntersection intersectAA = CGeoLineLineIntersection(ReverseDefenceLine,pointLine);
							if (intersectAA.Intersectant()){
								RgoaliePoint = intersectAA.IntersectPoint();
							}else{
								cout<<"goalie invalid"<<endl;
							}
						}else{
							RgoaliePoint =CGeoPoint(RoriginX-GoalieFrontBuffer ,n-5);
						}
					}
				}else{
					RgoaliePoint = CGeoPoint(RoriginX-m,0);
				}
			}
		}
		//cout<<"Rgoalie Point is "<<RgoaliePoint<<endl;
		GDebugEngine::Instance()->gui_debug_x(reversePoint(RgoaliePoint),COLOR_PURPLE);
		return RgoaliePoint;
	}

	CGeoPoint calcGoaliePointV3(const CGeoPoint& RTarget, double RDir, posSide RSide, const CGeoPoint& lastpoint, int mode) {
		double ROriginX = 0;
		CGeoPoint RGoaliePoint;
		CVisionModule* pVision = vision;
		double m, n;
		bool isTargetValid = true;
		static bool checkFrontParam = false;
		static bool firstInFlag = false;
		static bool up = false;

		if (firstInFlag == true) {
			if (isBallShotToTheGoal() == true && checkInDeadArea() == false) {
				checkFrontParam = true;
				firstInFlag = false;
			}
		}
		if (isBallShotToTheGoal() == false && firstInFlag == false) {
			firstInFlag = true;
			checkFrontParam = false;
		}

		// up和down的缓冲线
		if (up == true) {
			if (RTarget.x() > Param::Field::PITCH_LENGTH/4 - Param::Field::MAX_PLAYER_SIZE*2.5)
				up = false;
		} else {
			if (RTarget.x() < Param::Field::PITCH_LENGTH/4 - Param::Field::MAX_PLAYER_SIZE*4.5)
				up = true;
		}

		if (isBallShotToTheGoal() && checkFrontParam)
			up = true;

		// 如果球在两边就不要向前了！
		if ((RSide == POS_SIDE_LEFT || RSide == POS_SIDE_RIGHT) && mode == 0)
			up = false;

		// stop站下面
		if (vision->GetCurrentRefereeMsg() == "gameStop")
			up = false;

		// 没有后卫时不要向前
		int leftBack = TaskMediator::Instance()->leftBack();
		int rightBack = TaskMediator::Instance()->rightBack();
		int singleBack = TaskMediator::Instance()->singleBack();
		char backs = 0;
		if (up == true) {
			backs <<= 1; backs += (leftBack == 0 ? 0 : 1);
			backs <<= 1; backs += (rightBack == 0 ? 0 : 1);
			backs <<= 1; backs += (singleBack == 0 ? 0 : 1);
			switch (backs) {
				case 0: up = false; break;  // 2'b000
				case 1: up = true;  break;  // 2'b001
				case 2: up = false; break;  // 2'b010
				case 4: up = false; break;  // 2'b100
				case 6: up = true;  break;  // 2'b110
			}
		}

		// 后卫没到点时不要向前
		if (up == true) {
			const CGeoPoint& leftBackPosition = pVision->OurPlayer(leftBack).Pos();
			const CGeoPoint& rightBackPosition = pVision->OurPlayer(rightBack).Pos();
			const CGeoPoint& singleBackPosition = pVision->OurPlayer(singleBack).Pos();
			const CGeoPoint& leftBackTarget = DefPos2015::Instance()->getLeftPos();
			const CGeoPoint& rightBackTarget = DefPos2015::Instance()->getRightPos();
			const CGeoPoint& singleBackTarget = DefPos2015::Instance()->getSinglePos();
			if (backs == 1) {
				if (singleBackPosition.dist(singleBackTarget) > Param::Field::MAX_PLAYER_SIZE*2)
					up = false;
			} else if (backs == 6) {
				if (leftBackPosition.dist(leftBackTarget) > Param::Field::MAX_PLAYER_SIZE*2
						|| rightBackPosition.dist(rightBackTarget) > Param::Field::MAX_PLAYER_SIZE*2)
					up = false;
			}
		}

		if (up == true) {
			// 靠前的椭圆
			ROriginX = Param::Field::PITCH_LENGTH/2 + Param::Vehicle::V2::PLAYER_SIZE;
			m = Param::Field::PENALTY_AREA_DEPTH - Param::Vehicle::V2::PLAYER_SIZE*5;
			n = Param::Field::GOAL_WIDTH + Param::Vehicle::V2::PLAYER_SIZE;
		} else {
			// 靠后的椭圆
			ROriginX = Param::Field::PITCH_LENGTH/2 - Param::Vehicle::V2::PLAYER_SIZE/3;
			//m = Param::Vehicle::V2::PLAYER_SIZE*1.5;// 紧贴goalline
			m = Param::Field::PENALTY_AREA_DEPTH - Param::Vehicle::V2::PLAYER_SIZE * 5; //浪逼
			n = Param::Field::GOAL_WIDTH/2 + Param::Vehicle::V2::PLAYER_SIZE;
		}

		CGeoEllipse RDefendEllipse = CGeoEllipse(CGeoPoint(ROriginX, 0), m, n);
		CGeoLine reverseDefenceLine = CGeoLine(RTarget, RDir);
		CGeoLineEllipseIntersection intersect = CGeoLineEllipseIntersection(reverseDefenceLine, RDefendEllipse);
		if (intersect.intersectant() == true) {
			if(intersect.point1().dist(RTarget) < intersect.point2().dist(RTarget))
				RGoaliePoint = intersect.point1();
			else
				RGoaliePoint = intersect.point2();
			if (POS_SIDE_LEFT == RSide || POS_SIDE_RIGHT == RSide) {
				if (RGoaliePoint.x() > ROriginX - Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER
						|| RGoaliePoint.y() > n - Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER
						|| RGoaliePoint.y() < -(n - Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER))
					isTargetValid = false;
			}
		}
		if (intersect.intersectant() == false || isTargetValid == false){
			if (POS_SIDE_LEFT == RSide) {
				double x = Param::Field::PITCH_LENGTH/2.0 - Param::Vehicle::V2::PLAYER_SIZE - 2;
				double y = Param::Field::GOAL_WIDTH/2.0 + 1;
				RGoaliePoint = CGeoPoint(x, y);
			} else if (POS_SIDE_RIGHT == RSide) {
				double x = Param::Field::PITCH_LENGTH/2.0 - Param::Vehicle::V2::PLAYER_SIZE - 2;
				double y = -Param::Field::GOAL_WIDTH/2.0 - 1;
				RGoaliePoint = CGeoPoint(x, y);
			} else if (POS_SIDE_MIDDLE == RSide) {
				CGeoLine baseLine(CGeoPoint(Param::Field::PITCH_LENGTH/2.0 - GoalieFrontBuffer, -Param::Field::PITCH_WIDTH/2.0),
													CGeoPoint(Param::Field::PITCH_LENGTH/2.0 - GoalieFrontBuffer, Param::Field::PITCH_WIDTH/2.0));
				CGeoLineLineIntersection intersectionA = CGeoLineLineIntersection(reverseDefenceLine, baseLine);
				if(intersectionA.Intersectant() == true) {
					CGeoPoint pointA = intersectionA.IntersectPoint();
					if (pointA.y() > 0) {
						if (up == true) {
							// 向前的补充
							reverseDefenceLine = CGeoLine(RGOAL_RIGHT_POS, RTarget);
							CGeoLine pointLine = CGeoLine(CGeoPoint(ROriginX - Param::Field::PENALTY_AREA_DEPTH/4, Param::Field::PITCH_WIDTH/2),
																						CGeoPoint(ROriginX - Param::Field::PENALTY_AREA_DEPTH/4, -Param::Field::PITCH_WIDTH/2));
							CGeoLineLineIntersection intersectionB = CGeoLineLineIntersection(reverseDefenceLine, pointLine);
							if (intersectionB.Intersectant() == true)
								RGoaliePoint = intersectionB.IntersectPoint();
							else
								cout << "goalie invalid" << endl;
						} else {
							RGoaliePoint = CGeoPoint(ROriginX - GoalieFrontBuffer, n - Param::Vehicle::V2::PLAYER_SIZE);
						}
					} else {
						if(up == true) {
							reverseDefenceLine = CGeoLine(RGOAL_LEFT_POS, RTarget);
							CGeoLine pointLine = CGeoLine(CGeoPoint(ROriginX - Param::Field::PENALTY_AREA_DEPTH/4, Param::Field::PITCH_WIDTH/2),
																						CGeoPoint(ROriginX - Param::Field::PENALTY_AREA_DEPTH/4, -Param::Field::PITCH_WIDTH/2));
							CGeoLineLineIntersection intersectionB = CGeoLineLineIntersection(reverseDefenceLine, pointLine);
							if (intersectionB.Intersectant() == true)
								RGoaliePoint = intersectionB.IntersectPoint();
							else
								cout << "goalie invalid" << endl;
						} else {
							RGoaliePoint =CGeoPoint(ROriginX - GoalieFrontBuffer, -n + Param::Vehicle::V2::PLAYER_SIZE);
						}
					}
				} else {
					RGoaliePoint = CGeoPoint(ROriginX - m, 0);
				}
			}
		}

		// 球射门后守门员去截球
		const MobileVisionT& ball = pVision->Ball();
		const CGeoPoint RBallPos = reversePoint(ball.Pos());
		const int goalieNum = PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie());
    //cout << goalieNum << endl;
		const double RBallToGoalDir = (RGOAL_CENTRE_POS - RBallPos).dir();
		if (isBallShotToTheGoal() && RBallPos.x() > 0 && std::fabs(RBallToGoalDir) < Param::Math::PI*60/180) {
      const CGeoPoint& RCurrentGoaliePos = reversePoint(pVision->OurPlayer(goalieNum).Pos());
      //GDebugEngine::Instance()->gui_debug_arc(pVision->OurPlayer(goalieNum).RawPos(), 5, 0, 360, COLOR_WHITE);
			const double RBallVelDir = Utils::Normalize(ball.Vel().dir() + Param::Math::PI);
			const double RPatrolDir = Utils::Normalize(RBallVelDir + Param::Math::PI/2);
			const CGeoLine RBallVelLine(RBallPos, RBallVelDir);
			const CGeoLine RPatrolLine(RCurrentGoaliePos, RPatrolDir);
			const CGeoLineLineIntersection intersection(RPatrolLine, RBallVelLine);
			if (intersection.Intersectant() == true) {
				const CGeoPoint& point = intersection.IntersectPoint();
				RGoaliePoint.setX(point.x());
				RGoaliePoint.setY(point.y());
			}
    }

		// 输出Debug信息
		if (false) {
			GDebugEngine::Instance()->gui_debug_arc(CGeoPoint(-ROriginX + m, 0), 2, 0, 360, COLOR_RED);
			GDebugEngine::Instance()->gui_debug_arc(CGeoPoint(-ROriginX - m, 0), 2, 0, 360, COLOR_RED);
			GDebugEngine::Instance()->gui_debug_arc(CGeoPoint(-ROriginX, n), 2, 0, 360, COLOR_RED);
			GDebugEngine::Instance()->gui_debug_arc(CGeoPoint(-ROriginX, -n), 2, 0, 360, COLOR_RED);
			//if (intersect.intersectant() == true) {
			//	GDebugEngine::Instance()->gui_debug_arc(reversePoint(intersect.point1()), 5, 0, 360, COLOR_WHITE);
			//	GDebugEngine::Instance()->gui_debug_arc(reversePoint(intersect.point2()), 5, 0, 360, COLOR_WHITE);
			//} else {
			//	GDebugEngine::Instance()->gui_debug_line(CGeoPoint(-ROriginX + Param::Field::PENALTY_AREA_DEPTH/4, Param::Field::PITCH_WIDTH/2),
			//																					CGeoPoint(-ROriginX + Param::Field::PENALTY_AREA_DEPTH/4, -Param::Field::PITCH_WIDTH/2),
			//																					COLOR_WHITE);
			//}

			if (up == true)
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(50, 0), "front");
			else
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(50, 0), "back");

			//GDebugEngine::Instance()->gui_debug_line(reversePoint(RTarget),
			//																				reversePoint(RTarget + Utils::Polar2Vector(1000, RDir)),
			//																				COLOR_WHITE);
			//GDebugEngine::Instance()->gui_debug_arc(reversePoint(RGoaliePoint), 10, 0, 360, COLOR_WHITE);
		}
		return RGoaliePoint;
	}

	bool inHalfCourt(CGeoPoint target,double dir, int sideFactor){
 		bool result = false;
		double checkDir = (target - GOAL_CENTRE_POS).dir();
		if (sideFactor == 1){
			if (!Utils::OutOfField(target,-10) && checkDir>dir){
				result =true;
			}
		}else if (sideFactor == -1){
			if (!Utils::OutOfField(target,-10) && checkDir<dir){
				result =true;
			}
		}
		return result;
	}

	CGeoLine getSideBackTargetAndLine(CGeoPoint& RSideTarget, double& RSideDir){
		CVisionModule* pVision = vision;
		double ourgoal2BallDir = (pVision->Ball().Pos() - GOAL_CENTRE_POS).dir();

		//sideFactor = 1代表sideback应该站在右边
		static int sideFactor = 1;
		if (Utils::InOurPenaltyArea(pVision->Ball().Pos(), PENALTY_BUFFER) == false
				&& pVision->Ball().Valid() == true
				&& pVision->Ball().X() <= 0) {
			if (sideFactor == 1) {
				if (ourgoal2BallDir > SIDEBACK_SIDEFACTOR_BUFFER_DIR)
					sideFactor = -1;
			} else if (sideFactor == -1) {
				if (ourgoal2BallDir < -SIDEBACK_SIDEFACTOR_BUFFER_DIR)
					sideFactor = 1;
			}
		} else if (Utils::InOurPenaltyArea(pVision->Ball().Pos(), PENALTY_BUFFER) == false
							&& pVision->Ball().Valid() == true
							&& pVision->Ball().X() > 0) {
				if (sideFactor == 1) {
					if (pVision->Ball().Y() > SIDEBACK_BUFFER_DIST)
						sideFactor = -1;
				} else if (sideFactor == -1) {
					if (pVision->Ball().Y() < -SIDEBACK_BUFFER_DIST)
						sideFactor = 1;
				}
		} else if (Utils::InOurPenaltyArea(pVision->Ball().Pos(), PENALTY_BUFFER) == true
							|| pVision->Ball().Valid() == false) {
			int normalDefendNum = BestPlayer::Instance()->getTheirBestPlayer();
			if (normalDefendNum != 0) {
				double enemy2OurgoalDir = (pVision->TheirPlayer(normalDefendNum).Pos() - GOAL_CENTRE_POS).dir();
				if (sideFactor == 1) {
					if (enemy2OurgoalDir > SIDEBACK_SIDEFACTOR_BUFFER_DIR)
						sideFactor = -1;
				} else if (sideFactor == -1) {
					if (enemy2OurgoalDir < -SIDEBACK_SIDEFACTOR_BUFFER_DIR)
						sideFactor = 1;
				}
			} else {
				if (sideFactor == 1) {
					if (ourgoal2BallDir > SIDEBACK_SIDEFACTOR_BUFFER_DIR)
						sideFactor = -1;
				} else if(sideFactor == -1) {
					if (ourgoal2BallDir < -SIDEBACK_SIDEFACTOR_BUFFER_DIR)
						sideFactor = 1;
				}
			}
		}

		int defendnum = 0;
		double mindist = 1000;
		const CBestPlayer::PlayerList& bestPlayerList = BestPlayer::Instance()->theirFastestPlayerToBallList();
		for (CBestPlayer::PlayerList::const_iterator it = bestPlayerList.cbegin(); it != bestPlayerList.cend(); ++it) {
			const PlayerVisionT& player = pVision->TheirPlayer(it->num);
			if (player.Pos().x() < SIDEBACK_DEFEND_CRITICAL_X) {
				double dist = player.Pos().dist(GOAL_CENTRE_POS);
				double dir = (player.Pos() - GOAL_CENTRE_POS).dir();
				if (dist < mindist) {
					if (sideFactor == 1) {
						if (dir > 0) {
							defendnum = it->num;
							mindist = dist;
						}
					} else if (sideFactor == -1) {
						if (dir < 0) {
							defendnum = it->num;
							mindist = dist;
						}
					}
				}
			}
		}

		if (defendnum == 0) {
			RSideTarget = CGeoPoint(0, -Param::Field::PITCH_WIDTH / 2 * sideFactor);
			RSideDir = (RSideTarget - RGOAL_CENTRE_POS).dir();
		} else {
			const PlayerVisionT& player = pVision->TheirPlayer(defendnum);
			const double dir = (player.Pos() - GOAL_CENTRE_POS).dir();
			const double dist = player.Pos().dist(GOAL_CENTRE_POS);
			if (sideFactor == 1) {
				if (dir > SIDEBACK_DEFEND_BOUNDARY_DIR_MIN && dir < SIDEBACK_DEFEND_BOUNDARY_DIR_MAX) {
					RSideTarget = reversePoint(player.Pos());
					RSideDir = Utils::Normalize(dir + Param::Math::PI);
				} else if (dir <= SIDEBACK_DEFEND_BOUNDARY_DIR_MIN) {
					RSideTarget = reversePoint(GOAL_CENTRE_POS + Utils::Polar2Vector(dist, SIDEBACK_DEFEND_BOUNDARY_DIR_MIN));
					RSideDir = Utils::Normalize(SIDEBACK_DEFEND_BOUNDARY_DIR_MIN + Param::Math::PI);
        } else {  // dir >= SIDEBACK_DEFEND_BOUNDARY_DIR_MAX
					RSideTarget = reversePoint(GOAL_CENTRE_POS + Utils::Polar2Vector(dist, SIDEBACK_DEFEND_BOUNDARY_DIR_MAX));
					RSideDir = Utils::Normalize(dir + Param::Math::PI);
        }
			} else if (sideFactor == -1) {
				if (dir > -SIDEBACK_DEFEND_BOUNDARY_DIR_MAX && dir < -SIDEBACK_DEFEND_BOUNDARY_DIR_MIN) {
					RSideTarget = reversePoint(player.Pos());
					RSideDir = Utils::Normalize(dir + Param::Math::PI);
        } else if (dir <= -SIDEBACK_DEFEND_BOUNDARY_DIR_MAX) {
					RSideTarget = reversePoint(GOAL_CENTRE_POS + Utils::Polar2Vector(dist, -SIDEBACK_DEFEND_BOUNDARY_DIR_MAX));
					RSideDir = Utils::Normalize(dir + Param::Math::PI);
        } else {  // dir >= -SIDEBACK_DEFEND_BOUNDARY_DIR_MIN
					RSideTarget = reversePoint(GOAL_CENTRE_POS + Utils::Polar2Vector(dist, -SIDEBACK_DEFEND_BOUNDARY_DIR_MIN));
					RSideDir = Utils::Normalize(-SIDEBACK_DEFEND_BOUNDARY_DIR_MIN + Param::Math::PI);
        }
			}
		}

		if (false) {
			GDebugEngine::Instance()->gui_debug_line(reversePoint(RSideTarget), reversePoint(RSideTarget + Utils::Polar2Vector(500, RSideDir)));
		}
		return CGeoLine(RSideTarget, RSideTarget + Utils::Polar2Vector(500, RSideDir));
	}

	CGeoPoint calcPenaltyLinePoint(const double dir, const posSide side,double ratio){
		CGeoPoint result;
		int sideFactor;
		if (POS_SIDE_LEFT == side){
			sideFactor = 1;
		} else if (POS_SIDE_RIGHT == side){
			sideFactor = -1;
		} else if (POS_SIDE_MIDDLE == side){
			sideFactor = 0;
		}
		CVector transVector = Utils::Polar2Vector(PLAYERSIZE,Utils::Normalize(dir - sideFactor * Param::Math::PI / 2));
		CGeoPoint transPoint = DCENTERPOINT + transVector;
		CGeoLine targetLine = CGeoLine(transPoint,dir);
		/*
		double x = Param::Field::PENALTY_AREA_DEPTH +3 +AVOIDBUFFER;
		double y = (Param::Field::PENALTY_AREA_WIDTH + 2 +AVOIDBUFFER*2)/2;
		x = x + ratio*5;
		y = y + ratio*5;
		CGeoEllipse penaltyEllipse = CGeoEllipse(DCENTERPOINT,x,y);*/

		double x = RD_ELLIPSE2.Xaxis();
		double y = RD_ELLIPSE2.Yaxis();
		CGeoEllipse penaltyEllipse = CGeoEllipse(DCENTERPOINT, x, y);
		CGeoLineEllipseIntersection intersect = CGeoLineEllipseIntersection(targetLine, penaltyEllipse);
		if (intersect.intersectant()){
			if(intersect.point1().x() >= -Param::Field::PITCH_LENGTH/2){
				result = intersect.point1();
			}else{
				result = intersect.point2();
			}
		}else{
			cout<<"penalty line invalid"<<endl;
		}
		//GDebugEngine::Instance()->gui_debug_line(result,transPoint,COLOR_BLACK);
		return result;
	}

	CGeoPoint getCornerAreaPos(){
		CVisionModule* pVision = vision;
		return CornerAreaPos::Instance()->getCornerAreaPos(pVision);
	}

	CGeoPoint getIndirectDefender(double radius,CGeoPoint leftUp,CGeoPoint rightDown,int mode){
		CVisionModule* pVision = vision;
		return IndirectDefender::Instance()->getDefPos(pVision,radius,leftUp,rightDown,mode);
	}

	bool BallIsToPenaltyArea(){
		bool result = false;
		CVisionModule* pVision = vision;
		const MobileVisionT ball = pVision->Ball();
		double ballVeldirection = ball.Vel().dir();
		double ballVel = ball.Vel().mod();
		if (ballVel >50){
			CGeoLine targetLine = CGeoLine(ball.Pos(),ballVeldirection);
			CGeoLineEllipseIntersection intersect = CGeoLineEllipseIntersection(targetLine,D_ELLIPSE);
			if (intersect.intersectant()){
				result = true;
			}
		}
		return result;
	}

	bool checkInDeadArea(){
		CVisionModule* pVision = vision;
		double x = pVision->Ball().X();
		double y = pVision->Ball().Y();
		bool result = false;
		//球在死角区域内返回true
		if ((!Utils::InOurPenaltyArea(pVision->Ball().Pos(),30) && x >-Param::Field::PITCH_LENGTH / 2.0*Param::Field::RATIO && x<-160*Param::Field::RATIO &&
			(4*y - 5*x - 1500*Param::Field::RATIO)>0 && y >0 && y<Param::Field::PITCH_WIDTH / 2.0*Param::Field::RATIO)||
			(!Utils::InOurPenaltyArea(pVision->Ball().Pos(),30) && x >-Param::Field::PITCH_LENGTH / 2.0*Param::Field::RATIO && x<-160*Param::Field::RATIO &&
			(4*y + 5*x + 1500*Param::Field::RATIO)<0 && y <0 && y>-Param::Field::PITCH_WIDTH*Param::Field::RATIO)){
				result = true;
		}
		return result;
	}

	CGeoPoint getMiddleDefender(double bufferX){
		int enemyNum = BestPlayer::Instance()->getTheirBestPlayer();
		const PlayerVisionT& enemy = vision->TheirPlayer(enemyNum);
		double defendDir = 0;
		CGeoPoint RenemyPos = reversePoint(enemy.Pos());
		CGeoPoint RballPos = reversePoint(vision->Ball().Pos());
		double RenemyDir = Utils::Normalize(enemy.Dir() + Param::Math::PI);
		if (Utils::InBetween(RenemyDir,(RLEFT - RenemyPos).dir(),(RRIGHT - RenemyPos).dir()),0){
			defendDir = RenemyDir;
		}else{
			defendDir = (RballPos - RenemyPos).dir();
		}
		defendDir = (RballPos - RenemyPos).dir();
		CGeoLine defendLine = CGeoLine(RenemyPos,defendDir);
		//GDebugEngine::Instance()->gui_debug_line(reversePoint(RenemyPos),reversePoint(RenemyPos+Utils::Polar2Vector(500,defendDir)));
		CGeoLine intersectLine = CGeoLine(CGeoPoint(-bufferX,-300),CGeoPoint(-bufferX,300));
		CGeoLineLineIntersection intersect= CGeoLineLineIntersection(defendLine,intersectLine);
		CGeoPoint tempPoint = intersect.IntersectPoint();

		return reversePoint(tempPoint);
	}

	CGeoPoint getDefaultPos(int index){
		return DefaultPos::Instance()->getDefaultPosbyIndex(index);
	}

	double calcBalltoOurPenaty(){
		CVisionModule* pVision = vision;
		CGeoPoint BallPos = vision->Ball().Pos();
		CGeoPoint OurGoalPos = CGeoPoint(-450,0);
		CVector Ball2OurGoalV = CVector(BallPos.x()+450,BallPos.y());
		double Ball2OurGoal_len = Ball2OurGoalV.mod();
		CGeoLine Ball2OurGoalL(OurGoalPos,BallPos);
		if (abs(Ball2OurGoalV.dir())<0.25){					 //在直线的范围内
			CGeoLine PenaltyAreaLine =  CGeoLine(CGeoPoint(-350,25),CGeoPoint(-350,-25));
			CGeoPoint Point = CGeoLineLineIntersection(Ball2OurGoalL,PenaltyAreaLine).IntersectPoint();
			double Ball2Point_len = CVector(Point.x()-BallPos.x(),Point.y()-BallPos.y()).mod();
			return Ball2Point_len;
		}
		else {												 //在圆弧的范围内
			int flag = 1;
			if (BallPos.y()<0) flag = -1;
			CGeoPoint CentralPoint = CGeoPoint(-450,25*flag);//确定圆心
			CGeoCirlce Arc = CGeoCirlce(CentralPoint,100);
			CGeoLineCircleIntersection a = CGeoLineCircleIntersection(Ball2OurGoalL,Arc);
			CGeoPoint Point =  a.point1().x()>=-450 ? a.point1():a.point2();
			double Ball2Point_len = CVector(Point.x()-BallPos.x(),Point.y()-BallPos.y()).mod();
			return Ball2Point_len;
		}
	}
}
