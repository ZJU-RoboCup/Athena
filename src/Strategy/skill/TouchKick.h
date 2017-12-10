#ifndef __TOUCHKICK_h__
#define __TOUCHKICK_h__
#include <skill/PlayerTask.h>
#include "VisionModule.h"

/**********************************************************
* Skill: TochKick
* Description: ʹ�÷�װ��KickDirection��
			   TouchKickPos�����㳵����λ��
* Author: zhyaic
* E-mail: zhyaic@gmail.com
* Created Date: 2011.3.16
***********************************************************/

class CTouchKick : public CStatedTask{
public:
	CTouchKick();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual	CPlayerCommand *execute(const CVisionModule* pVision);

protected:
	virtual void toStream(std::ostream& os) const { os << "TouchKick\n"; }
private:
	bool _reach_pre_flag;
	int _lastCycle;
	int PreFrame;
};
#endif