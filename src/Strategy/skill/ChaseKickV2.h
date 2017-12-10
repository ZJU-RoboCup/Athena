#ifndef __SKILL2010_CHASE_KICK_V2_H__
#define __SKILL2010_CHASE_KICK_V2_H__
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: ChaseKickV2
* Description: �򵥵�׷�ߣ�������������
* Author: applejian
* E-mail: tangwenjian001@163.com
* Created Date: 2010.6/7 2014/3/3

* Updated Date: 2015.3
* Description: ���ӵ�׷�ߣ�����˸��������³����
* Author: FantasyChen

* Updated Date: 2016.3.28
* Description: ά����״̬��������������ʱ����chip��bug
* Author: PatrickHong
***********************************************************/

class CChaseKickV2 :public CStatedTask {
public:
	CChaseKickV2();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: ChaseKickV2\n"; }

private:
	bool faceTheirGoal(const CVisionModule* pVision, const int executor);
	CGeoPoint checkPointAvoidOurPenalty(const CVisionModule* pVision, CGeoPoint targetPoint);
	CPlayerCommand* _directCommand;	//ֱ�ӷ�������
	int _lastCycle;
	int _stateCounter;
	int _goKickCouter;
	double _compensateDir;
	int _relinquishCounter;
	int _openKickCounter;
};

#endif //__SKILL2010_CHASE_KICK_V1_H__