#ifndef _ENEMY_DEFEND_TACTIC_ANALYS_H_
#define _ENEMY_DEFEND_TACTIC_ANALYS_H_
/**
* Author: dxh
* Date: 2013.6.4
* Description: 用来做传球区域的划分和评价
* Email: dxiaohe@gmail.com
*/
#include "geometry.h"
#include <string>
#include "EnemyDefendTacticArea.h"
#include "singleton.h"
#include "param.h"
using namespace  std;
class CVisionModule;
#define MAX_AREAS 8

class CEnemyDefendTacticAnalys
{
public:
	CEnemyDefendTacticArea area[MAX_AREAS];
public:
	CEnemyDefendTacticAnalys();
	~CEnemyDefendTacticAnalys();
	void updateCurrentTacticInfo(const CVisionModule* pVision);
	string  doAnalys(const int enemyCarNum, const CVisionModule* pVision);
	string  doAnalys(string enemyRole, const CVisionModule* pVision);
	void addCricle(CGeoPoint & circleCenter,double radius,int  areaType,int ourCarNum);
	void addLongCircle(CGeoPoint& circleCenter,double radius,double jointLength,int Type);
	void addLongAnnulus(CGeoPoint & circleCenter,double interRadius,double outerRadius,double jointLength,int areaType);
	void setGoalieJudgeAreaSerialNum(int num);
	void setDefenderJudgeAreaSerialNum(int num);
private:
	int areaNum;
	int goalieAreaNum;
	int defenderAreaNum;
	int goalie[Param::Field::MAX_PLAYER];
	int goalieCounter;
	int defender[Param::Field::MAX_PLAYER];
	int defenderCounter;
	int marker[Param::Field::MAX_PLAYER];
	int markerCounter;
	int attacker[Param::Field::MAX_PLAYER];
	int attackerCounter;
	int blocker[Param::Field::MAX_PLAYER];
	int blockerCounter;
	int _lastCycle;
};
typedef NormalSingleton<CEnemyDefendTacticAnalys> EnemyDefendTacticAnalys;

#endif