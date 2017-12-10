#ifndef _GO_AROUND_ROBOT_H_
#define _GO_AROUND_ROBOT_H_
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: ÄÃÇò×ªÉíÌß
by dxh 2013/5/14
***********************************************************/

class CGoAroundRobot :public CStatedTask {
public:
	CGoAroundRobot();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: TestCircleBall\n"; }

private:
	int _lastCycle;
	CGeoPoint target;

};

#endif //_GO_AND_TURNKICK_H_