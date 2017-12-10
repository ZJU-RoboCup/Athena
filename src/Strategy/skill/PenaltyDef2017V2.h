#ifndef __PENALTY_DEF_2017V2_H__
#define __PENALTY_DEF_2017V2_H__
#include <skill/PlayerTask.h>

class CPenaltyDef2017V2 :public CStatedTask {
public:
	CPenaltyDef2017V2();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
	bool isBallShoot(const CVisionModule * pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: PenaltyGoalie2012\n"; }
private:
	int _lastCycle;
	int _case;
	bool _rushFlag;
	bool _turnFlag;
};

#endif //__PENALTY_GOALIE_2012_H__