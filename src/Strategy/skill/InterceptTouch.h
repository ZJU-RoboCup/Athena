#ifndef __INTERCEPTTOUCH_h__
#define __INTERCEPTTOUCH_h__
#include <skill/PlayerTask.h>
#include "VisionModule.h"

/**********************************************************
* Skill: InterTouch
* Description: 面朝某个点截球 然后直接射出
               适用于进攻和防守反击
* Author: soap
* Created Date: 2016.6.19
***********************************************************/

class CInterceptTouch : public CStatedTask {
public:
	CInterceptTouch();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty() const { return false; }
	virtual	CPlayerCommand* execute(const CVisionModule* pVision);

protected:
	virtual void toStream(std::ostream& os) const { os << "InterceptTouch\n"; }

private:
  enum {
    SIMPLY_GOTO_WAIT_POS = 1,
    GOTO_PROJ_POS,
    INTERCEPT,
	SAVE
  };

  int buffer_counter_;
	int last_cycle_;
};

#endif