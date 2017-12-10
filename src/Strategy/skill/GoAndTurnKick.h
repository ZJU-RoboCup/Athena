#ifndef _GO_AND_TURNKICK_H_
#define _GO_AND_TURNKICK_H_
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: ÄÃÇò×ªÉíÌß
***********************************************************/

class CGoAndTurnKick :public CStatedTask {
public:
	CGoAndTurnKick();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: CGoAndTurnKick\n"; }

private:
	int _lastCycle;
	double _lastRotVel;
};

#endif //_GO_AND_TURNKICK_H_