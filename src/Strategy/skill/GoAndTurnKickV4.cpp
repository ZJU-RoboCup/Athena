#include "GoAndTurnKickV4.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include <utils.h>
#include <DribbleStatus.h>
#include "BallSpeedModel.h"
#include <RobotSensor.h>
#include <KickStatus.h>

namespace{
	//>拿球的主状态机
	enum get_ball_state{
		DIRECTGOTO = 1,
		GETBALL,
		AVOIDBALL
	};
	//>AVOIDBALL状态下的避球状态分布
	enum avoid_ball_state{
		NOAVOID = 1,
		BALLBEHINDME,
		BALLBESIDEME
		//待添加
	};
	avoid_ball_state ab_state = NOAVOID;
	//>GETBALL的状态分布
	enum getball_state{
		SMALLANGLE = 1,
		LARGEANGLE
	};
	getball_state gb_state = LARGEANGLE;
	//>状态执行查看器 TODO

	//>需要用到的常量
	const double newVehicleBuffer = 0.6;               // 小嘴巴机器人PLAYER_FRONT_TO_CENTER补偿
	const double ballPredictBaseTime = 0.2;            // 球预测的基础时间
	const double SpeedLimitForPredictTime = 120;       // 球在该速度以上则加大预测时间
	const double nearBallRadius = 12;                  // 小车半径+球半径+2~3视频误差裕度,判定是否需要AVOIDBALL的半径
	const double ball2myheadLimit = 3;                 // 小嘴巴车嘴长7cm，球半径2cm，想要稳定拿住球需要(7-2)/2=2.5cm 再加1.0的余值
	const double avoidBallSuccessDist = 25;            // 判定避球成功的距离 25
	// 在转身时将这几个值进行修改，directGetBalDist要比minGetBallDist大3cm
	double minGetBallDist = 17;				   // 最小拿球距离
	double directGetBallDist = 20;               // 直接冲上去拿球的距离
	double maxGetBallDist = 30;				   // 25
	const double AllowFaceToFinalDist = 200;           // 进入该距离后开始转向
	const double transverseBallSpeed = 20;             // 对拿球产生影响的最低横向球速 ori：30
	const bool Allow_Start_Dribble = true;             // 控球控制阈值
	const double DribbleLevel = 3;                     // 控球力度等级	
	const double extremeAngle = Param::Math::PI/* * 176 / 180.0*/;
	const double directGetBballDirLimit = Param::Math::PI/6.0;
	const int maxFrared = 100;	//红外极大值
	// 在Begin时记住需要转的角度
	double turnAngleInBegin = 0;
	//>开关量
	bool Verbose = false;                              // 调试模式
	const bool USE_BALL_SPEED_MODEL = false;           // 应用球速模型

	//用到的静态变量
	bool trueNeedAvoidBall = false;
	int avoidBallCount = 0;
	int fraredOn = 0;
	int fraredOff = 0;
}

CGoAndTurnKickV4::CGoAndTurnKickV4()
{
	_lastCycle = 0;
}

void CGoAndTurnKickV4::plan(const CVisionModule* pVision)
{
	static bool isIn = true;
	if (Verbose)
	{
		if (isIn == true)
		{
			setState(BEGINNING);
			cout<<endl<<"get into CGetBallV3 !"<<endl;
			isIn = false;
		}
	}
	// 内部状态进行重置
	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1 ){
		setState(BEGINNING);
		ab_state = NOAVOID;
		gb_state = LARGEANGLE;
		trueNeedAvoidBall = false;
		avoidBallCount = 0;
		isIn = true;
	}
	//////////////////////////////////////////////////////////////////////////
	//视觉初步处理
	const MobileVisionT& ball = pVision->Ball();
	const int robotNum = task().executor;
	CTRL_METHOD mode = task().player.specified_ctrl_method;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const CGeoPoint myhead = me.Pos()+Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + newVehicleBuffer,me.Dir());
	const CVector self2ball = ball.Pos() - me.Pos();
	const CVector ball2self = me.Pos() - ball.Pos();
	const CVector head2ball = ball.Pos() - myhead;
	const double StopDist = task().player.rotvel;
	// 对小球的位置做预测
	double dAngle_ball2myhead_ballvel = Utils::Normalize(Utils::Normalize(head2ball.dir()+Param::Math::PI)-ball.Vel().dir());
	double dAngle_self2ball_medir = Utils::Normalize(self2ball.dir()-me.Dir());
	double BallPosWithVelFactorTmp = ballPredictBaseTime;
	double finalDir = /*(task().player.pos - ball.Pos()).dir()*/task().player.angle;
	double dAngle_finalDir2ballVel = Utils::Normalize(finalDir - ball.Vel().dir());
	CGeoPoint ballPosWithVel = CGeoPoint(0,0);
	CVector ballVel = ball.Vel();
	// 球速过大，背向拿球，加大预测
	if (ball.Vel().mod() > SpeedLimitForPredictTime && fabs(dAngle_finalDir2ballVel) > Param::Math::PI * 2 / 3) {
		BallPosWithVelFactorTmp += 0.2;
	}
	// 球在车正前方对球预测时间的影响，考虑红外
	bool frared = RobotSensor::Instance()->IsInfraredOn(robotNum);
	//带有小缓存功能的红外
	if ( frared ){
		fraredOn = fraredOn >= maxFrared ? maxFrared : fraredOn+1;
		fraredOff = 0;
	}
	else{
		fraredOn = 0;
		fraredOff = fraredOff >= maxFrared ? maxFrared : fraredOff+1;
	}
	if (fabs(dAngle_self2ball_medir) < Param::Vehicle::V2::KICK_ANGLE) {
		if (fabs(dAngle_ball2myhead_ballvel) < Param::Math::PI/6.0) {                      // 球朝车头滚过来,预测时间减小
			BallPosWithVelFactorTmp *= sin(fabs(dAngle_ball2myhead_ballvel));
		} else if (fabs(dAngle_ball2myhead_ballvel) > Param::Math::PI*5/6.0 &&
			fabs(Utils::Normalize(ball.Vel().dir() - finalDir)) < Param::Math::PI/6.0)               // 车追球，预测时间增大
		{
			if (fraredOn < 20)
			{
				BallPosWithVelFactorTmp += 0.2;
				BallPosWithVelFactorTmp *= cos(fabs(dAngle_ball2myhead_ballvel)) * -1;
			}else{
				//红外出现时特殊处理：将球速大小预测值改为0
				ballVel = Utils::Polar2Vector(0.01,ball.Vel().dir());
			}
		}
	}
	//预测BallPosWithVelFactorTmp时间之后的球位置
	ballPosWithVel = ball.Pos() + ballVel * BallPosWithVelFactorTmp;                           // 粗略计算预测球的位置
	//小球速预测
	if (ball.Vel().mod() < 20.0 && ball.Vel().mod() * sin(fabs(Utils::Normalize(ball.Vel().dir() - finalDir))) > 10)
	{
		ballPosWithVel = ball.Pos() + Utils::Polar2Vector(Param::Field::BALL_SIZE * 1.2,ball.Vel().dir());
	}
	//GDebugEngine::Instance()->gui_debug_x(ballPosWithVel,COLOR_YELLOW);
	// 计算相关变量赋值
	const CGeoLine myheadLine = CGeoLine(myhead,Utils::Normalize(me.Dir() + Param::Math::PI/2.0));
	const CGeoPoint ball2myheadLine_ProjPoint = myheadLine.projection(ballPosWithVel);
	double reverse_finalDir = Utils::Normalize(finalDir+Param::Math::PI);
	double dAngDiff_self2ball_finaldir = fabs(Utils::Normalize(self2ball.dir()-finalDir));
	CGeoLine ball_finalDir_line = CGeoLine(ball.Pos(),ball.Pos()+Utils::Polar2Vector(50,Utils::Normalize(finalDir)));

	//////////////////////////////////////////////////////////////////////////
	//拿球Task初始化
	TaskT getball_task(task());
	getball_task.player.angle = finalDir;
	getball_task.player.rotvel = 0.0;

	// 标签设置说明如下
	/*
	拿球标签: PlayerStatus::GOTO_GRASP_BALL	 --> 不管调用该任务的上层是否有设置，都强行予以设置，表征拿球
	避球标签: PlayerStatus::DODGE_BALL		 --> 不管调用该任务的上层是否有设置，都强行不予设置，表征不避球
	控球标签: PlayerStatus::DRIBBLING		 --> 结合上层决定是否开启控球
	*/
	getball_task.player.flag = getball_task.player.flag | PlayerStatus::DODGE_OUR_DEFENSE_BOX;//躲避我方禁区!!
	getball_task.player.flag = getball_task.player.flag;	//设置拿球标签
	getball_task.player.flag = getball_task.player.flag & (~PlayerStatus::DODGE_BALL);		//取消避球标签
	if( !(getball_task.player.flag & PlayerStatus::DRIBBLING) )
		getball_task.player.flag = getball_task.player.flag & (~PlayerStatus::DRIBBLING);	//取消控球标签

	//////////////////////////////////////////////////////////////////////////
	//状态判断模块
	bool isBallFrontOfMyhead = myhead.dist(ball2myheadLine_ProjPoint) < ball2myheadLimit ? true : false; //球到嘴巴线的投影点在可以拿球的范围内
	bool isInNearBallCircle = me.Pos().dist(ball.Pos()) < nearBallRadius ? true : false ;            //是否在AVOIDBALL小圈之内
	bool isInDirectGetBallCircle = me.Pos().dist(ballPosWithVel) < directGetBallDist ? true : false ;    //是否在直接冲上去拿球距离之内
	bool isGetBallDirReached = fabs(dAngDiff_self2ball_finaldir) < directGetBballDirLimit;
	bool isCanDirectGetBall = isBallFrontOfMyhead && isInDirectGetBallCircle && isGetBallDirReached;     //重要布尔量:是否能直接上前拿球
	bool dirOut = fabs(dAngDiff_self2ball_finaldir) > Param::Math::PI /5.0;
	bool canNOTDirectGetBall = !isBallFrontOfMyhead || !isInDirectGetBallCircle || dirOut;
	bool isBallBesideMe = false;                                                                         
	bool isBallBehindMe = false;
	if (isInNearBallCircle)
	{
		if (canNOTDirectGetBall)
		{
			if (fabs(dAngDiff_self2ball_finaldir) > Param::Math::PI / 2.0)
			{
				isBallBehindMe = true;
			} else isBallBesideMe = true;
		}
	}
	bool needAvoidBall = isBallBehindMe || isBallBesideMe;                                                //重要布尔量：是否需要躲避球
	if (Verbose)
	{
		//cout<<"isInNearBallCircle :"<<me.Pos().dist(ball.Pos())<<endl;
	}
		
	if (Verbose)
	{
		//cout<<"FrontOfMyhead: "<<isBallFrontOfMyhead<<"  InGetBallCircle: "<<isInDirectGetBallCircle<<"  DirReached: "<<isGetBallDirReached<<" dirOut: "<<dirOut<<endl;
	}
	//通过红外进一步检测是否需要avoidBall
	if (RobotSensor::Instance()->IsInfraredOn(robotNum))
	{
		needAvoidBall = false;
		if (Verbose)
		{
			//cout<<"fraredon!!! NONEED avoidBall"<<endl;
		}		
	}
	double isAvoidBallSuccess = nearBallRadius *(1.2 + dAngDiff_self2ball_finaldir / Param::Math::PI);
	bool avoidBallSuccess = me.Pos().dist(ball.Pos()) > isAvoidBallSuccess ? true : false;          //重要布尔量：是否躲避球成功，避球点在执行模块中计算
	if (Verbose)
	{
		//cout<<" avoidBallSuccess :  "<<avoidBallSuccess<<endl;
	}
	if (AVOIDBALL != getState())
	{
		if (needAvoidBall)
		{
			avoidBallCount++;
		} else avoidBallCount = 0;
		if (avoidBallCount > 15)//15
		{
			trueNeedAvoidBall = true;
		}
	}
    if (Verbose)
    {
		//cout<<"avoidBallCount: "<<avoidBallCount<<"  trueNeedAvoidBall: "<<trueNeedAvoidBall<<endl;
    }
	//////////////////////////////////////////////////////////////////////////
	//状态管理模块 *加入状态输出，便于查看状态跳转TODO
	if (BEGINNING == getState())             //当前状态为BEGINNING
	{
		turnAngleInBegin = fabs(Utils::Normalize(me.Dir() - finalDir))*180/Param::Math::PI;
		if (isCanDirectGetBall)
		{
			setState(DIRECTGOTO);
			if (Verbose)
			{
				cout<<"-->DirectGoto";
			}
		} else if (trueNeedAvoidBall)
		{
			setState(AVOIDBALL);
			if (Verbose)
			{
				cout<<"-->AvoidBall";
			}
			trueNeedAvoidBall = false;
			avoidBallCount = 0;
			if (isBallBehindMe)
			{
				ab_state = BALLBEHINDME;
			} else if (isBallBesideMe)
			{
				ab_state = BALLBESIDEME;
			}
		} else {
			setState(GETBALL);
			gb_state = LARGEANGLE;
			if (Verbose)
			{
				cout<<"-->GetBall";				
			}
		}
	} else if (DIRECTGOTO == getState())     //当前状态为DIRECTGOTO
	{
		if (RobotSensor::Instance()->IsInfraredOn(robotNum))
		{ //什么都不做，不跳转状态
		}
		else if (trueNeedAvoidBall)
		{
			setState(AVOIDBALL);
			if (Verbose)
			{
				cout<<"-->AvoidBall";
			}
			trueNeedAvoidBall = false;
			avoidBallCount = 0;
			if (isBallBehindMe)
			{
				ab_state = BALLBEHINDME;
			} else if (isBallBesideMe)
			{
				ab_state = BALLBESIDEME;
			}
		} else if (canNOTDirectGetBall)
		{
			setState(GETBALL);
			gb_state = LARGEANGLE;
			if (Verbose)
			{
				cout<<"-->GetBall";				
			}
		}		
	} else if (GETBALL == getState())        //当前状态为GETBALL
	{
		if (isCanDirectGetBall)
		{
			setState(DIRECTGOTO);
			if (Verbose)
			{
				cout<<"-->DirectGoto";
			}
		}else if (RobotSensor::Instance()->IsInfraredOn(robotNum))
		{ //什么都不做，不跳转状态
		}
		else if (trueNeedAvoidBall)
		{
			setState(AVOIDBALL);
			if (Verbose)
			{
				cout<<"-->AvoidBall";				
			}
			trueNeedAvoidBall = false;
			avoidBallCount = 0;
			if (isBallBehindMe)
			{
				ab_state = BALLBEHINDME;
			} else if (isBallBesideMe)
			{
				ab_state = BALLBESIDEME;
			}
		}
	} else if (AVOIDBALL == getState())      //当前状态为AVOIDBALL
	{
		if (avoidBallSuccess || isCanDirectGetBall)
		{
			setState(GETBALL);
			gb_state = LARGEANGLE;
			if (Verbose)
			{
				cout<<"-->GetBall";				
			}
			ab_state = NOAVOID;
		}
	}
	//状态查看
	if (Verbose)
	{
		if (DIRECTGOTO == getState())
		{
			GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(170, -150), "DirectGoto",COLOR_CYAN);
		} else if (GETBALL == getState())
		{
			GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(170, -150), "GetBall",COLOR_CYAN);
		} else if (AVOIDBALL == getState())
		{
			GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(170, -150), "AvoidBall",COLOR_CYAN);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	//状态执行模块
	int nowState = getState();
	switch (nowState)
	{
	case DIRECTGOTO:{
		getball_task.player.pos = ballPosWithVel + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + newVehicleBuffer + Param::Field::BALL_SIZE + StopDist - 2.5,reverse_finalDir); //  5.85
		break;}
	case GETBALL:{
		getball_task.player.pos = ball.Pos();
	/*	if (LARGEANGLE == gb_state)
		{
			if (fabs(dAngDiff_self2ball_finaldir) <= Param::Math::PI - extremeAngle + Param::Math::PI * 5 / 180)
			{
				gb_state = SMALLANGLE;
			}
		} else if (SMALLANGLE == gb_state )
		{
			if (fabs(dAngDiff_self2ball_finaldir) > Param::Math::PI * 3 / 180.0)
			{
				gb_state = LARGEANGLE;
			}
		}*/
		if(SMALLANGLE == gb_state)	{
			if (Verbose)
			{
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, -150), "small angle",COLOR_CYAN);
			}
			double getBallBuffer = -3 + 1 * me.Pos().dist(ballPosWithVel)/50;  //拿球时设计的余量-2 + 7 * me.Pos().dist(ballPosWithVel)/50;
			if (getBallBuffer > 2)
			{
				getBallBuffer = 2;
			}
			double getBallDist = Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + newVehicleBuffer + Param::Field::BALL_SIZE + getBallBuffer;
			if (getBallDist > me.Pos().dist(ballPosWithVel) )
			{
				getBallDist = Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + newVehicleBuffer + Param::Field::BALL_SIZE + StopDist - 2.5;
			}
			getball_task.player.pos = ballPosWithVel + Utils::Polar2Vector(getBallDist,reverse_finalDir); //该距离要小于directGetBallDist
		} else if(LARGEANGLE == gb_state)
		{
			if (Verbose)
			{
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, -150), "large angle",COLOR_CYAN);
			}
			//角度计算
			double theta_Dir = (pVision->OurPlayer(robotNum).RawPos() - pVision->Ball().Pos()).dir();//ball2self.dir();
			int sign = Utils::Normalize((theta_Dir - finalDir)) > 0 ? 1 : -1;			
			theta_Dir = Utils::Normalize(theta_Dir + sign * Param::Math::PI * 65 / 180.0); //Param::Math::PI * 75 / 180.0
			if (Verbose)
			{
				//GDebugEngine::Instance()->gui_debug_line(ballPosWithVel, ballPosWithVel+Utils::Polar2Vector(100,theta_Dir),COLOR_WHITE);
			}
			int sign2 = Utils::Normalize((theta_Dir - finalDir)) > 0 ? 1 : -1;		
			if (ball.Vel().mod() * sin(fabs(Utils::Normalize(ball.Vel().dir() - finalDir))) < transverseBallSpeed)//球速相对目标方向较小的时候
			{
				if (sign * sign2 < 0 || fabs(Utils::Normalize(theta_Dir - finalDir)) > extremeAngle)
				{
					theta_Dir = Utils::Normalize(finalDir + sign * extremeAngle);				
				}
			}	
			//距离计算
			double theta = fabs(Utils::Normalize(theta_Dir - finalDir));
			// 不同初始角度给的绕球距离不一样
			/*minGetBallDist = -0.04444444*turnAngleInBegin+34;
			if (minGetBallDist > 25) {
				minGetBallDist = 25;
			}
			if (minGetBallDist < 17) {
				minGetBallDist = 17;
			}
			maxGetBallDist = -0.04444444*turnAngleInBegin+34;
			if (maxGetBallDist > 30) {
				maxGetBallDist = 30;
			}
			if (maxGetBallDist < 26) {
				maxGetBallDist = 26;
			}*/

			directGetBallDist = minGetBallDist + 3;
			

			double getBallDist = minGetBallDist + (maxGetBallDist - minGetBallDist) * (1 + cos(theta));
			if (getBallDist > maxGetBallDist) {
				getBallDist = maxGetBallDist;
			}
			if (getBallDist < minGetBallDist) {
				if (theta < Param::Math::PI * 170 / 180.0) {
					getBallDist = minGetBallDist;
				}
			}
			//cout<<"getBallDist: "<<getBallDist<<" "<<theta<<endl;
			getball_task.player.pos = ballPosWithVel + Utils::Polar2Vector(getBallDist,theta_Dir);
		}		
		break;}
	case AVOIDBALL:{//TODO 加入球速影响，球在有速度的情况下修正躲避点
		if (BALLBEHINDME == ab_state)
		{
			double theta_Dir = ball2self.dir();
			double theta = Utils::Normalize(theta_Dir - finalDir);
			int sign = theta > 0 ? 1 : -1;
			theta_Dir = Utils::Normalize(theta_Dir + sign * Param::Math::PI * 60 / 180);
			getball_task.player.pos = ballPosWithVel + Utils::Polar2Vector(30,theta_Dir);
		} else if (BALLBESIDEME == ab_state)
		{
			double theta_Dir = reverse_finalDir;
			getball_task.player.pos = ballPosWithVel + Utils::Polar2Vector(30,theta_Dir);
		}		
		break;}
	default: break;
	}
	// 计算给定的朝向，比较远靠近时朝向先取车速的方向，之后再转
	double diffAngleVel2Final = fabs(dAngDiff_self2ball_finaldir);
	int sign = diffAngleVel2Final > Param::Math::PI / 2.0 ? 1 : 0;
	if( ball.Pos().dist(me.Pos()) > AllowFaceToFinalDist ) {
		getball_task.player.angle = Utils::Normalize(self2ball.dir() + sign*Param::Math::PI);
	}
	// 球在远处软件加速拿球,调试该参数要达到两点：1小车距离球远时有效加速 2不要冲的太猛碰开球
	//double extr_dist = 0.0;
	//if (self2ball.mod() > 200 ) {    
	//	extr_dist = self2ball.mod() * self2ball.mod() / 1000; 
	//	if (extr_dist > 160) {
	//		extr_dist = 160;
	//	}
	//	CGeoPoint fast_point = getball_task.player.pos + Utils::Polar2Vector(extr_dist,(getball_task.player.pos - me.Pos()).dir());
	//	if (! Utils::OutOfField(fast_point,0)) {
	//		getball_task.player.pos = fast_point;
	//	}		
	//}
	// 控球设定
	// 有设置踢球命令则停止控球 [8/19/2011 cliffyin]
	//if (KickStatus::Instance()->needKick(robotNum)) {
	//	DribbleStatus::Instance()->setDribbleCommand(robotNum,0);
	//} else {
	//	if (Allow_Start_Dribble && me.Pos().dist(ball.Pos()) < 50 && !(task().player.flag & PlayerStatus::NOT_DRIBBLE)) 
	//	{
	//		DribbleStatus::Instance()->setDribbleCommand(robotNum,DribbleLevel);
	//	}
	//}
	
	// 面版图形绘制
	if(Verbose) {
		//GDebugEngine::Instance()->gui_debug_x(ball.Pos(),COLOR_WHITE);
		//GDebugEngine::Instance()->gui_debug_x(ballPosWithVel,COLOR_BLACK);
		GDebugEngine::Instance()->gui_debug_x(getball_task.player.pos,COLOR_RED);
		GDebugEngine::Instance()->gui_debug_line(getball_task.player.pos,getball_task.player.pos + getball_task.player.vel,COLOR_RED);
	}
	// 调用底层控制
	getball_task.player.is_specify_ctrl_method = true;
	getball_task.player.specified_ctrl_method = mode;
	if( DIRECTGOTO == getState()) {
		setSubTask(TaskFactoryV2::Instance()->GotoPosition(getball_task));
	} else {
		setSubTask(TaskFactoryV2::Instance()->GotoPosition(getball_task));
	}

	_lastCycle = pVision->Cycle();
	CStatedTask::plan(pVision);
}

CPlayerCommand* CGoAndTurnKickV4::execute(const CVisionModule* pVision)
{
	if( subTask() ){
		return subTask()->execute(pVision);
	}

	return NULL;
}