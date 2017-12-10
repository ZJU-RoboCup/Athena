#ifndef __PROTECT_BALL_H__
#define __PROTECT_BALL_H__
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: PROTECTBall
* Description: 
* Author: applejian
* E-mail: 
* Created Date: 
***********************************************************/

class CProtectBall :public CStatedTask {
public:
	CProtectBall();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: CPassBall\n"; }

private:
	CPlayerCommand* _directCommand;	//Ö±½Ó·¢ËÍÃüÁî
	int _lastCycle;
	int _stateCounter;
};

#endif //__SHOOT_BALL_H__