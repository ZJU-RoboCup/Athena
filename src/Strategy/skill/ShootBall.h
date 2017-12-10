#ifndef __SHOOT_BALL_H__
#define __SHOOT_BALL_H__
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: ShootBall
* Description: ��ShootAgent����skill�е���
* Author: zhyaic
* E-mail: zhyaic@gmail.com
* Created Date: 2012.5.17

* Updated Date: 2015.6.10
* Description: ���ڸ���Skill�ı����������ԭ�еĲ����ж���������ɾ���˲��ֵ�״̬
* Author: FantasyChen
***********************************************************/

class CShootBall :public CStatedTask {
public:
	CShootBall();
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