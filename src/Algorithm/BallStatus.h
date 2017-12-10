#ifndef _BALL_STATUS_H_
#define _BALL_STATUS_H_

#include "singleton.h"
#include "VisionModule.h"
#include "ContactChecker.h"

/************************************************************************/
/*				��Ĳ�����״̬���ݵĶ�д����                            */
/* ����:																*/
/* 1. ��������Ϣ��	ͼ��ģ�飨VisionModule)  +  ˫��ͨѶ��RobotSensor)	*/
/* 2. ��ʷ������Ϣ������״̬��KickStatus�� + ����״̬��DribbleStatus)	*/
/*																		*/
/* �����																*/
/* 1.�ײ���Ϣ�� �Ƿ����߳����ķ��ĸ���Ա��							*/
/*				�Ƿ��򱻿�ס���ķ��ĸ���Ա��							*/
/* 2.�߲���Ϣ�� �������												*/
/************************************************************************/

#define MAX_CMD_STORED 10

/// �ķ�����
enum emWhoTouchedBall{
	WTB_Us = 1,			// �ҷ�
	WTB_Them = 2,		// �Է�
	WTB_Unknown = 3		// ����ȷ��
};

enum BallState{
	None,
	OurBall,
	StandOff,
	WaitAdvance,
	GiveUpAdvance,
};


class CSendCmd{
public:
	CSendCmd() : _number(0), _normalKick(0), _chipKick(0), _dribble(0) {}
	void setKickCmd(int num, int normalKick, int chipKick, unsigned char dribble) {
		_number = num;
		_normalKick = normalKick;
		_chipKick = chipKick;
		_dribble = dribble;
	}
	void clear(void){ _number=0; _normalKick=0; _chipKick=0; _dribble=0; }

	int num() { return _number; }
	double normalKick() { return _normalKick; }
	double chipKick() { return _chipKick; }
	unsigned char dribble() { return _dribble; }

private:
	int _number;			// ����
	double _normalKick;		// ƽ���߳�����
	double _chipKick;		// �����߳�����
	unsigned char _dribble; // ������

};

/// ��״̬��װ��
class CBallStatus{
public:
	CBallStatus(void);
	~CBallStatus(void){}
	
public:
	// ������ܸ��½ӿڣ���Ҫÿ�����ڱ�����һ��
	void UpdateBallStatus(const CVisionModule* pVision);

	// ��ȡ���˶��ٶ�
	CVector ballMovingVel(void) { return _ballMovingVel; }

	CGeoLine ballChipLine(void) { return _ballChipLine; }

	// ���߳�,�㷵��true(����������,Ҳ������ƽ��)
	const bool IsBallKickedOut(void) const { return _isKickedOut; }

	// ��ĳ����ɫ�߳�,�㷵��true(����������,Ҳ������ƽ��)
	const bool IsBallKickedOut(int num) const { return _isKickedOut && num == _kickerNum;}; 

	// ���߳�,��������ָ���߳�,�ŷ���true
	const bool IsChipKickedOut(void) const { return _isChipKickOut && _isKickedOut; }

	// ��ĳ����ɫ�߳�,��������ָ���߳�,�ŷ���true;
	const bool IsChipKickedOut(int num) const { return IsChipKickedOut() && num == _kickerNum; } 
	// �õ������Ա�ĺ���
	int getKickerNum() { return _kickerNum; }

	int getBallToucher(){
		return _ballToucher;
	}
	int getLastBallToucher() {
		return _lastBallToucher;
	}
	bool ballToucherChanged(){
		return _ballToucher!=_lastBallToucher;
	}

	
private:
	// ��״̬�����µ�����
	int _ballStatusCycle;

	// �������˶��ٶ�
	void UpdateBallMoving(const CVisionModule* pVision);
	void CheckKickOutBall(const CVisionModule* pVision);

	CVector _ballMovingVel;
	CGeoLine _ballChipLine;

	// �����ķ����������
	bool _isKickedOut;
	int _kickerNum;
	bool _isChipKickOut;
	int _ballToucher;
	int _lastBallToucher;
	int _chipCycle;

	ContactChecker _contactChecker;

	// �Ƚ�PlayInterface����Ϣ����BallStatus��
public:
	string checkBallState(const CVisionModule* pVision,int meNum=0);
	void clearBallStateCouter();
	void setChipKickState(bool b_set){ _chipkickstate = b_set;}
	bool getChipKickState()const { return _chipkickstate; }
	void setCommand(CSendCmd kickCmd, int cycle);
	void setCommand(int num, int normalKick, int chipKick, unsigned char dribble, int cycle);
	CSendCmd getKickCommand(int num, int cycle) { return _kickCmd[num][cycle % MAX_CMD_STORED]; }
	void clearKickCmd(int num, int cycle){ _kickCmd[num][cycle % MAX_CMD_STORED].clear(); }
	void clearKickCmd(); // ������г�ָ��
	void initializeCmdStored();
private:
	CSendCmd _kickCmd[Param::Field::MAX_PLAYER+1][MAX_CMD_STORED];
	bool _chipkickstate; // trueΪ���ڿ���,falseΪ�Ѿ����
	int _ballState;
	int _ballStateCouter;
};

typedef NormalSingleton< CBallStatus > BallStatus;

#endif