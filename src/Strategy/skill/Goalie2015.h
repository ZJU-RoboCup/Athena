#ifndef __GOALIE_2015_H__
#define __GOALIE_2015_H__
#include <skill/PlayerTask.h>

class CGoalie2015 :public CStatedTask 
{
public:
	CGoalie2015();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: CGoalie2013\n"; }
	bool needtoClearBall(const CVisionModule* pVision);
	bool needtoAttackEnemy(const CVisionModule* pVision);
	bool checkWeHaveHelper(const CVisionModule* pVision);
private:
	int state;
	double calcKickBackAng(const CVisionModule* pVision);
	bool veryDangerous(const CVisionModule* pVision);
};

#endif //__GOALIE_2013_H__