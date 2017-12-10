//////////////////////////////////////////////////////////////////////////
/*
	存储贝叶斯先验39个参数的类
	huangqiangsheng9@gmail.com

	不足之处在于
	1. 不具备通用性, 应该抽象成通用模块实现
	2. 缺少数据边界检查
									by cliffyin
*/
//////////////////////////////////////////////////////////////////////////

#ifndef BAYESPARAM_H_
#define BAYESPARAM_H_

//////////////////////////////////////////////////////////////////////////
// include files
#include <iostream>
#include <string>
#include <vector>
#include <map>
using namespace::std;

//////////////////////////////////////////////////////////////////////////
// define enumerator for match state
typedef enum {
	MS_ATTACK = 0,
	MS_DEADLOCK,
	MS_DEFEND,
} MatchStateT;

typedef MatchStateT ControlT;
typedef MatchStateT MeasureT;

//////////////////////////////////////////////////////////////////////////
// define the CBayesParam class for store bayes-filter parameters
class CBayesParam {
public:
	// constructor
	CBayesParam() {};

	// copy constructor
	CBayesParam(const CBayesParam& bayesparam);

	// destructor
	~CBayesParam() {};

	// get state transition probability (3*3*3)
	double GetPxux(int last_x, int u, int x) const;

	// get measurement probability (3*3)
	double GetPzx(int z, int x) const ;

	// get initial belief (3*1)
	double GetInit(int x) const;

	// set state transition probability (3*3*3)
	void InPxux(int last_x, int u, int x,double val);

	// set measurement probability (3*3)
	void InPzx(int z, int x,double val);

	// set initial belief (3*1)
	void InInit(int x,double val);

	// debug print (optional)
	void printDebug();

private:
	// matrix for state transition probability
	double Pxux[3][3][3];

	// matrix for measurement probability
	double Pzx[3][3];

	// matrix for initial belief
	double Bel[3];
};

#endif // ~BAYESPARAM_H_