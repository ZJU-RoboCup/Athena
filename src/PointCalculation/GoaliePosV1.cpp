#include "GoaliePosV1.h"
#include <GDebugEngine.h>
#include "ShootRangeList.h"
#include "PlayInterface.h"
#include "TaskMediator.h"
#include "ValueRange.h"
#include <vector>
#include <math.h>
#include "BestPlayer.h"
#include <map>

CGoaliePosV1::CGoaliePosV1()
{

}

int CGoaliePosV1::GetNearestEnemy(const CVisionModule *pVision)
{
	CGeoPoint goal_center(- Param::Field::PITCH_LENGTH / 2, 0);
	double nearest_dist = Param::Field::PITCH_LENGTH;
	int enemy_num = 1;
	for (int i = 1; i <= Param::Field::MAX_PLAYER; i ++)
	{
		if (pVision->TheirPlayer(i).Valid() == true)
		{
			if (pVision->TheirPlayer(i).Pos().dist(goal_center) < nearest_dist)
			{
				nearest_dist = pVision->TheirPlayer(i).Pos().dist(goal_center);
				enemy_num = i;
			}
		}
	}
	return enemy_num;
}
CGeoPoint CGoaliePosV1::GetPenaltyShootPos(const CVisionModule *pVision)
{
	const MobileVisionT ball = pVision->Ball();
	const PlayerVisionT& enemy = pVision->TheirPlayer(this->GetNearestEnemy(pVision));
	double enemy_dir = enemy.Dir();
	CGeoLine enemy_sht_line(enemy.Pos(), enemy.Pos() + Utils::Polar2Vector(10, enemy_dir));
	if (enemy_dir < Param::Math::PI / 2 && enemy_dir > - Param::Math::PI / 2)
	{
		enemy_sht_line = CGeoLine(ball.Pos(), ball.Pos() + Utils::Polar2Vector(10, Param::Math::PI));
	}
	// 不应该用加大的球门 [6/15/2011 zhanfei]
	const double goalie_x = Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + 1;
	const CGeoLine goal_line(CGeoPoint(-Param::Field::PITCH_LENGTH / 2 + goalie_x, Param::Field::GOAL_WIDTH / 2)
		, CGeoPoint(-Param::Field::PITCH_LENGTH / 2 + goalie_x, -Param::Field::GOAL_WIDTH / 2));
	const CGeoLineLineIntersection lli(enemy_sht_line, goal_line);
	CGeoPoint pen_sht_pos;
	if (lli.Intersectant() == false)
	{
		pen_sht_pos = CGeoPoint(-Param::Field::PITCH_LENGTH / 2, 0);
	}
	else
	{
		pen_sht_pos = lli.IntersectPoint();
	}
	const double GOAL_BUFFER = -3;
	if (pen_sht_pos.y() < -Param::Field::GOAL_WIDTH / 2 + Param::Vehicle::V2::PLAYER_SIZE - GOAL_BUFFER)
	{
		pen_sht_pos = CGeoPoint(pen_sht_pos.x(), -Param::Field::GOAL_WIDTH / 2 + Param::Vehicle::V2::PLAYER_SIZE - GOAL_BUFFER);
	}
	else if (pen_sht_pos.y() > Param::Field::GOAL_WIDTH / 2 - Param::Vehicle::V2::PLAYER_SIZE + GOAL_BUFFER)
	{
		pen_sht_pos = CGeoPoint(pen_sht_pos.x(), Param::Field::GOAL_WIDTH / 2 - Param::Vehicle::V2::PLAYER_SIZE + GOAL_BUFFER);
	}
	return pen_sht_pos;
}
CGeoPoint CGoaliePosV1::GetPenaltyShootPosV2(const CVisionModule *pVision)
{
	const MobileVisionT ball = pVision->Ball();
	const PlayerVisionT& enemy = pVision->TheirPlayer(this->GetNearestEnemy(pVision));
	//double enemy_dir = enemy.Dir();
	const CGeoPoint goal_center(- Param::Field::PITCH_LENGTH / 2, 0);
	double enemy_dir = Utils::Normalize((ball.Pos() - goal_center).dir());
	CGeoLine enemy_sht_line(enemy.Pos(), enemy.Pos() + Utils::Polar2Vector(10, enemy_dir));
	if (enemy_dir < Param::Math::PI / 2 && enemy_dir > - Param::Math::PI / 2)
	{
		enemy_sht_line = CGeoLine(ball.Pos(), ball.Pos() + Utils::Polar2Vector(10, Param::Math::PI));
	}
	// 不应该用加大的球门 [6/15/2011 zhanfei]
	const double goalie_x = Param::Vehicle::V2::PLAYER_SIZE;
	const CGeoLine goal_line(CGeoPoint(-Param::Field::PITCH_LENGTH / 2 + goalie_x, Param::Field::GOAL_WIDTH / 2)
		, CGeoPoint(-Param::Field::PITCH_LENGTH / 2 + goalie_x, -Param::Field::GOAL_WIDTH / 2));
	const CGeoLineLineIntersection lli(enemy_sht_line, goal_line);
	CGeoPoint pen_sht_pos;
	if (lli.Intersectant() == false)
	{
		pen_sht_pos = CGeoPoint(-Param::Field::PITCH_LENGTH / 2, 0);
	}
	else
	{
		pen_sht_pos = lli.IntersectPoint();
	}
	const double GOAL_BUFFER = 1;
	if (pen_sht_pos.y() < -Param::Field::GOAL_WIDTH / 2 + Param::Vehicle::V2::PLAYER_SIZE - GOAL_BUFFER)
	{
		pen_sht_pos = CGeoPoint(pen_sht_pos.x(), -Param::Field::GOAL_WIDTH / 2 + Param::Vehicle::V2::PLAYER_SIZE - GOAL_BUFFER);
	}
	else if (pen_sht_pos.y() > Param::Field::GOAL_WIDTH / 2 - Param::Vehicle::V2::PLAYER_SIZE + GOAL_BUFFER)
	{
		pen_sht_pos = CGeoPoint(pen_sht_pos.x(), Param::Field::GOAL_WIDTH / 2 - Param::Vehicle::V2::PLAYER_SIZE + GOAL_BUFFER);
	}
	return pen_sht_pos;
}