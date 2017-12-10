#ifndef _ENEMY_SITUATION_H_
#define _ENEMY_SITUATION_H_

#include "geometry.h"
#include "param.h"
#include <vector>
using namespace std;
// 要实时更新的结构
typedef int oppNum;
typedef int ourNum;
typedef pair<oppNum,ourNum> _markPair;
typedef vector<_markPair> _markList;        //盯人表,包括持球人的盯防

typedef struct{ 		//动态形势下，对方离球最近的一个人的基本情况，我们这里称其为对手持球人
	int num;			//对手持球人的号码
	CGeoPoint pos;		//对手持球人的位置
	double dir;			//对手持球人的朝向
	bool ballTaked;		//对手持球人是否将球拿稳
	bool backPass;		//回传意图
	bool passMe;		//是否把我过了
	int Num(){return num;}
	CGeoPoint Pos(){return pos;}
	double Dir(){return dir;}
	bool isBallTaked(){return ballTaked;}
	bool isBackPass(){return backPass;}
	bool isPassMe(){return passMe;}
} _ballKeeper;

struct attackPotentialPair{ 
	attackPotentialPair(int n, double p) : num(n), potential(p){ }
	bool operator < (const attackPotentialPair& n) const { return potential < n.potential; }
	int num;			// 队员的号码
	double potential;	// 进攻潜在威胁的势能
};
typedef std::vector< attackPotentialPair > attackPlayerList;

class CEnemySituation{
public:
	CEnemySituation();
	~CEnemySituation(){};

	_ballKeeper ballKeeper; //对手持球人
	
	int receiver;			//对手最有可能接球的人
	double receiverDir;		//对手最有可能接球的人的朝向
	bool receiverExist;

	bool isMarked[Param::Field::MAX_PLAYER + 1];	//在盯人skill里面刷新，同时刷新markList  注意此时1~6对应对手1~6号车
	int markCycle[Param::Field::MAX_PLAYER + 1];
	_markList markList;

	int attackerNum;		//对手参与进攻的人数
	int attackArray[Param::Field::MAX_PLAYER];		//对手参与进攻的人的号码，*通过威胁度依次排列* 
	int steadyAttackArray[Param::Field::MAX_PLAYER];//变化平稳的attackArray
	//attackArray[0] = ballKeeper.num , attackArray[1] 是对手跑位的车中威胁度最大的点。
	//attackArray中前attackerNum-1个数字有意义
	//威胁度计算：（不依靠盯人车时）其射门角度（影响度小），
	//其最近的车的补防距离（影响度大），持球车对其的传球朝向（影响度中）	
	
	bool keeperChange;
	bool attackerNumChange;
	bool attackerChange;
	bool defSituationChange;

public:
	_ballKeeper getBallKeeper(){return ballKeeper;}
	int getReceiver(){return receiver;}
	double getReceiverDir(){return receiverDir;}
	bool isReceiverExist(){return receiverExist;}
	bool queryMarked(int i){return isMarked[i];}
	_markList getMarkList(){return markList;}
	int getAttackNum(){return attackerNum;}
	int getAttackEnemy(int i){return attackArray[i];}
	int getSteadyAttackEnemy(int i){return steadyAttackArray[i];}
	bool isKeeperchanged(){return keeperChange;}
	bool isAttackerNumChanged(){return attackerNumChange;}
	bool isAttackerChanged(){return attackerChange;}
	bool isSituationChanged(){return defSituationChange;}
};

#endif //_ENEMY_SITUATION_H_