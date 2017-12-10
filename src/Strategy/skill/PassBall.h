#ifndef __PASS_BALL_H__
#define __PASS_BALL_H__
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: PassBall
* Description: 将PassAgent放入skill中调试
* Author: zhyaic
* E-mail: zhyaic@gmail.com
* Created Date: 2012.5.25
***********************************************************/

class CPassBall :public CStatedTask {
public:
	CPassBall();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: CPassBall\n"; }

private:
	CPlayerCommand* _directCommand;	//直接发送命令
	int _lastCycle;
	int _lastRunner;
};

#endif //__SHOOT_BALL_H__