#ifndef __WAITTOUCH_h__
#define __WAITTOUCH_h__
#include <skill/PlayerTask.h>
#include "VisionModule.h"

class CWaitTouch : public CStatedTask{
public:
	CWaitTouch();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual	CPlayerCommand *execute(const CVisionModule* pVision);

protected:
	virtual void toStream(std::ostream& os) const { os << "WaitKick\n"; }
private:
	int last_cycle;
	bool need_shoot;
	int cur_cnt;
};
#endif