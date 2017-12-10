#ifndef _DEFAULT_POS_
#define _DEFAULT_POS_

#include "AtomPos.h"
#include <VisionModule.h>

typedef struct{
	int defender;
	CGeoPoint pos;
	int side;
}Defender;

typedef vector<Defender> DefenderList;

class CDefaultPos :public CAtomPos
{
public:
	CDefaultPos();
	~CDefaultPos();

	/** 
	@brief �ⲿȡ��ӿ� */
	CGeoPoint getDefaultPosbyIndex(int index);
	//indexΪ0��1��ʾһ��������������ʱ��Ĭ��վλ�㣬indexΪ3��ʾ���������ߵ�Ĭ��վλ��
	/** 
	@brief  ����վλ��ĺ������ڲ�ʹ�� */
	virtual CGeoPoint generatePos(const CVisionModule* pVision);
private:
	//����-1��������࣬0�����м䣬1�������Ҳ�
	bool inCourt(int enemyNum, int side);
	void updateDefInfo();
	CGeoPoint _default[3];
	DefenderList _defenderList;
	vector<int> _leftDefList;
	vector<int> _rightDefList;
	vector<int> _middleDefList;

	int _lastCycle;
};

typedef NormalSingleton< CDefaultPos > DefaultPos;

#endif