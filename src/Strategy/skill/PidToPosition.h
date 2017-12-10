#ifndef __PID2POS__
#define __PID2POS__
#include <skill/PlayerTask.h>

class CPidToPosition :public CStatedTask
{
public:
	CPidToPosition(){}
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: Pid To Position\n"; }


private:
	
};

#endif //__GOALIE_2013_H__