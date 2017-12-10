#ifndef  _ADVANCE_BALL_V2_H_
#define _ADVANCE_BALL_V2_H_

#include <skill/PlayerTask.h>
/**********************************************************
* High Level Skill: Advance Ball
* Author: yys
* Created Date: 2014/07/11
***********************************************************/
class CAdvanceBallV2 : public CStatedTask {
public:
	CAdvanceBallV2();
	~CAdvanceBallV2();
	CAdvanceBallV2(const CGeoPoint& ballTarget);
	virtual void plan(const CVisionModule* pVision);
	virtual CPlayerCommand* execute(const CVisionModule * pVision);
	virtual bool isEmpty() const {return false;}
private:
	enum {
		GOTO = 1,
		DRIBBLE_PULL,
		INTERCEPT,
		NORMAL_PUSH,
		PRE_ROTATE,
		ROTATE,
		DRIBBLE_TURN,
		LIGHT_KICK,
		KICK_DRIBBLE,
		KICK,
		STOP
	};
	enum {
		ATTACK_TO_CENTER,
		ATTACK_TO_LEFT,
		ATTACK_TO_RIGHT
	};
	//位置判断函数//
		bool ballOnTheirHalf(const CVisionModule* pVision, const double buffer = -100);
		bool checkBallFront(const CVisionModule* pVision, double angle = Param::Math::PI / 3.0);
		bool checkBallBehind(const CVisionModule* pVision, double angle = Param::Math::PI / 3.0);
		bool checkBallFrontOrBehindBigPre(const CVisionModule* pVision);
		bool ballInTheirPenaltyArea(const CVisionModule* pVision, const int vecNumber);
		bool ballBetweentMeAndOpp(const CVisionModule* pVision, const int vecNumber);
		bool meNearballThanOpp(const CVisionModule* pVision, const int vecNumber);
		bool oppBetweenBallAndMe(const CVisionModule* pVision, const int vecNumber);
	//条件判断函数//
		bool ballMoving2Opp(const CVisionModule* pVision);
		bool ballMoving2Me(const CVisionModule* pVision, const int vecNumber);
		bool ballMoving2OurField(const CVisionModule* pVision, const int vecNumber);
		void decideChipOrShoot(const CVisionModule* pVision, double& kickDir, const int vecNumber);
		bool oppBlockMe(const CVisionModule* pVision, const int vecNumber);
		bool selectChipKick(const CVisionModule* pVision, int vecNumber);
		//checkOppHasBall没有考虑车朝向的问题
		bool checkOppHasBall(const CVisionModule* pVision);
		bool isShootBlocked(const CVisionModule* pVision, const int vecNumber);
		bool isMePassedOpp(const CVisionModule* pVision, const int vecNumber);
		bool isVisionHasBall(const CVisionModule* pVision, const int vecNumber);
		bool checkUsingVoilence(const CVisionModule* pVision, int vecNumber);
		bool checkTheyCanShoot(const CVisionModule* pVision, int vecNumber);
	//朝向判断函数//
		bool isOppFaceOurDoor(const CVisionModule* pVision, double angle = Param::Math::PI / 3.0);
		bool isOppFaceTheirDoor(const CVisionModule* pVision, const double angle = Param::Math::PI / 3.0);
		bool meFaceTheirDoor(const CVisionModule* pVision, const int vecNumber, const int angle);
		bool meFaceOpp(const CVisionModule* pVision, const int vecNumber);
		bool faceTheirGoal(const CVisionModule* pVision, const int executor);
	//动作函数//
		void voilenceActionTime(const CVisionModule* pVision, int vecNumber);
		void blockingOuterOppTime(const CVisionModule* pVision, int vecNumber);
		void blockingBestOppTime(const CVisionModule* pVision, int vecNumber);
	//计算函数//
		void calcPushTarget(const CVisionModule* pVision);
		void drawBestOpp(const CGeoPoint& oppPos);
		//true为有空档吸球转身射门，false为没有空档
		bool checkEnoughSpaceToTurnKick(const CVisionModule* pVision);
		bool isShootBlockedInPenaltyArea(const CVisionModule* pVision, const int vecNumber);
		int getTheirMostClosetoPosPlayerNum(const CVisionModule* pVision, CGeoPoint pos);
		double generateNormalPushDir(const CVisionModule* pVision, const int vecNumber);
	
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
	int meHasBall;
	int meLoseBall;
	int opponentID; //敌方最有威胁的车
	int attack_side;
	CGeoPoint _pushBallTarget;
	bool pullBackFlag;
	int normalPushCnt;
	int rotateCnt;
	int pre_rotateCnt;
	int kick_dribble;
	double dribbleTurnFinalDir;

protected:
	CPlayerCommand* _directCommand;
};
#endif