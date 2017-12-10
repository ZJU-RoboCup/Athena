#ifndef _ENEMY_SITUATION_H_
#define _ENEMY_SITUATION_H_

#include "geometry.h"
#include "param.h"
#include <vector>
using namespace std;
// Ҫʵʱ���µĽṹ
typedef int oppNum;
typedef int ourNum;
typedef pair<oppNum,ourNum> _markPair;
typedef vector<_markPair> _markList;        //���˱�,���������˵Ķ���

typedef struct{ 		//��̬�����£��Է����������һ���˵Ļ�������������������Ϊ���ֳ�����
	int num;			//���ֳ����˵ĺ���
	CGeoPoint pos;		//���ֳ����˵�λ��
	double dir;			//���ֳ����˵ĳ���
	bool ballTaked;		//���ֳ������Ƿ�������
	bool backPass;		//�ش���ͼ
	bool passMe;		//�Ƿ���ҹ���
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
	int num;			// ��Ա�ĺ���
	double potential;	// ����Ǳ����в������
};
typedef std::vector< attackPotentialPair > attackPlayerList;

class CEnemySituation{
public:
	CEnemySituation();
	~CEnemySituation(){};

	_ballKeeper ballKeeper; //���ֳ�����
	
	int receiver;			//�������п��ܽ������
	double receiverDir;		//�������п��ܽ�����˵ĳ���
	bool receiverExist;

	bool isMarked[Param::Field::MAX_PLAYER + 1];	//�ڶ���skill����ˢ�£�ͬʱˢ��markList  ע���ʱ1~6��Ӧ����1~6�ų�
	int markCycle[Param::Field::MAX_PLAYER + 1];
	_markList markList;

	int attackerNum;		//���ֲ������������
	int attackArray[Param::Field::MAX_PLAYER];		//���ֲ���������˵ĺ��룬*ͨ����в����������* 
	int steadyAttackArray[Param::Field::MAX_PLAYER];//�仯ƽ�ȵ�attackArray
	//attackArray[0] = ballKeeper.num , attackArray[1] �Ƕ�����λ�ĳ�����в�����ĵ㡣
	//attackArray��ǰattackerNum-1������������
	//��в�ȼ��㣺�����������˳�ʱ�������ŽǶȣ�Ӱ���С����
	//������ĳ��Ĳ������루Ӱ��ȴ󣩣����򳵶���Ĵ�����Ӱ����У�	
	
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