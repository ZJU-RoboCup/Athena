#ifndef __SKILL2010_MARKING_TOUCH_H__
#define __SKILL2010_MARKING_TOUCH_H__
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: MarkingTouch
* Description: MarkingTouch
* Author: applejian
* E-mail: tangwenjian001@163.com
* Created Date: 2010.6/7 2014/3/3
***********************************************************/

class CMarkingTouch :public CStatedTask {
public:
	CMarkingTouch();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
	void caculMarkingPos(const CGeoSegment& passSegment,const bool markDirection);
	void resetState();
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: MarkingTouch\n"; }

private:
	int _lastCycle;
	int _stateCounter;
	int _markEnemyNum;
	int _kickEnemyNum;
	CGeoPoint _markPos;
	CGeoPoint _pointA;
	CGeoPoint _pointB;
	int _marktoTouchCouter;
	int _touchToBeginCouter;
	int _markAgainToBeginCouter;
	int _ballVelChangeCouter;
	bool _ballVelDirChanged;
	CGeoPoint _lastLeftUpPos;
	CGeoPoint _lastRightDownPos;
};

#endif //__SKILL2010_DFIFT_KICK_H__