#ifndef _GOALIE_POS_V1_
#define _GOALIE_POS_V1_

#include <VisionModule.h>
#include "geometry.h"

// 守门员位置算法 [5/17/2011 zhanfei]

class CGoaliePosV1
{
public:
	CGoaliePosV1();
	// 点球的防守点 [6/15/2011 zhanfei]
	CGeoPoint GetPenaltyShootPos(const CVisionModule *pVision);
	// 点球的防守点，跟随球-门连线 [7/7/2011 zhanfei]
	CGeoPoint GetPenaltyShootPosV2(const CVisionModule *pVision);
	// 取得离球门最近的敌方车 [6/15/2011 zhanfei]
	int GetNearestEnemy(const CVisionModule *pVision);
};

typedef NormalSingleton< CGoaliePosV1 > GoaliePosV1;

#endif // _GOALIE_POS_V1_