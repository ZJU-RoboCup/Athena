#ifndef _GO_AROUND_BALL_H_
#define _GO_AROUND_BALL_H_
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: CGoAroundBall ,在护球时调用
***********************************************************/

class CGoAroundBall :public CStatedTask {
public:
	CGoAroundBall();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: CGoAroundBall\n"; }

private:
	int _lastCycle;
};

#endif //_GO_AROUND_BALL_H_