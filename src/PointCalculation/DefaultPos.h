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
	@brief 外部取点接口 */
	CGeoPoint getDefaultPosbyIndex(int index);
	//index为0，1表示一个或两个进攻者时的默认站位点，index为3表示三个进攻者的默认站位点
	/** 
	@brief  生成站位点的函数，内部使用 */
	virtual CGeoPoint generatePos(const CVisionModule* pVision);
private:
	//传入-1代表最左侧，0代表中间，1代表最右侧
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