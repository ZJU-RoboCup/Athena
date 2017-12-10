#ifndef _GET_BALL_V4_H__
#define _GET_BALL_V4_H__
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: GetBallV4
* Description:ÄÃÇòµÚËÄ°æ
***********************************************************/

class CGetBallV4 :public CStatedTask {
public:
	CGetBallV4();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
	int getStaticDir(const CVisionModule* pVision, int staticDir);
	void judgeMode(const CVisionModule* pVision);
	bool canShootBall(const CVisionModule* pVision);
	bool judgeShootMode(const CVisionModule* pVision);
	CVector getShootVel(const CVisionModule* pVision);
	CGeoPoint getBallPredictPos(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: GetBallV4\n"; }

private:
	int _lastCycle;
	int getBallMode;
	int cnt = 0;
	bool IsInField(const CGeoPoint);
	bool needDribble = false;
	double last_speed = 0;
};

#endif //_GET_BALL_V4_H__