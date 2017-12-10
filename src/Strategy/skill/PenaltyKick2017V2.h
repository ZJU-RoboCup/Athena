#ifndef _PENALTY_KICK_2017V2_H_
#define _PENALTY_KICK_2017V2_H_
#include <skill/PlayerTask.h>

class CPenaltyKick2017V2 : public CStatedTask
{
public:
	CPenaltyKick2017V2(void);
	~CPenaltyKick2017V2(void) {}
	virtual void plan(const CVisionModule* pVision);
	void updateMsg(const CVisionModule* pVision);
	CGeoPoint getBestPoint(const CVisionModule* pVision);
	bool isVisionHasBall(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Penalty kick"; }
private:
	int _lastCycle;
	int _theirGoalie;
	CGeoPoint _targetPoint;//左边还是右边的目标点
	int _targetside;
	int _timeCnt;
	bool _shootFlag;
	bool _chipFlag;

private:
	CGeoCirlce _enemyCircle;
	CGeoCirlce _enemyBallCircle;
	CGeoCirlce _meCircle;
	CVector _me2EnemyVec;
	bool _hasInter = 0;
	bool _isClose = 0;
	bool _isFar = 0;
};

#endif