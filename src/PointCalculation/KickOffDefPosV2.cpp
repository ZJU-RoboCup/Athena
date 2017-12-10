#include "KickOffDefPosV2.h"
#include <GDebugEngine.h>
#include <vector>

namespace
{
	const double DefDist = - Param::Vehicle::V2::PLAYER_SIZE;
	CGeoLine def_line;
	double DefY[3];
}
CKickOffDefPosV2::CKickOffDefPosV2()
{
	VERBOSE_MODE = false;
	def_line = CGeoLine(CGeoPoint(DefDist, -Param::Field::PITCH_WIDTH / 2)
		, CGeoPoint(DefDist, Param::Field::PITCH_WIDTH / 2));
	DefY[0]= 0;
	DefY[1]= - Param::Field::CENTER_CIRCLE_R;
	DefY[2]= Param::Field::CENTER_CIRCLE_R;
} 
CGeoPoint CKickOffDefPosV2::GetKickOffDefPos(const CVisionModule *pVision, const int pos_num)
{
	const CGeoPoint goal_center(- Param::Field::PITCH_LENGTH / 2, 0);
	double dist_min = Param::Field::PITCH_LENGTH;
	int enemy_min = 0;
	for (int i = 1; i <= Param::Field::MAX_PLAYER; i ++)
	{
		if (pVision->TheirPlayer(i).Valid() == true && pVision->TheirPlayer(i).Pos().x() < 200*Param::Field::RATIO
			&& this->GetPosNum(pVision, pVision->TheirPlayer(i).Pos()) == pos_num)
		{
			if (pVision->TheirPlayer(i).Pos().dist(goal_center) < dist_min)
			{
				enemy_min = i;
				dist_min = pVision->TheirPlayer(i).Pos().dist(goal_center);
			}
		}
	}
	CGeoPoint target;
	if (enemy_min == 0)
	{
		target = CGeoPoint(DefDist, DefY[pos_num - 1]);
	}
	else
	{
		CGeoLine enemy_line(pVision->TheirPlayer(enemy_min).Pos(), goal_center);
		CGeoLineLineIntersection lli(enemy_line, def_line);
		if (lli.Intersectant() == true)
		{
			target = lli.IntersectPoint();
		}
		else
		{
			target = CGeoPoint(DefDist, DefY[pos_num - 1]);
		}
	}
	const double Buffer = Param::Vehicle::V2::PLAYER_SIZE;
	target = Utils::MakeOutOfOurPenaltyArea(target, Buffer);
	target = Utils::MakeInField(target, Buffer);
	target = Utils::MakeOutOfCircleAndInField(CGeoPoint(0, 0), Param::Field::CENTER_CIRCLE_R, target, Buffer);
	if (VERBOSE_MODE)
	{
		GDebugEngine::Instance()->gui_debug_x(target, COLOR_YELLOW);
	}
	return target;
}
int CKickOffDefPosV2::GetPosNum(const CVisionModule *pVision, const CGeoPoint pos)
{
	if (pos.y() < - Param::Field::CENTER_CIRCLE_R)
	{
		return 2;	// 左点 [7/1/2011 zhanfei]
	}
	else if (pos.y() > Param::Field::CENTER_CIRCLE_R)
	{
		return 3;	// 右点 [7/1/2011 zhanfei]
	}
	else
	{
		return 1;	// 中间点 [7/1/2011 zhanfei]
	}
}
CGeoPoint CKickOffDefPosV2::GetMidPos(const CVisionModule *pVision)
{
	//return this->GetKickOffDefPos(pVision, 1);
	return CGeoPoint(- Param::Field::CENTER_CIRCLE_R - Param::Vehicle::V2::PLAYER_SIZE - 1, 0);
}
CGeoPoint CKickOffDefPosV2::GetLeftPos(const CVisionModule *pVision)
{
	return this->GetKickOffDefPos(pVision, 2);
}
CGeoPoint CKickOffDefPosV2::GetRightPos(const CVisionModule *pVision)
{
	return this->GetKickOffDefPos(pVision, 3);
}