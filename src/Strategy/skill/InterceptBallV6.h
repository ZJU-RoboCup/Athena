#pragma once
#include <skill/PlayerTask.h>
#include "VisionModule.h"

/**********************************************************
* Skill: CInterceptBallV6
* Description: 
* Author: Wayne
* Created Date: 2017/11/29
***********************************************************/
class CInterceptBallV6 : public CStatedTask
{
public:
	CInterceptBallV6();
	~CInterceptBallV6();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual	CPlayerCommand *execute(const CVisionModule* pVision);

protected:
	virtual void toStream(std::ostream& os) const { os << "InterceptBallV6\n"; }

private:
	CPlayerCommand* _directCommand;
	int cnt = 0;
	bool IsInField(const CGeoPoint);
	bool needDribble = false;
	double last_speed = 0;
	double lastAngle = 0;
	CGeoPoint waitPoint;
};

