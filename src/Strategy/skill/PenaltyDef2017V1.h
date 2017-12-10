#ifndef __PENALTY_DEF_2017V1_H__
#define __PENALTY_DEF_2017V1_H__
#include <skill/PlayerTask.h>

class CPenaltyDef2017V1 :public CStatedTask {
public:
	CPenaltyDef2017V1();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
	CGeoPoint getHorizonPoint(const CVisionModule* pVision);
	CGeoPoint getTurnPoint(const CVisionModule* pVision);
	bool needHorizon(const CVisionModule* pVision);
	bool isBallShoot(const CVisionModule* pVision);
	bool needTurnBack(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: PenaltyGoalie2012\n"; }
private:
	bool _isBallShoot;
	int _horizonType;
	int _turnType;
	bool _needHorizon;
	bool _needTurnBack;
	int _state;
	double _dirDiff;
	double _enemyDir;
	double _lastEnemyDir;
	int _lastCycle;
};

#endif //__PENALTY_GOALIE_2012_H__