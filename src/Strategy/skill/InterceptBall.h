#ifndef __INTERCEPT_BALL__
#define __INTERCEPT_BALL__
#include <skill/PlayerTask.h>

class CInterceptBall :public CStatedTask
{
public:
	CInterceptBall(){}
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: CIntercept Ball\n"; }

private:

};

#endif //__GOALIE_2013_H__