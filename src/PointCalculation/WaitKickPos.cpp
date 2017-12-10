#include <WaitKickPos.h>
#include "utils.h"
#include "Global.h"
#include <GDebugEngine.h>

namespace{
	CGeoPoint cur_point[7];
	CGeoPoint last_point[7];
	int last_cycle[7];
	bool need_reset[7];
	vector<CGeoPoint> move_ball;
}

///> 构造函数
CWaitKickPos::CWaitKickPos(){
	reset();
}

///> 内部重置接口
void CWaitKickPos::reset(){
	for(int i = 0; i <= Param::Field::MAX_PLAYER; i++){
		cur_point[i] = CGeoPoint(Param::Field::PITCH_LENGTH/2.0,0.0);
		last_point[i] = cur_point[i];
		last_cycle[i] = 0;
		need_reset[i] = true;
	}
}

///> 对外的调用接口
void CWaitKickPos::GenerateWaitKickPos(const CGeoPoint pointT, const double angle ,const int player,const double kickdir)
{
	///> 2.图像信息
	CVisionModule* pVision = vision;
	const MobileVisionT &ball	= pVision->Ball();
	double ballVelDir = ball.Vel().dir();									// 球速方向
	double kickDirReverse = Utils::Normalize(kickdir+Param::Math::PI);		// 目标方向反向
	CGeoLine pointLine = CGeoLine(pointT,pointT+Utils::Polar2Vector(500,angle));
	CGeoSegment pointSegment = CGeoSegment(pointT+Utils::Polar2Vector(50,angle), pointT+Utils::Polar2Vector(-50,angle));
	CGeoLine ballLine = CGeoLine(ball.Pos(),ball.Pos()+Utils::Polar2Vector(100,ballVelDir));
	CGeoLineLineIntersection intersect = CGeoLineLineIntersection(pointLine,ballLine);
	
	if (intersect.Intersectant()){
		cur_point[player] = intersect.IntersectPoint();
		if (Utils::OutOfField(cur_point[player], 0) || !pointSegment.IsPointOnLineOnSegment(cur_point[player]) || Utils::InTheirPenaltyArea(cur_point[player],5)){
			cur_point[player] = last_point[player];
			return ;
		}
	}else{
		cur_point[player] = last_point[player];
		return ;
	}

	cur_point[player] = cur_point[player]+ Utils::Polar2Vector(Param::Vehicle::V2::TOUCH_SHIFT_DIST,kickDirReverse);
	last_point[player] = cur_point[player];
}

void CWaitKickPos::GenerateWaitKickPos(const CGeoPoint pointA,const CGeoPoint pointB,const int player,const double kickdir)
{
	if(vision->Cycle() - last_cycle[player] > 6){
		cout<<"reset!!"<<vision->Cycle()<<" "<<last_cycle[player]<<endl;
		need_reset[player] = true;
		move_ball.clear();
	}
	if(need_reset[player]){
		last_point[player] = CGeoPoint((pointA.x()+pointB.x())/2.0,(pointA.y()+pointB.y())/2.0);
	}
	last_cycle[player] = vision->Cycle();
	CVisionModule* pVision = vision;
	const MobileVisionT &ball	= pVision->Ball();
	double ballVelDir = ball.Vel().dir();									// 球速方向
	double kickDirReverse = Utils::Normalize(kickdir+Param::Math::PI);		// 目标方向反向
	CGeoLine pointLine = CGeoLine(pointA,pointB);
	CGeoSegment pointSegment = CGeoSegment(pointA,pointB);
	CGeoLine ballLine = CGeoLine(ball.Pos(),ball.Pos()+Utils::Polar2Vector(100,ballVelDir));
	CGeoLineLineIntersection intersect = CGeoLineLineIntersection(pointLine,ballLine);

	
	double ball2playerdir = (pVision->OurPlayer(player).Pos() - ball.Pos()).dir();
	double diff_ball2playerdir_ballveldir = fabs(Utils::Normalize(ballVelDir - ball2playerdir));
	double ball2playerdist = (pVision->OurPlayer(player).Pos() - ball.Pos()).mod();
	
	if(ball2playerdist < 250){	
		if(intersect.Intersectant()){
			CGeoPoint interP = intersect.IntersectPoint();
			if(pointSegment.IsPointOnLineOnSegment(interP)){
				cur_point[player] = interP;
				need_reset[player] = false;
			} else{
				cur_point[player] = CGeoPoint((pointA.x()+pointB.x())/2.0,(pointA.y()+pointB.y())/2.0);
			}
		} else if(need_reset[player]){
			cur_point[player] = CGeoPoint((pointA.x()+pointB.x())/2.0,(pointA.y()+pointB.y())/2.0);
		} else{
			cur_point[player] = CGeoPoint((pointA.x()+pointB.x())/2.0,(pointA.y()+pointB.y())/2.0);
		}
	} else{
		cur_point[player] = CGeoPoint((pointA.x()+pointB.x())/2.0,(pointA.y()+pointB.y())/2.0);
	}
	
	cur_point[player] = cur_point[player] + Utils::Polar2Vector(Param::Vehicle::V2::TOUCH_SHIFT_DIST,kickDirReverse);
	last_point[player] = cur_point[player];
}

CGeoPoint CWaitKickPos::getKickPos(int player)
{
	return cur_point[player];
}