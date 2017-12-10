#ifndef _INTERCEPT_BALL_V3_H_
#define _INTERCEPT_BALL_V3_H_
#include <skill/PlayerTask.h>
/*
 * CInterceptBallV3.
 *原有InterceptBall效果不好
 * 改进版本
 */

class CInterceptBallV3 :public CStatedTask{
public:
	CInterceptBallV3();
	~CInterceptBallV3();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty() const { return false; }

	enum{ APPROACH_BALL = 10, STABLE_BALL ,TURN};

protected:
	virtual void toStream(std::ostream& os)const;
private:
	int checkOpp(const CVisionModule* pVision);
	bool checkOppProccession(const CVisionModule* pVision, const int oppID);
	void planApproachOpp(const CVisionModule* pVision, const int myID, const int oppID,const double finaldir,double dist);
	//void planJamBall(const CVisionModule* pVision, const int myID, const int oppID, const CGeoPoint& ballTarget);
	//void planPushOpp(const CVisionModule* pVision, const int myID, const int oppID);
	//void planPushBallOut(const CVisionModule* pVision, const int myID, const int oppID);
	bool isVisionHasBall(const CVisionModule* pVision,const int vecNumber);
	//	
	int _lastCycle;
//	bool jamming;
//	bool pushing_opp;
	int infraredOn;
	int infraredOff;
	int meHasBall;
	int meLoseBall;

};
#endif 