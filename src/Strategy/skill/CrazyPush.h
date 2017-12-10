#ifndef _CRAZY_PUSH_H_
#define _CRAZY_PUSH_H_

#include <skill/PlayerTask.h>
/**********************************************************
* High Level Skill: crazyPush
* Description: ÄÃÇòÍùÇ°¶¥
* Author: dxh and yys
* Created Date: 2013/06/14
* Updated Date: 2014/07/07
***********************************************************/
class CCrazyPush : public  CStatedTask{

public:
	CCrazyPush();
	~CCrazyPush();
	virtual void plan(const CVisionModule* pVision);
	virtual CPlayerCommand* execute(const CVisionModule * pVision);
	virtual bool isEmpty() const{ return false; }

private:
	int _lastCycle;
	int _state;
	int _cycle;
	int infraredOn;
	int infraredOff;
	int meHasBall;
	int meLoseBall;

	enum {
		CRAZY_PUSH = 1
	};

protected:
	CPlayerCommand* _directCommand;
};
#endif