#include <TouchKickPos.h>

#include <RobotCapability.h>
#include <CMmotion.h>
#include <ControlModel.h>
#include <cornell/Trajectory.h>
#include "Global.h"
#include <GDebugEngine.h>

namespace{
	///> 调试开关
	const bool GDebugEnable = true;
	
	///> 计算TouchKick站位点的适用条件参数
	const double DiffAngle_TPB_Max = Param::Math::PI/2.0;	// 踢球目标-车-球 夹角
	const double DiffAngle_TPBV_Max = Param::Math::PI/2.0;	// 踢球目标-车-球速方向 夹角
	const double BaseBallSpeed = 40;						// 75cm/s

	///> 避球参数
	const double AvoidBallAngle = Param::Math::PI*90/180;
}

///> 构造函数
CTouchKickPos::CTouchKickPos()
{
	_pass_set_cycle = 0;
	_ball_invalid_pass_dir = 0;
	reset();
}

///> 内部重置接口
void CTouchKickPos::reset()
{
	_kick_valid = false;
	_kick_pos = CGeoPoint(Param::Field::PITCH_LENGTH/2.0,0.0);
}

///> 对外的调用接口
void CTouchKickPos::GenerateTouchKickPos(const CVisionModule *pVision, const int player, const double kickdir)
{
	///> 1.进行重置
	reset();

	///> 2.图像信息
	const PlayerVisionT &kicker	= pVision->OurPlayer(player);
	const MobileVisionT &ball	= pVision->Ball();

	CVector self2ball			= ball.Pos() - kicker.Pos();
	double ballVelDir			= ball.Vel().dir();									// 球速方向
	double ballVelReverse		= Utils::Normalize(ballVelDir+Param::Math::PI);		// 球速反向


	///> 3.判断是否需要执行 : 球速较大 & 踢球方向-车球方向夹角 & 踢球方向-球速反方向夹角
	_kick_valid = ball.Vel().mod() > BaseBallSpeed
				&& fabs(Utils::Normalize(kickdir-self2ball.dir())) < DiffAngle_TPB_Max 
				&& fabs(Utils::Normalize(kickdir-ballVelReverse)) < DiffAngle_TPBV_Max ;

	///> 4.调用内部函数生成相应的站位点
	if (_kick_valid) {
		GenerateValidTouchKickPos(pVision, player, kickdir);
	} else {
		GenerateInvalidTouchKickPos(pVision, player, kickdir);
	}

	///> 5.是否调试
	if (GDebugEnable) {
		GDebugEngine::Instance()->gui_debug_x(_kick_pos,COLOR_RED);
	}
}

///> 内部调用接口
void CTouchKickPos::GenerateValidTouchKickPos(const CVisionModule *pVision , const int player, const double kickdir)
{
	/************************************************************************/
	/* 1.根据图像模块中的信息准备计算                                         */
	/************************************************************************/

	GDebugEngine::Instance()->gui_debug_line(pVision->OurPlayer(player).Pos(), pVision->OurPlayer(player).Pos()+Utils::Polar2Vector(1000,pVision->OurPlayer(player).Dir()));
	///> 图像信息
	const PlayerVisionT &kicker  = pVision->OurPlayer(player);
	const MobileVisionT &ball = pVision->Ball();
	CGeoPoint kickerHeaderPos = kicker.Pos() + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER,kicker.Dir());

	CVector self2ball = ball.Pos() - kicker.Pos();
	double ballVelDir = ball.Vel().dir();									// 球速方向
	double ballVelReverse = Utils::Normalize(ballVelDir+Param::Math::PI);	// 球速反向
	double kickDirReverse = Utils::Normalize(kickdir+Param::Math::PI);		// 目标方向反向
	
	// 球速直线
	CGeoLine ballMovingLine = CGeoLine(ball.Pos(),ball.Pos()+ball.Vel());
	// 踢球直线
	CGeoLine kickDirLine = CGeoLine(kicker.Pos(),kicker.Pos()+Utils::Polar2Vector(50,kickdir));

	kickDirLine = CGeoLine(kicker.Pos(),kicker.Pos()+Utils::Polar2Vector(50,kickdir));
	double intersectangle = std::abs(Utils::Normalize(kickdir+Param::Math::PI/2) - ballVelReverse);
	double intersectangle2 = std::abs(kickdir - ballVelReverse);
	if (intersectangle > Param::Math::PI/2){
		intersectangle = Param::Math::PI - intersectangle;
	}
	if (intersectangle2 > Param::Math::PI/2){
		intersectangle2 = Param::Math::PI - intersectangle2;
	}
	//cout<<intersectangle*180/Param::Math::PI<<"  "<<intersectangle2*180/Param::Math::PI<<endl;
	if (intersectangle>=intersectangle2){
		//cout<<"1111111111111"<<endl;
		CGeoLineLineIntersection intersect = CGeoLineLineIntersection(ballMovingLine,CGeoLine(kicker.Pos()+Utils::Polar2Vector(Param::Vehicle::V2::TOUCH_SHIFT_DIST,kickdir),Utils::Normalize(kickdir+Param::Math::PI/2)));
		if(intersect.Intersectant() ){
			_kick_pos = intersect.IntersectPoint();
		}else{
			_kick_pos = ballMovingLine.projection(_kick_pos);
		}
		_kick_pos = _kick_pos+ Utils::Polar2Vector(Param::Vehicle::V2::TOUCH_SHIFT_DIST,kickDirReverse);
		GDebugEngine::Instance()->gui_debug_line(ball.Pos(),ball.Pos() + Utils::Polar2Vector(500,ball.Vel().dir()),COLOR_WHITE);
		GDebugEngine::Instance()->gui_debug_x(_kick_pos,COLOR_WHITE);
		GDebugEngine::Instance()->gui_debug_line(_kick_pos,_kick_pos+Utils::Polar2Vector(500,Utils::Normalize(kickdir+Param::Math::PI/2)),COLOR_WHITE);
	}else{
		//cout<<"2222222222222"<<endl;
		CGeoLineLineIntersection intersect = CGeoLineLineIntersection(ballMovingLine,kickDirLine);
		if(intersect.Intersectant() ){
			_kick_pos = intersect.IntersectPoint();
		}else{
			_kick_pos = ballMovingLine.projection(_kick_pos);
		}
		_kick_pos = _kick_pos+ Utils::Polar2Vector(Param::Vehicle::V2::TOUCH_SHIFT_DIST,kickDirReverse);
		GDebugEngine::Instance()->gui_debug_line(ball.Pos(),ball.Pos() + Utils::Polar2Vector(500,ball.Vel().dir()),COLOR_WHITE);
		GDebugEngine::Instance()->gui_debug_x(_kick_pos,COLOR_WHITE);
		GDebugEngine::Instance()->gui_debug_line(_kick_pos,_kick_pos+Utils::Polar2Vector(500,kickdir),COLOR_WHITE);
	}
	//cout<<vision->Cycle()<<" "<<rawdir<<" "<<shiftdist<<" "<<_kick_pos<<endl;
}

///> 内部调用接口
void CTouchKickPos::GenerateInvalidTouchKickPos(const CVisionModule *pVision , const int player, const double kickdir)
{
	/************************************************************************/
	/*   效果非常好，推广 [4/30/2011 cliffyin]                               */
	/************************************************************************/

	///> 图像信息
	const PlayerVisionT &kicker	= pVision->OurPlayer(player);
	const MobileVisionT &ball	= pVision->Ball();

	CVector self2ball		= ball.Pos() - kicker.Pos();
	double ballVelDir		= ball.Vel().dir();									// 球速方向
	double ballVelReverse	= Utils::Normalize(ballVelDir+Param::Math::PI);		// 球速反向
	double ballSpeed		= ball.Vel().mod();

//////////////////////////////////////////////////////////////////////////

	const CGeoPoint myhead = kicker.Pos()+Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + 0.6,kicker.Dir());
	const CVector ball2self = kicker.Pos() - ball.Pos();
	const CVector head2ball = ball.Pos() - myhead;
	// 对小球的位置做预测
	double dAngle_ball2myhead_ballvel = Utils::Normalize(Utils::Normalize(head2ball.dir()+Param::Math::PI)-ball.Vel().dir());
	double dAngle_self2ball_medir = Utils::Normalize(self2ball.dir()-kicker.Dir());
	double BallPosWithVelFactorTmp = 0.2;
	// 球速过大，背向拿球，加大预测
	if (ball.Vel().mod() > 120) {
		BallPosWithVelFactorTmp += 0.2;
	}
	// 球在车正前方对球预测时间的影响，迎面过来预测时间减小，追球时为加快追赶速度增大预测时间
	if (fabs(dAngle_self2ball_medir) < Param::Vehicle::V2::KICK_ANGLE) {
		if (fabs(dAngle_ball2myhead_ballvel) < Param::Math::PI/6.0) {                
			BallPosWithVelFactorTmp *= sin(fabs(dAngle_ball2myhead_ballvel));
		} else if (fabs(dAngle_ball2myhead_ballvel) > Param::Math::PI*5/6.0)
		{
			BallPosWithVelFactorTmp += 0.7;
			BallPosWithVelFactorTmp *= cos(fabs(dAngle_ball2myhead_ballvel)) * -1;
		}
	}

	CGeoPoint ballPosWithVel = ball.Pos() + ball.Vel() * BallPosWithVelFactorTmp;
	_kick_pos = ballPosWithVel;

	///> 对于跑位点的特殊处理方式
	double dAngleForKickDir = fabs(Utils::Normalize(kickdir-self2ball.dir()));
	double extra_buffer = 2.0;
	// 球在正前方判断 : 含缓冲
	bool is_ball_just_front = fabs(Utils::Normalize(self2ball.dir()-kicker.Dir())) < 0.75*Param::Vehicle::V2::KICK_ANGLE
								&& fabs(Utils::Normalize(kickdir-kicker.Dir())) < 0.5*Param::Vehicle::V2::KICK_ANGLE
								&& self2ball.mod() < 4.5*Param::Vehicle::V2::PLAYER_SIZE;
	// TODO
	static int ball_just_front_count = 0;
	static int last_cycle = 0;
	if(pVision->Cycle() - last_cycle > 0.1*Param::Vision::FRAME_RATE) {
		ball_just_front_count = 0;
	}
	last_cycle = pVision->Cycle();
	if (is_ball_just_front) {
		ball_just_front_count ++;
	} else {
		ball_just_front_count /= 2;
	}
	if (ball_just_front_count>=0.2*Param::Vision::FRAME_RATE) {
		ball_just_front_count = 0.2*Param::Vision::FRAME_RATE;
		is_ball_just_front = true;
	} else {
		is_ball_just_front = false;
	}
	// 球在正后方判断 ： 不含缓冲
	bool is_ball_just_back = kicker.Pos().dist(ball.Pos()) <= Param::Field::BALL_SIZE + 1.75*Param::Vehicle::V2::PLAYER_SIZE
								&& fabs(Utils::Normalize(self2ball.dir() - kickdir)) > Param::Math::PI*3/4;

	// 球在正前方，且朝向目标方向
	if (is_ball_just_front) {	
		GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(170, -150), "BALL_FRONT",COLOR_CYAN);
		extra_buffer = - Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER*cos(dAngleForKickDir)*self2ball.mod()/(1.5*Param::Vehicle::V2::PLAYER_SIZE) - 2;
	} else {
		extra_buffer = max(5.0 - 2*cos(dAngleForKickDir),3.0);
		extra_buffer = min(extra_buffer,1.5);
	}

	// 初步计算跑位点
	double kickDirReverse = Utils::Normalize(kickdir + Param::Math::PI);		// 目标方向反向
	_kick_pos = _kick_pos + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER+extra_buffer,kickDirReverse);

	
	///> 特殊情况：避球处理
	double dir_kickpos_2_ballpos = (ball.Pos()-_kick_pos).dir();
	double dir_kickpos_2_kickerpos = (kicker.Pos()-_kick_pos).dir();
	CGeoSegment seg_ballpos_2_kickpos = CGeoSegment(ball.Pos(),_kick_pos);
	CGeoSegment seg_kickerpos_2_kickpos = CGeoSegment(kicker.Pos(),_kick_pos);
	CGeoPoint proj_me_on_seg = seg_ballpos_2_kickpos.projection(kicker.Pos());
	CGeoPoint proj_ball_on_seg = seg_kickerpos_2_kickpos.projection(ball.Pos());

	// kicker的投影点位于线段上
	bool is_safe = true;
	double dist_projball_2_ball = proj_ball_on_seg.dist(ball.Pos());
	double dAngleTmp = fabs(Utils::Normalize(dir_kickpos_2_ballpos - dir_kickpos_2_kickerpos));

	double dAngleTmp2 = fabs(Utils::Normalize(CVector(ball.Pos() - kicker.Pos()).dir() - CVector(_kick_pos - kicker.Pos()).dir()));

	// 以下认为不安全
	if ( (dist_projball_2_ball < Param::Field::BALL_SIZE + 1.25*Param::Vehicle::V2::PLAYER_SIZE
		&& dAngleTmp < AvoidBallAngle && !is_ball_just_front && dAngleTmp2 < Param::Math::PI / 2.0)
		|| is_ball_just_back ) {	// 球在正后方
		is_safe = false;
	}
	
	// 躲避球
	if (!is_safe) {
		CGeoPoint _kick_pos_pre = _kick_pos;
		double nowdir = Utils::Normalize(self2ball.dir()+Param::Math::PI);
		int sign = Utils::Normalize((nowdir - kickdir))>0?1:-1;
		// 角度正相关于 dAngleTmp
		double offset_angle = dAngleTmp*1.25/2.0 + Param::Math::PI*1.0/2.0;	
		if (is_ball_just_back) {	// 球在正后方
			offset_angle = Param::Math::PI*1.0/4.0 * (1.0 + (self2ball.mod()/Param::Vehicle::V2::PLAYER_SIZE/4.0));
		}
		nowdir = Utils::Normalize(kicker.Dir()+sign*offset_angle);

		// 距离正相关于 dAngleTmp
		double offset_dist = Param::Field::BALL_SIZE + 1.25*Param::Vehicle::V2::PLAYER_SIZE;
		if (is_ball_just_back) {	// 球在正后方
			offset_dist += 1.0*Param::Vehicle::V2::PLAYER_SIZE;
		}
		offset_dist = offset_dist + 1.0*Param::Vehicle::V2::PLAYER_SIZE*cos(dAngleTmp);

		_kick_pos = ballPosWithVel + Utils::Polar2Vector(offset_dist,nowdir);
		_kick_pos = _kick_pos + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + 2,kickDirReverse);

		// 和之前的球综合权衡
		double alpha = dist_projball_2_ball/(Param::Field::BALL_SIZE + 1.50*Param::Vehicle::V2::PLAYER_SIZE)+0.5;
		if (alpha>1.0) {
			alpha = 1.0;
		} else if (is_ball_just_back) {	// 球在正后方
			alpha = 0.9;
		}
		_kick_pos = _kick_pos_pre + (_kick_pos-_kick_pos_pre) * alpha;
	}
	if (!ball.Valid() && pVision->Cycle() - _pass_set_cycle < 60) {
		cout<<"Ball InValid In TouchKick"<<endl;
		CGeoLine ballMovingLine = CGeoLine(ball.Pos(), _ball_invalid_pass_dir);
		CGeoLine kickDirLine = CGeoLine(kicker.Pos(),kicker.Pos()+Utils::Polar2Vector(50,kickdir));
		double intersectangle = std::abs(Utils::Normalize(pVision->OurPlayer(player).Dir()+Param::Math::PI/2) - Utils::Normalize(ball.Vel().dir()+Param::Math::PI));
		double intersectangle2 = std::abs(kickdir - Utils::Normalize(ball.Vel().dir()+Param::Math::PI));
		if (intersectangle > Param::Math::PI/2){
			intersectangle = Param::Math::PI - intersectangle;
		}
		if (intersectangle2 > Param::Math::PI/2){
			intersectangle2 = Param::Math::PI - intersectangle2;
		}
		if (intersectangle>=intersectangle2){
			CGeoLineLineIntersection intersect = CGeoLineLineIntersection(ballMovingLine,CGeoLine(pVision->OurPlayer(player).Pos()+Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER+2.25,kickdir),Utils::Normalize(kickdir+Param::Math::PI/2)));
			if(intersect.Intersectant() ){
				_kick_pos = intersect.IntersectPoint();
			}else{
				_kick_pos = ballMovingLine.projection(_kick_pos);
			}
			_kick_pos = _kick_pos+ Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER+2.15,kickDirReverse);
		}else{
			CGeoLineLineIntersection intersect = CGeoLineLineIntersection(ballMovingLine,kickDirLine);
			if(intersect.Intersectant() ){
				_kick_pos = intersect.IntersectPoint();
			}else{
				_kick_pos = ballMovingLine.projection(_kick_pos);
			}
			_kick_pos = _kick_pos+ Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER+2.15,kickDirReverse);
		}
	}
}
		