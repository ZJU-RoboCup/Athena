#ifndef _GO_AND_TURNKICKV4_H_
#define _GO_AND_TURNKICKV4_H_
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: CGoAndTurnKickV3
* Description:ʹ��CGetBallV3�ķ���������ת����
***********************************************************/

class CGoAndTurnKickV4 :public CStatedTask {
public:
	CGoAndTurnKickV4();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: GoAndTurnKickV4\n"; }

private:
	int _lastCycle;
};

#endif //_GO_AND_TURNKICKV4_H_