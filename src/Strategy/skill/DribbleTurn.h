#ifndef _DRIBBLE_TURN_H_
#define _DRIBBLE_TURN_H_
#include <skill/PlayerTask.h>

/**********************************************************/
/* Skill: ÎüÇò×ªÉí
/* by yys 2014-05-26
/**********************************************************/

class CDribbleTurn:public CStatedTask {
public:
	CDribbleTurn();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty() const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: DribbleTurn\n" << std::endl; }
	bool CDribbleTurn::isVisionHasBall(const CVisionModule* pVision, const int vecNumber);

private:
	enum TestCircleBall {
		GET = 1,
		TURN,
		END,
	};
	int _lastCycle;
	double _lastRotVel;
	int count;
	int circleNum;
	double angeDiff_Use;
	double rotVel;
	double CircleCounter;
	int adjustCount;
	int meHasBall;
	int meLoseBall;
	int infraredOn;
};

#endif //_DRIBBLE_TURN_H_