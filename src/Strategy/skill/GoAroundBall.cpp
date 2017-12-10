#include "GoAroundBall.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include <utils.h>
#include <DribbleStatus.h>
#include "BallSpeedModel.h"

namespace
{
	//配置状态机
	enum mystate
	{
		isFar,
		isCanDirectGoto,
		isBallJustBehindme,
		isFirstAngle,
		isSecondAngle
	};
	mystate _mystate = isFar;
	//1.调试面板作图开关
	bool Debug_Draw = true;
	//2.走位相关参数
	double MiddlePointVelFactor = 0.25;
	double BallPosWithVelFactor = 0.25;		// 预测球位置 0.1s
	const double DeepDist = 9;
	const double distFactor = 2.0; 
	const double AvoidOverShootDist_For_SmallAngle = 3*Param::Vehicle::V2::PLAYER_SIZE;		//36
	const double AvoidOverShootOffsize_For_SmallAngle = Param::Vehicle::V2::PLAYER_SIZE;	
	const double AvoidOverShootDist_For_BigAngle = 5*Param::Vehicle::V2::PLAYER_SIZE;		//54
	const double AvoidOverShootOffsize_For_BigAngle = Param::Vehicle::V2::PLAYER_SIZE*2;

	const double AllowFaceToFinalDist = 200;	//距离球150cm以内开始转向，否则面向球走，x方向执行效率高
	const double AllowFaceToFinalAngle = Param::Math::PI/2.0;
	const double ballVelFactor2time = 120;
}

CGoAroundBall::CGoAroundBall()
{
	_lastCycle = 0;
}

void CGoAroundBall::plan(const CVisionModule* pVision)
{
	// 内部状态进行重置
	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1 ){
		cout<<"get into CGoAroundBall !"<<endl;
		_mystate = isFar;
	}

	// 小车相关图像信息获取
	const MobileVisionT& ball = pVision->Ball();
	const int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const CVector self2ball = ball.Pos() - me.Pos();
	const double StopDist = task().player.rotvel;

	// 对小球的位置做点预测
	double dAngle_ball2self_ballvel = Utils::Normalize(Utils::Normalize(self2ball.dir()+Param::Math::PI)-ball.Vel().dir());
	double dAngle_self2ball_medir = Utils::Normalize(self2ball.dir()-me.Dir());
	double BallPosWithVelFactorTmp = BallPosWithVelFactor;
	if (fabs(dAngle_ball2self_ballvel) < Param::Math::PI/2.0) {
		BallPosWithVelFactorTmp *= sin(fabs(dAngle_ball2self_ballvel));
		BallPosWithVelFactorTmp += 0.1;
	}
	if (fabs(dAngle_self2ball_medir) < Param::Vehicle::V2::KICK_ANGLE) {
		BallPosWithVelFactorTmp *= sin(fabs(dAngle_self2ball_medir));
		BallPosWithVelFactorTmp += 0.1;
	}

	static CGeoPoint ballPosWithVel = BallSpeedModel::Instance()->posForTime((int)(BallPosWithVelFactorTmp*60),pVision);//计算若干帧以后的绝对位置
	if (ball.Valid()) {
		ballPosWithVel = BallSpeedModel::Instance()->posForTime((int)(BallPosWithVelFactorTmp*60),pVision);
	}
	GDebugEngine::Instance()->gui_debug_x(ballPosWithVel,COLOR_RED);
	// 计算相关变量赋值
	double finalDir = task().player.angle/*(task().player.pos - ballPosWithVel).dir()*/;
	double reverse_finalDir = Utils::Normalize(finalDir+Param::Math::PI);
	double dAngDiff = Utils::Normalize(self2ball.dir()-finalDir);
	CGeoLine ball_finalDir_line = CGeoLine(ball.Pos(),ball.Pos()+Utils::Polar2Vector(50,Utils::Normalize(finalDir)));
	CGeoPoint meProjPoint = ball_finalDir_line.projection(me.Pos());

	TaskT getball_task(task());
	getball_task.player.angle = finalDir;
	getball_task.player.rotvel = 0.0;

	getball_task.player.flag = getball_task.player.flag;	//设置拿球标签
	//设置避开我方禁区标签 [6/27/2011 zhanfei]
	getball_task.player.flag = getball_task.player.flag | PlayerStatus::DODGE_OUR_DEFENSE_BOX;//躲禁区
	getball_task.player.flag = getball_task.player.flag /*& (~PlayerStatus::DODGE_BALL)*/;		//取消避球标签
	if( !(getball_task.player.flag & PlayerStatus::DRIBBLING) )
		getball_task.player.flag = getball_task.player.flag & (~PlayerStatus::DRIBBLING);	//取消控球标签

	if( me.Pos().dist(ball.Pos()) <= 2*Param::Vehicle::V2::PLAYER_SIZE+Param::Field::BALL_SIZE ) { //
		if(
			isFar == _mystate && fabs(Utils::Normalize(self2ball.dir() - finalDir)) < Param::Math::PI*1/6.0   ||
			isFirstAngle == _mystate && fabs(Utils::Normalize(self2ball.dir() - finalDir)) < Param::Math::PI*0.9/6.0)
		{
			_mystate = isCanDirectGoto;
			GDebugEngine::Instance()->gui_debug_msg(
				CGeoPoint(Param::Field::PITCH_LENGTH / 2 + 20, -Param::Field::PITCH_WIDTH / 2), "CanDirectGoto",COLOR_CYAN);
		}
		else if (
			isFar == _mystate && fabs(Utils::Normalize(self2ball.dir() - finalDir)) >= Param::Math::PI*2/4.0 &&
			fabs(Utils::Normalize(self2ball.dir() - finalDir)) < Param::Math::PI*3/4.0  ||
			isFirstAngle == _mystate && fabs(Utils::Normalize(self2ball.dir() - finalDir)) >= Param::Math::PI*1.1*2/4.0   ||
			isBallJustBehindme == _mystate && fabs(Utils::Normalize(self2ball.dir() - finalDir)) < Param::Math::PI*0.9*3/4.0)
		{
			_mystate = isSecondAngle;
			GDebugEngine::Instance()->gui_debug_msg(
				CGeoPoint(Param::Field::PITCH_LENGTH / 2 + 20, -Param::Field::PITCH_WIDTH / 2), "isSecondAngle",COLOR_CYAN);
		} else if (
			isFar == _mystate && fabs(Utils::Normalize(self2ball.dir() - finalDir)) >= Param::Math::PI*1/6.0 &&
			fabs(Utils::Normalize(self2ball.dir() - finalDir)) < Param::Math::PI*2/4.0   ||
			isCanDirectGoto == _mystate && fabs(Utils::Normalize(self2ball.dir() - finalDir)) >= Param::Math::PI*1.1/6.0   ||
			isSecondAngle == _mystate && fabs(Utils::Normalize(self2ball.dir() - finalDir)) < Param::Math::PI*0.9*2/4.0)
		{
			_mystate = isFirstAngle;
			GDebugEngine::Instance()->gui_debug_msg(
				CGeoPoint(Param::Field::PITCH_LENGTH / 2 + 20, -Param::Field::PITCH_WIDTH / 2), "isFirstAngle",COLOR_CYAN);
		}

		else if(
			isFar == _mystate && fabs(Utils::Normalize(self2ball.dir() - finalDir)) >= Param::Math::PI*3/4.0   ||
			isSecondAngle == _mystate && fabs(Utils::Normalize(self2ball.dir() - finalDir)) >= Param::Math::PI*1.1*3/4.0) 
		{
			_mystate = isBallJustBehindme;
			GDebugEngine::Instance()->gui_debug_msg(
				CGeoPoint(Param::Field::PITCH_LENGTH / 2 + 20, -Param::Field::PITCH_WIDTH / 2), "isBallJustBehindme",COLOR_CYAN);
		}	
	} else _mystate = isFar;
	//走位规划
	if (isCanDirectGoto == _mystate)
	{
		getball_task.player.pos = ballPosWithVel + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + Param::Field::BALL_SIZE + StopDist,reverse_finalDir);
	} 
	else if (isFirstAngle == _mystate)
	{
		double nowdir = Utils::Normalize(self2ball.dir()+Param::Math::PI);
		int sign = Utils::Normalize((nowdir - finalDir))>0?1:-1;
		nowdir = Utils::Normalize(me.Dir() - sign*Param::Math::PI*9/10);
		getball_task.player.pos = ballPosWithVel + Utils::Polar2Vector(Param::Field::BALL_SIZE + Param::Field::MAX_PLAYER_SIZE + StopDist - me.Pos().dist(ball.Pos()),nowdir); //20.15 + 1
	}
	else if (isSecondAngle == _mystate)
	{
		double nowdir = Utils::Normalize(self2ball.dir()+Param::Math::PI);
		int sign = Utils::Normalize((nowdir - finalDir))>0?1:-1;
		nowdir = Utils::Normalize(me.Dir() - sign*Param::Math::PI*8/10);
		getball_task.player.pos = ballPosWithVel + Utils::Polar2Vector(Param::Field::BALL_SIZE + Param::Field::MAX_PLAYER_SIZE * 1.2 + StopDist * 1.2 - me.Pos().dist(ball.Pos()),nowdir);
	} 
	else if (isBallJustBehindme == _mystate)
	{
		double nowdir = Utils::Normalize(self2ball.dir()+Param::Math::PI);
		int sign = Utils::Normalize((nowdir - finalDir))>0?1:-1;
		nowdir = Utils::Normalize(me.Dir() - sign*Param::Math::PI*7/10);
		getball_task.player.pos = ballPosWithVel + Utils::Polar2Vector(Param::Field::BALL_SIZE + Param::Field::MAX_PLAYER_SIZE * 1.2 + StopDist * 1.5 - me.Pos().dist(ball.Pos()),nowdir);

	}
	else{
		getball_task.player.pos = ball.Pos();
		if( fabs(dAngDiff) <= Param::Math::PI/6.0 )	{
			getball_task.player.pos = ballPosWithVel + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER+Param::Field::BALL_SIZE+1.5,reverse_finalDir);
		} else if ( fabs(dAngDiff) <= Param::Math::PI/3.0 ) {
			getball_task.player.pos = ballPosWithVel + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_SIZE+Param::Field::BALL_SIZE+DeepDist,reverse_finalDir);

			//相对水平距离过大，偏移一个半径，防超调
			if( meProjPoint.dist(me.Pos()) > AvoidOverShootDist_For_SmallAngle ) {
				getball_task.player.pos = getball_task.player.pos + Utils::Polar2Vector(AvoidOverShootOffsize_For_SmallAngle,Utils::Normalize((me.Pos()-meProjPoint).dir()));
			}
		} else if ( fabs(dAngDiff) <= Param::Math::PI/2.0 ) {
			getball_task.player.pos = ballPosWithVel + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_SIZE*1.5+Param::Field::BALL_SIZE+DeepDist,reverse_finalDir);
			//相对水平距离过大，偏移一个半径，防超调
			if( meProjPoint.dist(me.Pos()) > AvoidOverShootDist_For_BigAngle
				|| (fabs(dAngDiff) >= Param::Math::PI/2.25 && self2ball.mod() > 5*Param::Vehicle::V2::PLAYER_SIZE) ) {
					getball_task.player.pos = getball_task.player.pos + Utils::Polar2Vector(AvoidOverShootOffsize_For_BigAngle,Utils::Normalize((me.Pos()-meProjPoint).dir()));
			}
		} else {
			double nowdir = Utils::Normalize(self2ball.dir()+Param::Math::PI);
			int sign = Utils::Normalize((nowdir - finalDir))>0?1:-1;
			nowdir = Utils::Normalize(nowdir+sign*Param::Math::PI/2.0);

			getball_task.player.pos = ballPosWithVel + Utils::Polar2Vector(Param::Field::MAX_PLAYER_SIZE*1.5,nowdir);
			getball_task.player.vel = Utils::Polar2Vector(task().player.vel.mod()*MiddlePointVelFactor,self2ball.dir());
		}
		getball_task.player.angle = (ball.Pos()-me.Pos()).dir();
	}

	//护球时无视敌人卡位
	getball_task.player.flag = getball_task.player.flag | PlayerStatus::IGNORE_PLAYER_CLOSE_TO_TARGET;

	// 设定控球
	if( 0/*me.Pos().dist(ball.Pos()) <= 2*Param::Vehicle::V2::PLAYER_SIZE+Param::Field::BALL_SIZE
		 && fabs(Utils::Normalize(self2ball.dir() - me.Dir())) < Param::Math::PI/6*/  ) {
			 unsigned char set_power = DribbleStatus::Instance()->getDribbleCommand(robotNum);
			 if ( set_power <= 0 ) {
				 DribbleStatus::Instance()->setDribbleCommand(robotNum,0);
			 }
			 getball_task.player.flag = getball_task.player.flag | PlayerStatus::DRIBBLING;
	}
	//底层路径规划选择

	// 面包调试输出
	if( Debug_Draw ) {
		GDebugEngine::Instance()->gui_debug_x(getball_task.player.pos,COLOR_RED);
		GDebugEngine::Instance()->gui_debug_line(getball_task.player.pos,getball_task.player.pos+getball_task.player.vel,COLOR_RED);
	}

	// 调用底层控制
	//if (fabs(Utils::Normalize((ball.Pos()-me.Pos()).dir() - finalDir)) < Param::Math::PI*4/180 && ball.Pos().dist(me.Pos()) < 13
	//	&& fabs(Utils::Normalize(me.Dir() - finalDir)) < Param::Math::PI*4/180)
	//{
	//	setSubTask(PlayerRole::makeItStop(robotNum));
	////	setSubTask(PlayerRole::makeItSimpleGoto(robotNum, ball.Pos(), finalDir,Utils::Polar2Vector(20, finalDir),0));
	//	//cout<<"I am stopped"<<endl;
	//}else
	if( fabs(dAngDiff) > Param::Math::PI / 2){
		getball_task.player.angle = finalDir;
		setSubTask(TaskFactoryV2::Instance()->NoneTrajGetBall(getball_task));
	} else{
		if( ball.Pos().dist(me.Pos()) < 50) {
			setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(getball_task));
		} else {
			setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(getball_task));
		}
	}

	//cout<<getball_task.player.pos<<"           "<<getball_task.player.angle<<endl;
	_lastCycle = pVision->Cycle();
	CStatedTask::plan(pVision);
}

CPlayerCommand* CGoAroundBall::execute(const CVisionModule* pVision)
{
	if( subTask() ){
		return subTask()->execute(pVision);
	}

	return NULL;
}
