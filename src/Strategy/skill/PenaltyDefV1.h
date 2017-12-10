#ifndef __PENALTY_DEF_V1_H__
#define __PENALTY_DEF_V1_H__
#include <skill/PlayerTask.h>
/**********************************************************
* Skill: PenaltyDefV1
* Description: ∑¿ ÿµ„«Ú
* Author: zhanfei
* E-mail: zillion217@gmail.com
* Created Date: 2011-06-12
***********************************************************/

class CPenaltyDefV1 :public CStatedTask {
public:
	CPenaltyDefV1();
	virtual void plan(const CVisionModule* pVision);
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
	virtual bool isEmpty() const { return false; }
protected:
	virtual void toStream(std::ostream& os) const { os << "CPenaltyDefV1"; }
private:
	CPlayerCommand* _directCommand;
	int _lastCycle;
};

#endif //__PENALTY_DEF_V1_H__