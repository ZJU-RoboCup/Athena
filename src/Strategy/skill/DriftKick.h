#ifndef __SKILL2010_DFIFT_KICK_H__
#define __SKILL2010_DFIFT_KICK_H__
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: DriftKick
* Description: 横向追踢
* Author: applejian
* E-mail: tangwenjian001@163.com
* Created Date: 2010.6/7 2014/3/3
***********************************************************/

class CDriftKick :public CStatedTask {
public:
	CDriftKick();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: DriftKick\n"; }

private:
	CPlayerCommand* _directCommand;	//直接发送命令
	int _lastCycle;
	int _stateCounter;
	int _goKickCouter;
};

#endif //__SKILL2010_DFIFT_KICK_H__