#ifndef _GOALIE_POS_V1_
#define _GOALIE_POS_V1_

#include <VisionModule.h>
#include "geometry.h"

// ����Աλ���㷨 [5/17/2011 zhanfei]

class CGoaliePosV1
{
public:
	CGoaliePosV1();
	// ����ķ��ص� [6/15/2011 zhanfei]
	CGeoPoint GetPenaltyShootPos(const CVisionModule *pVision);
	// ����ķ��ص㣬������-������ [7/7/2011 zhanfei]
	CGeoPoint GetPenaltyShootPosV2(const CVisionModule *pVision);
	// ȡ������������ĵз��� [6/15/2011 zhanfei]
	int GetNearestEnemy(const CVisionModule *pVision);
};

typedef NormalSingleton< CGoaliePosV1 > GoaliePosV1;

#endif // _GOALIE_POS_V1_