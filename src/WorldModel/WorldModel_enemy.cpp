#include "WorldModel.h"
#include <algorithm>
#include "TaskMediator.h"
#include "PlayInterface.h"
#include "GDebugEngine.h"
#include "BestPlayer.h"
#include "ShootRangeList.h"
#include "math.h"

/************************************************************************/
/* 提供WorldModel_enemy中所有更新函数，查询函数                    */
/************************************************************************/

enum _enemyTeam{
	COSTOM = 0,
	SKUBA = 1,
	IMMORTAL,
};
namespace{
	int lastCycle = 0;
	CGeoPoint myGoalCenter = CGeoPoint(-300,0);									//球门中心

	const double ballTakedDist = Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + 5; //是否拿球拿稳的距离判据
	const double ballTakedAngle = Param::Math::PI / 3.0;						 //是否拿球拿稳的角度判据
	const double change2attackPosition = 110;		//变成进攻单位的x值
	const double noLonger2attackPosition = 160;		//不再是进攻单位的x值
	const double dist_Factor = 1.0;					//补防距离系数
	const double shoot_Factor = 1.0;				//射门角度系数	
	const double keeperFace_Factor = 1.0;			//持球人对其朝向的系数
	const double dangerousAngle = Param::Math::PI / 6;
	const double ENEMY_PASS_VEL = 50;				//判断敌人快速传球的球速ori:100

	//DEBUG
	const bool debug_ShootDir = false;
	const bool debug_Mark = false;
	const bool debug_ballTaked = false;
	const bool debug_attackNum = false;
	_enemyTeam enemyTeam = SKUBA;
}