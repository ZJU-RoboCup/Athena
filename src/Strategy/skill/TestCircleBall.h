#ifndef _TEST_CIRCLE_BALL_H_
#define _TEST_CIRCLE_BALL_H_
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: ÄÃÇò×ªÉíÌß
***********************************************************/

class CTestCircleBall :public CStatedTask {
public:
	CTestCircleBall();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: TestCircleBall\n"; }

private:
//	int _lastCycle;
	double _lastRotVel;
	int count;
	double angeDiff_Use;
	double rotVel;

};

#endif //_GO_AND_TURNKICK_H_