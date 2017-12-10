#ifndef _SUPPORT_2015_H_
#define _SUPPORT_2015_H_
#include "geometry.h"
#include "singleton.h"

/**********************************************************
* SupportPos
* Description: 助攻车跑点，优先考虑补刀，后考虑接传球
* 目前的设计，仅支持补射跑位，后续可考虑加入抢球补位
* Author: FantasyChen
* Created Date: 2015.6.25
***********************************************************/


class CVisionModule;

class CSupportPos {
public:
	CSupportPos();
	~CSupportPos() {}

	void generatePos(const CVisionModule* pVision,int num,int num2 = 0);
	CGeoPoint getSupPos(const CVisionModule* pVision,int num);
	CGeoPoint getSupPosNew(const CVisionModule* pVision,int num1,int num2);
	CGeoPoint getSupPosDirectly() { return _supportPos; }
	int getSupNum(){return ourSupporterNum;}
	void setPassed(bool flag){advancePassed = flag;}
	void setNewPoint(CGeoPoint a) { _supportPos = a; }
private:
	int _lastCycle;
	int _defenderList[10];
	CGeoPoint reflectPos;
	double reflectDir;
	bool isBallInOurControl(const CVisionModule* pVision);
	CGeoPoint _supportPos;
	CGeoPoint _lastPos;
	int calcEnemyDefenderAmount(const CVisionModule* pVision);
	double calcBallReflectDirAndPos(const CVisionModule* pVision, const int enemyDefenderAmount);
	double calcBallReflectPower(const CVisionModule *pVision, CGeoPoint fisrtPos, CGeoPoint secondPos);
	CGeoCirlce calcTheBestZone(const CVisionModule *pVision, const double power);
	CGeoPoint calcTheBestPoint(const CVisionModule *pVision, const CGeoCirlce bestZone);
	void checkAvoidShootLine(const CVisionModule *pVision);
	void keepLeastDistanceFromLeader(const CVisionModule *pVision,double leastDist);
	void checkAvoidCenterLine(const CVisionModule *pVision, double radius,double buffer);
	int ourSupporterNum;
	int ourLeaderNum;
	int _leaderChangeCounter;
	int _supporterChangeCounter;
	int _roleChangeCounter;
	int _savedCycle;
	bool _isRoleChanged;
	bool advancePassed;
};

typedef NormalSingleton<CSupportPos> SupportPos;

#endif 
