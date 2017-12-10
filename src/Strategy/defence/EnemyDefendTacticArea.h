#ifndef _ENEMY_DEFEND_TACTIC_AREA_H_
#define _ENEMY_DEFEND_TACTIC_AREA_H_
/**
* Author: dxh
* Date: 2013.6.4
* Description: 用来做传球区域的划分和评价
* Email: zhyaic@gmail.com
*/
#include <string>
#include "geometry.h"

class CVisionModule;

enum{
	CIRCLE =1,
	LONGCIRCLE,
	ANNULUS
};

class CEnemyDefendTacticArea{
	public:
		CEnemyDefendTacticArea();
		~CEnemyDefendTacticArea();
		bool isInCircleArea(CGeoPoint & enemyCarPos);
		bool isInAnnulus(CGeoPoint & enemyCarPos);
		bool isInLongCircle(CGeoPoint & enemyCarPos);
		bool isMarkingOurCar(int enemyCarNum , const CVisionModule * pVision);
		bool isOnBallHalf(int enemyCarNum,const CVisionModule* pVision);
		
	public:
		int ourCarNum;
		int mask;
		int areaShape;
		CGeoPoint circleCenter;
		double jointLength;
		double interRadius;
		double outerRadius;
};

#endif