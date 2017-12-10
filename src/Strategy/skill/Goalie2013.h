#ifndef __GOALIE_2013_H__
#define __GOALIE_2013_H__
#include <skill/PlayerTask.h>

class CGoalie2013 :public CStatedTask {
public:
	CGoalie2013();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: CGoalie2013\n"; }
	bool needtoClearBall(const CVisionModule* pVision);
	bool needtoAttackEnemy(const CVisionModule* pVision);
	bool checkWeHaveHelper(const CVisionModule* pVision);
private:
	bool _clearBall;
	bool _attackEnemy;
};

#endif //__GOALIE_2013_H__