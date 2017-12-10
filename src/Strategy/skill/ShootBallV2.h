#ifndef __SHOOT_BALL_V2_H__
#define __SHOOT_BALL_V2_H__
#include <skill/PlayerTask.h>

/**********************************************************
* Skill:         重新修改的shoot用于与V1做对比
* Description:   ShootBallV2
* Author:        Mark修改自ShootBall
* Created Date:  2016.5.10
***********************************************************/

class CShootBallV2 :public CStatedTask {
public:
	CShootBallV2();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: ShootBall\n"; }

private:
	int _lastCycle;
	int _lastRunner;
	int _out_inter_cnt;
	bool _out_inter;
	double _backShootDir;
	double _savedCycle;
	double _ballVelChangeCounter;
	double _lastBallVelDir;
	double _defendGoalChangeCounter;
	double checkShootDirNotOutTheirGoal(const CVisionModule* pVision, const double finalDir,const double buffer);
};

#endif //__SHOOT_BALL_H__