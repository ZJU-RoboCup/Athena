#ifndef _BALL_ADVANCE_DECISION_H_
#define _BALL_ADVANCE_DECISION_H_

#include "singleton.h"
#include "VisionModule.h"
#include "WorldModel.h"

class CBallAdvanceDecision {
private:
	bool canShoot();
	bool canLightKick();
	bool canLightChip();
	bool canPass();
	bool isBallMovingToMe();
	bool canWaitTouch();
	void confirmLeader();
	int _state;
	int _leader;
	int _receiver;
	int _lastCycle;
	int _lastBallArea;
	int _stateCounter;
	int _bestPlayerCount;
	bool ballTransit;
	bool ballTransitRecord;
	const CVisionModule* _pVision;
	const void update(const CVisionModule* pVision, int meNum) {
		_leader = meNum;
		_pVision = pVision;
	}
	void generateRunPosition();
	void generateReceiveBallPosition();
	CGeoPoint runPointCompute(int number, vector <CGeoPoint> targetPoint);
	CGeoPoint receiveBallPointCompute(int number, vector <CGeoPoint> targetPoint);
	double robotTimeCompute(int vehicleNumber);
	struct AreaStatus {
		vector <CGeoRectangle> location;
		vector <CGeoPoint> targetPos;
		vector <bool> status;
		double lineAmount;
		double columnAmount;
	} area;
	const int State_Counter_Num = 5;
public:
	CBallAdvanceDecision::CBallAdvanceDecision();
	CBallAdvanceDecision::~CBallAdvanceDecision();
public:
	string generateAttackDecision(const CVisionModule* pVision, int meNum);
};

typedef NormalSingleton< CBallAdvanceDecision > BallAdvanceDecision;

#endif

