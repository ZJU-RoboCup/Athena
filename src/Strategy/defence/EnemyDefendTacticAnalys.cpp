#include "EnemyDefendTacticAnalys.h"
#include <string>
#include <VisionModule.h>
#include "GDebugEngine.h"
using namespace std;
namespace{
	double JudgeDist = Param::Vehicle::V2::PLAYER_SIZE * 5;
}
CEnemyDefendTacticAnalys::CEnemyDefendTacticAnalys()
{
	for (int i = 0 ; i < Param::Field::MAX_PLAYER ; i++)
	{
		goalie[i]  = 0;
		defender[i] = 0;
		marker[i] = 0;
		attacker[i] = 0;
	}
	areaNum = 0;
	goalieAreaNum = 99;
	defenderAreaNum =99;
	goalieCounter = 0;
	defenderCounter = 0;
	markerCounter = 0;
	attackerCounter = 0;
	_lastCycle = 0;
}
CEnemyDefendTacticAnalys::~CEnemyDefendTacticAnalys()
{

}

void CEnemyDefendTacticAnalys::updateCurrentTacticInfo(const CVisionModule* pVision)
{
	////添加队员判断规则区域
	
	if(pVision->Cycle() != _lastCycle){
		areaNum = 0;
		for (int i = 1;i <(Param::Field::MAX_PLAYER+1);i ++ )
		{
			if (pVision->OurPlayer(i).Valid())
			{
				CGeoPoint ourPlayerPos = pVision->OurPlayer(i).Pos();
				addCricle(ourPlayerPos,JudgeDist,CIRCLE,i);
				GDebugEngine::Instance()->gui_debug_arc(ourPlayerPos,JudgeDist,0,360);
			}
			
		}
		////添加对方守门员判断区域
		CGeoPoint theirGoal = CGeoPoint(Param::Field::PITCH_LENGTH / 2.0,0);
		double jointLength = Param::Field::PENALTY_AREA_L;
		double penaltyDepth = Param::Field::PENALTY_AREA_DEPTH;
		addLongCircle(theirGoal,penaltyDepth,jointLength,LONGCIRCLE);
		////添加对方后卫判断区域
		double outerRadius = penaltyDepth + 5*Param::Vehicle::V2::PLAYER_SIZE;
		addLongAnnulus(theirGoal,penaltyDepth,outerRadius,jointLength,ANNULUS);
	}
}

void CEnemyDefendTacticAnalys::addCricle(CGeoPoint & circleCenter,double radius,int areaType,int ourCarNum)
{
	if (areaNum > MAX_AREAS)
	{
		return;
	}
	area[areaNum].ourCarNum = ourCarNum;
	area[areaNum].circleCenter = circleCenter;
	area[areaNum].interRadius = 0;
	area[areaNum].outerRadius = radius;
	area[areaNum].jointLength =0;
	area[areaNum].mask = 1;
	area[areaNum].areaShape = areaType;
	areaNum ++ ;
}
void CEnemyDefendTacticAnalys::addLongCircle(CGeoPoint& circleCenter,double radius,double jointLength,int areaType)
{
	if (areaNum > MAX_AREAS)
	{
		return;
	}
	area[areaNum].ourCarNum = 0;
	area[areaNum].circleCenter = circleCenter;
	area[areaNum].interRadius = 0;
	area[areaNum].outerRadius = radius;
	area[areaNum].jointLength =jointLength;
	area[areaNum].mask = 1;
	area[areaNum].areaShape = areaType;
	setGoalieJudgeAreaSerialNum(areaNum);
	areaNum ++ ;
}

void CEnemyDefendTacticAnalys::addLongAnnulus(CGeoPoint & circleCenter,double interRadius,double outerRadius,double jointLength,int areaType)
{
	if (areaNum > MAX_AREAS)
	{
		return;
	}
	area[areaNum].ourCarNum = 0;
	area[areaNum].circleCenter = circleCenter;
	area[areaNum].interRadius = interRadius;
	area[areaNum].outerRadius = outerRadius;
	area[areaNum].jointLength =jointLength;
	area[areaNum].mask = 1;
	area[areaNum].areaShape = areaType;
	setDefenderJudgeAreaSerialNum(areaNum);
	areaNum ++ ;
}
string CEnemyDefendTacticAnalys::doAnalys(const int enemyCarNum,const CVisionModule * pVision)
{
	updateCurrentTacticInfo(pVision);
	const PlayerVisionT& enemy = pVision->TheirPlayer(enemyCarNum);
	CGeoPoint enemyPos = enemy.Pos();
	if (area[goalieAreaNum].isInLongCircle(enemyPos))
	{
		return "goalie";
	}
	if (area[defenderAreaNum].isInAnnulus(enemyPos))
	{
		return "defender"; 
	}
	int tempCounter = 0;
	double recordDsit = 999;
	for (int i = 0;i < MAX_AREAS;i ++)
	{
		if ((i != goalieAreaNum) && (i!= defenderAreaNum)&& area[i].mask == 1)
		{
			double me2oppDist = (pVision->OurPlayer(area[i].ourCarNum).Pos() - pVision->TheirPlayer(enemyCarNum).Pos()).mod();
			if (me2oppDist < recordDsit)
			{
				recordDsit = me2oppDist;
				tempCounter = i;
			}
		}
	}

	if (area[tempCounter].isInCircleArea(enemyPos) && area[tempCounter].isMarkingOurCar(enemyCarNum,pVision))
	{
		return "marker";
	}else
	{
		for (int i = 0;i < MAX_AREAS;i ++)
		{
			if ((i != goalieAreaNum) && (i!= defenderAreaNum)&& area[i].mask == 1)
			{
				if (area[i].isOnBallHalf(enemyCarNum,pVision))
				{
					return "blocker";
				}
			}
		}
	}

	return "attacker";

}

string CEnemyDefendTacticAnalys::doAnalys( string  enemyRole,const CVisionModule * pVision)
{
	updateCurrentTacticInfo(pVision);
	for (int i = 1 ;i  <=Param::Field::MAX_PLAYER ;i ++)
	{
		string currentEnemyRole = doAnalys(i,pVision);
		if (currentEnemyRole == "goalie")
		{
			goalie[goalieCounter] = i;
			goalieCounter ++;
		}else if (currentEnemyRole == "defender")
		{
			defender[defenderCounter] = i;
			defenderCounter ++;
		}else if (currentEnemyRole == "marker")
		{
			marker[markerCounter] = i;
			markerCounter ++;
		}else if (currentEnemyRole == "attacker")
		{
			attacker[attackerCounter] = i;
			attackerCounter++;
		}else if (currentEnemyRole == "blocker")
		{
			blocker[blockerCounter] = i;
			blockerCounter++;
		}
	}
	char buf[256];
	if ( enemyRole == "goalie" )
	{
		char tempStr[100] = "[";
		for (int i = 0; i < goalieCounter ;i ++)
		{
			sprintf(buf,"%d",goalie[i]);
			strcat(tempStr,buf);
		}
		strcat(tempStr,"]");
		return tempStr;
	}else if (enemyRole == "defender")
	{
		char tempStr[100] = "[";
		for (int i = 0; i < defenderCounter ;i ++)
		{
			sprintf(buf,"%d",defender[i]);
			strcat(tempStr,buf);
		}
		strcat(tempStr,"]");
		return tempStr;
	}else if (enemyRole == "Marker")
	{
		char tempStr[100] = "[";
		for (int i = 0; i < markerCounter ;i ++)
		{
			sprintf(buf,"%d",marker[i]);
			strcat(tempStr,buf);
		}
		strcat(tempStr,"]");
		return tempStr;
	}else if(enemyRole == "attacker")
	{
		char tempStr[100] = "[";
		for (int i = 0; i < attackerCounter ;i ++)
		{
			sprintf(buf,"%d",attacker[i]);
			strcat(tempStr,buf);
		}
		strcat(tempStr,"]");
		return tempStr;
	}else if (enemyRole == "blocker")
	{
		char tempStr[100] = "[";
		for (int i = 0; i < blockerCounter ;i ++)
		{
			sprintf(buf,"%d",blocker[i]);
			strcat(tempStr,buf);
		}
		strcat(tempStr,"]");
		return tempStr;
	}
	else
	{
		return "input enemy role error!";
	}
}

void CEnemyDefendTacticAnalys::setDefenderJudgeAreaSerialNum(int num)
{
	defenderAreaNum = num;
}

void CEnemyDefendTacticAnalys::setGoalieJudgeAreaSerialNum(int num)
{
	goalieAreaNum = num;
}