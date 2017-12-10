#ifndef _DRIBBLE_TURN_KICK_
#define _DRIBBLE_TURN_KICK_
#include <skill/PlayerTask.h>

class CDribbleTurnKick :public CStatedTask
{
public:
	CDribbleTurnKick();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: DribbleTurnKick\n"; }

private:
	int _lastCycle;
	int _lastRunner;
	int _turnToKickCouter;
	CPlayerCommand * _directCommand;
	double _initDir;
};
#endif