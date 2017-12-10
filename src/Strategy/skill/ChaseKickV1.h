#ifndef __SKILL2010_CHASE_KICK_V1_H__
#define __SKILL2010_CHASE_KICK_V1_H__
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: ChaseKickV1
* Description: 简单的追踢，考虑球速在内
* Author: cliffyin and applejian
* E-mail: cliffyin007@gmail.com tangwenjian001@163.com
* Created Date: 2010.6/7 2014/3/3
***********************************************************/

class CChaseKickV1 :public CStatedTask {
public:
	CChaseKickV1();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: ChaseKickV1\n"; }

private:
	CPlayerCommand* _directCommand;	//直接发送命令
	int _lastCycle;
	int StateCouter;
};

#endif //__SKILL2010_CHASE_KICK_V1_H__