#ifndef __WAIT_CHIP_2016_H__
#define __WAIT_CHIP_2016_H__
#include <skill/PlayerTask.h>
#include "VisionModule.h"

class CWaitChip2016 : public CStatedTask{
public:
	CWaitChip2016();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual	CPlayerCommand *execute(const CVisionModule* pVision);

protected:
	virtual void toStream(std::ostream& os) const { os << "WaitChip2016\n"; }

private:
	bool isBallLanded(CVector ball_vel, int mode=0);
	bool calcBallLine(CGeoPoint& pt, CGeoLine& line, int mode=0);
	int last_cycle;
	bool need_shoot;
	int cur_cnt;
};
#endif