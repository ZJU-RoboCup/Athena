#ifndef __PENALTY_GOALIE_2012_H__
#define __PENALTY_GOALIE_2012_H__
#include <skill/PlayerTask.h>

class CPenaltyGoalie2012 :public CStatedTask {
public:
	CPenaltyGoalie2012();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: PenaltyGoalie2012\n"; }

private:
	double _dirDiff;
	double _enemyDir;
	double _lastEnemyDir;
	bool _rushGo;
	int _lastCycle;
	CGeoPoint _rushPoint;
};

#endif //__PENALTY_GOALIE_2012_H__