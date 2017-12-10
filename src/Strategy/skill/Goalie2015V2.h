#ifndef __GOALIE_2015V2_H__
#define __GOALIE_2015V2_H__
#include <skill/PlayerTask.h>

class CGoalie2015V2 :public CStatedTask 
{
public:
	CGoalie2015V2();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: CGoalie 2015\n"; }
	bool needtoClearBall(const CVisionModule* pVision);
	bool needtoAttackEnemy(const CVisionModule* pVision);
	bool checkWeHaveHelper(const CVisionModule* pVision);
	double calcKickBackAng(const CVisionModule* pVision);

private:
	int state;
};

#endif //__GOALIE_2013_H__