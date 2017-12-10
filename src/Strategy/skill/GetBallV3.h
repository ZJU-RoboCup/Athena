#ifndef _GET_BALL_V3_H__
#define _GET_BALL_V3_H__
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: GetBallV3
* Description:拿球第三版，适用于小嘴巴底盘
***********************************************************/

class CGetBallV3 :public CStatedTask {
public:
	CGetBallV3();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: GetBallV3\n"; }

private:
	int _lastCycle;
};

#endif //_GET_BALL_V3_H__