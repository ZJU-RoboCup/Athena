//////////////////////////////////////////////////////////////////////////
/*
	贝叶斯滤波类
	huangqiangsheng9@gmail.com
	init()为初始换贝叶斯的39个先验参数
	filter()输入u和z的状态，输出x的状态

	不足之处在于
	1. 不具备通用性, 应该抽象成通用模块实现
	2. 缺少数据边界检查
									by cliffyin
*/
//////////////////////////////////////////////////////////////////////////

#ifndef BAYESFILTER_H_
#define BAYESFILTER_H_

//////////////////////////////////////////////////////////////////////////
// include files
#include "BayesParam.h"
#include "BayesReader.h"

//////////////////////////////////////////////////////////////////////////
// define the CBayesFilter class used to filter some state
class CBayesFilter 
{
public:
	// constructor
	CBayesFilter() {};

	// destructor
	~CBayesFilter() {};

	// initialize the parameters for the filter
	void init(const CBayesParam& bayesparam);

	// get distributed beliefs before filtering
	void getBeliefsBeforeFiltering(double bel_bar[]);

	// do filter with input current control and measurement
	int filter(int u, int z);

	// get distributed beliefs after filtering
	void getBeliefsAfterFiltering(double bel[]);

	// reset distributed beliefs with forced values
	void resetBeleifs(double reset_bel[]);
	
private:
	// previous belief distribution (before filtering)
	double _bel_bar[3];

	// current belief distribution (after filtering)
	double _bel[3];

	// parameters for the filter
	CBayesParam _filter_param;
};

#endif // ~BAYESFILTER_H_