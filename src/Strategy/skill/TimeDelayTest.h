#ifndef __TIME_DELAY_TEST_H__
#define __TIME_DELAY_TEST_H__
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: Time Delay Test
* Description: —” ±≤‚ ‘∫Ø ˝
***********************************************************/

class CTimeDelayTest :public CStatedTask {
public:
	CTimeDelayTest();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: TimeDelayTest\n"; }

private:
	int _lastCycle;
	CPlayerCommand * _directCommand;
};

#endif //__TIME_DELAY_TEST_H__