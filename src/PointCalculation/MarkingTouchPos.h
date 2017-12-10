#ifndef __MARKING_TOUCHPOS_H__
#define __MARKING_TOUCHPOS_H__
#include "geometry.h"
#include "singleton.h"
/**********************************************************
* Skill: MarkingTouch
* Description: MarkingTouch
* Author: applejian
* E-mail: tangwenjian001@163.com
* Created Date: 2010.6/7 2014/3/3
***********************************************************/

class CMarkingTouchPos{
public:
	CMarkingTouchPos();
	void caculMarkingPos(int areaNum,const CGeoSegment& passSegment,const bool markDirection);
	CGeoPoint caculMarkingTouchPos(int areaNum,CGeoPoint leftUpPos,CGeoPoint rightDownPos,bool markDirection);
	void resetState();
	void setState(int i,int areaNum){
		_state[areaNum]=i;
	}
	int state(int areaNum){
		return _state[areaNum];
	}

private:
	static const int BEGINNING = 0; // ¿ªÊ¼×´Ì¬
	static const int MaxNum=6;

	int _lastCycle;
	int _kickEnemyNum;

	CGeoPoint _markPos[MaxNum];
	CGeoPoint _pointA[MaxNum];
	CGeoPoint _pointB[MaxNum];
	CGeoPoint _lastLeftUpPos[MaxNum];
	CGeoPoint _lastRightDownPos[MaxNum];

	int _markEnemyNum[MaxNum];
	int _marktoTouchCouter[MaxNum];
	int _touchtoAdvanceCouter[MaxNum];
	int _ballVelChangeCouter[MaxNum];
	bool _ballVelDirChanged[MaxNum];


	int _state[MaxNum];


};

typedef NormalSingleton< CMarkingTouchPos > MarkingTouchPos;

#endif //__SKILL2010_DFIFT_KICK_H__