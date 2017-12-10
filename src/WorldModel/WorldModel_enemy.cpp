#include "WorldModel.h"
#include <algorithm>
#include "TaskMediator.h"
#include "PlayInterface.h"
#include "GDebugEngine.h"
#include "BestPlayer.h"
#include "ShootRangeList.h"
#include "math.h"

/************************************************************************/
/* �ṩWorldModel_enemy�����и��º�������ѯ����                    */
/************************************************************************/

enum _enemyTeam{
	COSTOM = 0,
	SKUBA = 1,
	IMMORTAL,
};
namespace{
	int lastCycle = 0;
	CGeoPoint myGoalCenter = CGeoPoint(-300,0);									//��������

	const double ballTakedDist = Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + 5; //�Ƿ��������ȵľ����о�
	const double ballTakedAngle = Param::Math::PI / 3.0;						 //�Ƿ��������ȵĽǶ��о�
	const double change2attackPosition = 110;		//��ɽ�����λ��xֵ
	const double noLonger2attackPosition = 160;		//�����ǽ�����λ��xֵ
	const double dist_Factor = 1.0;					//��������ϵ��
	const double shoot_Factor = 1.0;				//���ŽǶ�ϵ��	
	const double keeperFace_Factor = 1.0;			//�����˶��䳯���ϵ��
	const double dangerousAngle = Param::Math::PI / 6;
	const double ENEMY_PASS_VEL = 50;				//�жϵ��˿��ٴ��������ori:100

	//DEBUG
	const bool debug_ShootDir = false;
	const bool debug_Mark = false;
	const bool debug_ballTaked = false;
	const bool debug_attackNum = false;
	_enemyTeam enemyTeam = SKUBA;
}