#ifndef __INTERCEPT_BALL_V2_H__
#define __INTERCEPT_BALL_V2_H__
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: InterceptBallV2
* Description: 动态截球
* Author: FantasyChen
* E-mail: fantasychen_2016@163.com
* Created Date: 2015.4.30

***********************************************************/

class CInterceptBallV2 :public CStatedTask {
public:
	CInterceptBallV2();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: InterceptBallV2\n"; }

private:
	int _lastCycle;
	int _drribleCounter;
	int _drribleKickCounter;
	int _stateCounter;
	int _hasBallCounter;
	int _normalKickCounter;
	bool _touchBallFlag;
	CPlayerCommand* _directCommand;	//直接发送命令
	int checkEnemy(const CVisionModule* pVision);
	double enemyMenacePoint(const CVisionModule* pVision, const int enemyNum);
	bool faceTheirGoal(const CVisionModule* pVision, const int executor);
	bool isVisionHasBall(const CVisionModule* pVision, const int vecNumber);
	CGeoPoint checkPointAvoidOurPenalty(const CVisionModule* pVision, CGeoPoint targetPoint);
};

#endif //__INTERCEPT_BALL_V2_H__