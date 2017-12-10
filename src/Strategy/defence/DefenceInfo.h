#ifndef __DEFNCE_INFO_H__
#define __DEFNCE_INFO_H__

#include "singleton.h"
#include <vector>
#include <iostream>
#include "param.h"
#include "OppPlayer.h"
#include "geometry.h"
using namespace std;
// 要实时更新的结构
typedef int oppNum;
typedef int ourNum;
typedef pair<oppNum,ourNum> MarkPair;
typedef vector<MarkPair> MarkList;        //盯人表,包括持球人的盯防

class CVisionModule;
class CAttributeSet;
class CTrigger;

struct MarkField{
	MarkField(const CGeoPoint& upLeft,const CGeoPoint& downRight){_upLeft = upLeft; _downRight = downRight;}
	CGeoPoint _upLeft;
	CGeoPoint _downRight;
	int markTouchNum;//我方调用markTouch的车
};

struct AttackerStruct{ 
	AttackerStruct(int n, int pri,double subV) : num(n), prioirty(pri),subValue(subV){}
	bool operator < (const AttackerStruct& n) const {
		if (prioirty < n.prioirty)//优先级越小越好,排在前面
		{
			return true;
		} else if (n.prioirty == prioirty)
		{
			return subValue > n.subValue;//角色隶属度越大越好，排在前面
		}
		return false;
	}
	int num;			// 队员的号码
	int prioirty;	// 优先级
	double subValue;//优先级下的值
};
typedef std::vector< AttackerStruct > AttackerList;

class CDefenceInfo
{
public: 
	CDefenceInfo();
	~CDefenceInfo();
	void initialization();
	void updateDefenceInfo(const CVisionModule *pVision);

	void setNoChangeFlag();
	void clearNoChangeFlag();
	
	void setNoMarkingField(CGeoPoint upLeft, CGeoPoint downRight);
	void clearNoMarkingField(CGeoPoint upLeft, CGeoPoint downRight);
	void clearAll();
	void changeAttrSet(CAttributeSet& as);
	//开球区域剔除挡位车
	void setNoMarkingNum(int enemyNum);
	void resetMarkingInfo(){_normalNoMarkList.clear();}

public:
	int getAttackNum();
	int getAttackOppNumByPri(int i);
	int getSteadyAttackOppNumByPri(int i);
	bool queryMarked(int i){return _isMarked[i];}
	int getOurMarkDenfender(const int enemyNum);	
	bool getMarkMode(const int enemy){return _markMode[enemy];}
	bool getTriggerState(){return _triggerOccur;}
	COppPlayer* getOppPlayerByNum(int i){return _oplayer[i];}
	bool getBallTaken(){return _ballTaken;}
	vector<MarkField> getMarkFieldList(){return _fieldList;}
	bool checkInRecArea(int enemyNum, const CVisionModule* pVision,MarkField markField);
	bool checkOurInRecArea(int myNum, const CVisionModule* pVision,MarkField markField);
	int getMarkingTouchNum(CGeoPoint upLeft,CGeoPoint downRight);

public:
	//反馈接口
	void setMarkList(const CVisionModule* pVision,int myNum,int enemyNum);
	void setMarkingTouchInfo(const CVisionModule* pVision,int myNum,CGeoPoint upLeft,CGeoPoint rightDown);
	void setMarkMode(int myNum,int enemyNum,bool mode);
	void setMarkFrontNum(int num);
	int getMarkFrontNum();
private:
	void updateAttackArray(const CVisionModule* pVision);
	void updateMarkPair(const CVisionModule* pVision);
	void updateSteadyAttackArray();	
	void updateBallTaken(const CVisionModule* pVision);//对方bestPlayer是否持球
	void checkSteadyAttack(const CVisionModule* pVision, AttackerList attackerList);
	int checkFieldAttack(MarkField markField, AttackerList attackerList);
	bool checkInAllField(int enemyNum);
	//bool checkInAllField(int enemyNum);

private:
	int _lastCycle;
	int _markCycle[Param::Field::MAX_PLAYER + 1];
	MarkList _markList;
	bool _isMarked[Param::Field::MAX_PLAYER + 1];
	bool _markMode[Param::Field::MAX_PLAYER+ 1];//盯人模式，普通盯人还是前盯

	int _attackerNum;									//对手参与进攻的人数,根据威胁度计算，而不是根据单单一条x方向阈值
	int _attackArray[Param::Field::MAX_PLAYER];			//对手参与进攻的人的号码，*通过威胁度依次排列* 
	int _steadyAttackArray[Param::Field::MAX_PLAYER];	//变化平稳的attackArray
	
	COppPlayer* _oplayer[Param::Field::MAX_PLAYER + 1];
	vector<CTrigger*> _trigger;
	bool _triggerOccur;//状态量
	bool _ballTaken;

	vector<MarkField> _fieldList;
	vector<int> _noMarkList;
	CGeoPoint _upLeft;
	CGeoPoint _downRight;

	bool _noChangeFlag;
	COppPlayer* _noChangeOplayer[Param::Field::MAX_PLAYER + 1];
	int _noChangeAttackArray[Param::Field::MAX_PLAYER];			
	int _noChangeSteadyAttackArray[Param::Field::MAX_PLAYER];
	int _noChangeAttackNum;

	vector<int> _normalNoMarkList;

	int _markingFrontNum;

};

typedef NormalSingleton<CDefenceInfo> DefenceInfo;

#endif