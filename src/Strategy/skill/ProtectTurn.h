#ifndef _PROTECT_TURN_
#define _PROTECT_TURN_
#include <skill/PlayerTask.h>

class CProtectTurn :public CStatedTask
{
public:
	CProtectTurn();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: ProtectTurn\n"; }

private:
	int _lastCycle;
	int _lastRunner;
	int _turnToKickCouter;
	CPlayerCommand * _directCommand;
	double _initDir;
};
#endif