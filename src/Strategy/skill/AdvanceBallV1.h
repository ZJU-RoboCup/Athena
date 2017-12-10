#ifndef  _ADVANCE_BALL_V1_H_
#define _ADVANCE_BALL_V1_H_

#include <skill/PlayerTask.h>
#include "DefendUtils.h"
/**********************************************************
* High Level Skill: Advance Ball Up Field / Down Field
* Description: ͨ��������-->����-->���ţ����н������߽�������
* Author: dxh
* Created Date: 2013/4/24
***********************************************************/
class CAdvanceBallV1 : public  CStatedTask{

public:

	CAdvanceBallV1();
	~CAdvanceBallV1();
	CAdvanceBallV1(const CGeoPoint& ballTarget);
	virtual void plan(const CVisionModule* pVision);
	virtual CPlayerCommand* execute(const CVisionModule * pVision);
	virtual bool isEmpty()const{return false;}

private:

	enum{
		GOTO= 1,
		TURN,
		DRIBBLE_PULL,
		RUSH,
		NORMAL_PUSH,
		CRAZYPUSH,
		KICK,
		STOP
	};
	enum{
		ATTACK_TO_CENTER,
		ATTACK_TO_LEFT,
		ATTACK_TO_RIGHT
	};

	//λ���жϺ���//
		bool isBallInPushArea(const CVisionModule* pVision);
		bool ballOnTheirHalf(const CVisionModule* pVision);
		bool checkBallFront(const CVisionModule* pVision,double angle = Param::Math::PI / 3.0);
		bool checkBallBehind(const CVisionModule* pVision,double angle = Param::Math::PI / 3.0);
		bool checkBallFrontOrBehindBigPre(const CVisionModule* pVision);
		bool ballInTheirPenaltyArea(const CVisionModule* pVision, const int vecNumber);
		bool ballBetweentMeAndOpp(const CVisionModule* pVision,const int vecNumber);
	//�����жϺ���//
		bool ballMoving2Opp(const CVisionModule* pVision,const int vecNumber);
		void decideChipOrShoot(const CVisionModule* pVision,double& kickDir,const int vecNumver);
		bool oppBlockMe(const CVisionModule* pVision,const int vecNumber);
		//true:pull������ǰ�볡, false:push�������Լ��볡
		bool decidePushOrPull(const CVisionModule* pVision,int& flag); 
		bool selectChipKick(const CVisionModule* pVision,int vecNumber);
		//checkOppHasBallû�п��ǳ����������
		bool checkOppHasBall(const CVisionModule* pVision);
		bool isShootBlocked(const CVisionModule* pVision,const int vecNumber);
		bool isMePassedOpp(const CVisionModule* pVision,const int vecNumber);
		bool isVisionHasBall(const CVisionModule* pVision,const int vecNumber);
		bool checkUsingVoilence(const CVisionModule* pVision,int vecNumber);
	//�����жϺ���//
		bool isOppFaceOurDoor(const CVisionModule* pVision,double angle = Param::Math::PI / 3.0);
		bool isOppFaceTheirDoor(const CVisionModule* pVision,const double angle = Param::Math::PI / 3);
		bool meFaceTheirDoor(const CVisionModule* pVision,const int vecNumber);
		bool faceTheirGoal(const CVisionModule* pVision, const int executor);

	//��������//
		void voilenceActionTime(const CVisionModule* pVision,int vecNumber);
		void blockingOuterOppTime(const CVisionModule* pVision,int vecNumber);
		void blockingBestOppTime(const CVisionModule* pVision,int vecNumber);
	//���㺯��//
	void calcPushTarget(const CVisionModule* pVision);
	void drawBestOpp(const CGeoPoint& oppPos);
	
	//trueΪ�пյ�����ת�����ţ�falseΪû�пյ�
	bool checkEnoughSpaceToTurnKick(const CVisionModule* pVision);
	//tureΪ���ڶԷ�������

	//trueΪ�Ѿ����˶Է��ĳ�
	
	bool isShootBlockedInPenaltyArea(const CVisionModule* pVision,const int vecNumber);

	int getTheirMostClosetoPosPlayerNum(const CVisionModule* pVision, CGeoPoint pos);
	
	int pullCnt;
	double finalTurnDir;
	double pullBallDir;
	CGeoPoint rushPoint;
	double rushDir;
	int rushCnt;
	bool rushFlag;
	int turnCnt;
	CGeoPoint turnPos;
	int _lastCycle;
	int _state;
	int _cycle;
	int infraredOn;
	int infraredOff;
	int meHasBall;
	int meLoseBall;
	int opponentID;//�з�������в�ĳ�
	int dribble_last_state;
	bool isBallInPenalty;
	int attack_side;
	CGeoPoint _pushBallTarget;
	bool pullBackFlag;
	bool dribbleFlag ;

protected:
	virtual void toStream(std::ostream& os)const;
	CPlayerCommand* _directCommand;
};
#endif