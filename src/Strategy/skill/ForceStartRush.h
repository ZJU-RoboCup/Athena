#ifndef __FORCESTART_RUSH__
#define __FORCESTART_RUSH__
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: ForceStartRush
* Description: 
* Author: applejian
* E-mail: 
* Created Date: 
***********************************************************/

class CForceStartRush :public CStatedTask {
public:
	CForceStartRush();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: CForceStartRush\n"; }

private:
	CPlayerCommand* _directCommand;	//Ö±½Ó·¢ËÍÃüÁî
	int _lastCycle;
	int _stateCounter;
};

#endif //__FORCESTART_RUSH__