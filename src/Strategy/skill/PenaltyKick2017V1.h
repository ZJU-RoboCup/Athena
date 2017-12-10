#ifndef _PENALTY_KICK_2017V1_H_
#define _PENALTY_KICK_2017V1_H_

#include <skill/PlayerTask.h>

class CPenaltyKick2017V1 : public CStatedTask
{
public:
	CPenaltyKick2017V1(void);
	~CPenaltyKick2017V1(void) {}
	virtual void plan(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Penalty kick"; }
	void planPrepare(const CVisionModule* pVision);
	void planChipKick(const CVisionModule* pVision);
	void planDirectKick(const CVisionModule* pVision);
	bool canShoot(const CVisionModule* pVision);
private:
	int _lastCycle;
	int _theirGoalie;
	CGeoPoint _targetPoint;//��߻����ұߵ�Ŀ���
	int _targetside;
	int _waitingcount;
	int _round;
	bool _prepared;
	bool _canShoot;
	bool _getball;
};

#endif