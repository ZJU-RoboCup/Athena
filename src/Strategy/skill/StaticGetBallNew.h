#pragma once
#include <skill/PlayerTask.h>
#include "VisionModule.h"

/**********************************************************
* Skill: CStaticGetBallNew
* Description: Only for STILL ball
* Author: Wayne
* Created Date: 2017/12/13
***********************************************************/
class CStaticGetBallNew : public CStatedTask
{
public:
	CStaticGetBallNew();
	~CStaticGetBallNew();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual	CPlayerCommand *execute(const CVisionModule* pVision);

protected:
	virtual void toStream(std::ostream& os) const { os << "StaticGetBallNew\n"; }
private:
	CPlayerCommand* _directCommand;
	bool canGetBall;
	int getStaticDir(const CVisionModule* pVision, int staticDir);
};