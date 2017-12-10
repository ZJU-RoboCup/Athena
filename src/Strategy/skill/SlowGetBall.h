#ifndef _SLOW_GET_BALL_H_
#define _SLOW_GET_BALL_H_
#include <skill/PlayerTask.h>

/**********************************************************
/* Skill: SlowGetBall
/* Description:������ǰ����,������ס�������ڶ�λ����
/***********************************************************/

class CSlowGetBall :public CStatedTask {
public:
	CSlowGetBall();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: SlowGetBall\n"; }

private:
	int _lastCycle;
	CPlayerCommand * _directCommand;
};

#endif //_SLOW_GET_BALL_H__