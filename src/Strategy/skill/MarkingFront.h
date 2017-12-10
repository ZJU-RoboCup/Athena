#ifndef __SKILL2010_MARKING_FRONT_H__
#define __SKILL2010_MARKING_FRONT_H__
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: MarkingFront
* Description: MarkingFront
* Author: applejian
* E-mail: tangwenjian001@163.com
* Created Date: 2010.6/7
***********************************************************/

class CMarkingFront :public CStatedTask {
public:
	CMarkingFront();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
	void resetState();
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: MarkingTouch\n"; }

private:
	int _lastCycle;
	int _stateCounter;
	int _markEnemyNum;
	int _kickEnemyNum;
	CGeoPoint _markPos;
	int _marktoTouchCouter;
	int _touchToBeginCouter;
	int _markAgainToBeginCouter;
	int _ballVelChangeCouter;
	bool _ballVelDirChanged;
	CGeoPoint _lastLeftUpPos;
	CGeoPoint _lastRightDownPos;
	double _passDir;
	bool toIntercept;
};

#endif //__SKILL2010_DFIFT_KICK_H__