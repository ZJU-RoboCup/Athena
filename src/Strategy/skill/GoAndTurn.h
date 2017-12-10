#ifndef _GO_AND_TURN_H_
#define _GO_AND_TURN_H_
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: ÄÃÇò×ªÉíÌß
by dxh 2013/5/14
***********************************************************/

class CGoAndTurn :public CStatedTask {
public:
	CGoAndTurn();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: TestCircleBall\n"; }

private:
	int _lastCycle;
	double _lastRotVel;
	int count;
	double angeDiff_Use;
	double rotVel;
	double R;
	double CircleCounter;
	int part;
	CGeoPoint target;

};

#endif //_GO_AND_TURNKICK_H_