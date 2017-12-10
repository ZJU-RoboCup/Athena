#ifndef _GO_AVOID_SHOOTLINE_
#define _GO_AVOID_SHOOTLINE_
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: GoAvoidShootLine
* Description: 
* Author: applejian
* E-mail: tangwenjian001@163.com
* Created Date:2014/4/7
***********************************************************/

class CGoAvoidShootLine :public CStatedTask {
public:
	CGoAvoidShootLine();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: GoAvoidShootLine\n"; }

private:
	int _lastCycle;
	int _stateCouter;
};

#endif 