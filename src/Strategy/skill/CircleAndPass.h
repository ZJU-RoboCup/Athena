#ifndef _CIRCLE_AND_PASS_
#define _CIRCLE_AND_PASS_
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: CircleAndPass
***********************************************************/

class CCircleAndPass : public CStatedTask {
public:
	CCircleAndPass();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: CCircleAndPass\n"; }

private:
	int _lastCycle;
	double _lastRotVel;
};

#endif