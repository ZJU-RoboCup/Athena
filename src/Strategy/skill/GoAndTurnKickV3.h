#ifndef _GO_AND_TURNKICKV3_H_
#define _GO_AND_TURNKICKV3_H_
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: ÄÃÇò×ªÉíÌß
by dxh 2013/5/14
***********************************************************/

class CGoAndTurnKickV3 :public CStatedTask {
public:
	CGoAndTurnKickV3();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: TestCircleBall\n"; }

private:
	int _lastCycle;
	double _lastRotVel;
	int count;
	int circleNum;
	double angeDiff_Use;
	double rotVel;
	CGeoPoint target;
	double CircleCounter;
	int adjustCount;
};

#endif //_GO_AND_TURNKICK_H_