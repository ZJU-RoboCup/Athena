#ifndef __TANDEM_H__
#define __TANDEM_H__
#include <skill/PlayerTask.h>

/**********************************************************
* Skill: Tandem Defend avoiding opponent advance
* Description: 通过在争球的时候起协防作用，防止对方快速突破
* Author: zhanfei
* Created Date: 2011-06-28
***********************************************************/

class CTandem : public CStatedTask
{
public:
	CTandem();
	~CTandem();
	virtual void plan(const CVisionModule* pVision);
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
	virtual bool isEmpty() const { return false; }

	enum BallSituation{
		Special = 1, // 双方争抢中
		BallMoving2TheirHalf,
		BallMoving2OurHalf,
		OurChance,
		TheirChance
	};
private:
	void analyzSituation(const CVisionModule* pVision, int teammate_id, int opp_id);
	int checkThreatonOpp(const CVisionModule* pVision);

	void planAssistAttack(const CVisionModule* pVision, int teammate_id);
	void planAssistGetBall(const CVisionModule* pVision, int teammate_id, int opp_id);
	void planBlock(const CVisionModule* pVision, int teammate_id, int opp_id);

	CPlayerCommand* _directCommand;
	int _lastCycle;
	int _tandem_role; // 配协助的角色
	bool _init;
	bool _called_for_attack;
	CGeoPoint _myPosition;
};

class TandemStatus{
public:
	enum{
		UnInit = 0,
		StandByOk = 1,
		TandemBlocking,
		TandemGetBall,
		ReadyForPickup
	};
	~TandemStatus(){}
	static TandemStatus* Instance();
	int getTandemAgentStatus()const { return _currentStatus; }
	void setTandemAgentStatus(int status){ _currentStatus = status; }
	const CGeoPoint& getStandByPosition()const { return _pos; }
	void setStandByPosition(const CGeoPoint& standby ){ _pos = standby; }
	void callForHelp(const CGeoPoint& helpPos);
	bool getHelpPos(CGeoPoint& helpPos);
private:
	TandemStatus(){ _currentStatus = UnInit; }
	int _currentStatus;
	CGeoPoint _pos;
	CGeoPoint _helpPos;
	bool is_helping;
};

#endif