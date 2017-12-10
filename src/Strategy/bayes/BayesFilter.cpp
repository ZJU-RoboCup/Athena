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

#include "BayesFilter.h"

// initialize the parameters for the filter
void CBayesFilter::init(const CBayesParam& bayesparam)
{
	// initial parameters
	_filter_param = CBayesParam(bayesparam);

	// initial belief distribution
	for (int iter = 0; iter<3; iter++)
	{
		_bel[iter] = _filter_param.GetInit(iter);
		_bel_bar[iter] = _bel[iter];
	}

	return ;
}

// get distributed beliefs before filtering
void CBayesFilter::getBeliefsBeforeFiltering(double bel_bar[])
{
	for (int i = 0; i < 3; i ++) {
		bel_bar[i] = _bel_bar[i];
	}

	return ;
}

// do filter with input current control and measurement
int CBayesFilter::filter(int u, int z)
{
	double prebel[3];
	// 第一步：预测，获取先验经验
	for (int iter = 0; iter < 3; iter ++) {
		// 可能存在问题，需要进一步检查 ！！！ cliffyin
		prebel[iter] = 0;
		for (int lastiter = 0; lastiter < 3; lastiter ++) {
			prebel[iter] = prebel[iter] + _filter_param.GetPxux(lastiter,u,iter) * _bel_bar[lastiter];
		} 
	}

	// 第二步：更新，根据观测计算
	for (int iter = 0; iter < 3; iter ++) {
		// 可能存在问题，需要进一步检查 ！！！ cliffyin
		_bel[iter] = _filter_param.GetPzx(z,iter) * prebel[iter];
	}

	// 第三步： 归一化
	double sumx = 0.0;
	for (int iter = 0; iter < 3; iter ++) {
		sumx += _bel[iter];
	}

	for (int iter = 0; iter < 3; iter ++) {
		_bel[iter] = _bel[iter] / sumx;
		_bel_bar[iter] = _bel[iter];
	}

	// 第四步：决定目前状态，需要去掉	cliffyin
	double largestx = _bel[0];
	int temp_state = 0;
	for (int iter = 1; iter < 3 ; iter ++) {
		if (_bel[iter] > largestx) {
			temp_state = iter;
			largestx = _bel[iter];
		}
	}

	return temp_state;	
}

// get distributed beliefs after filtering
void CBayesFilter::getBeliefsAfterFiltering(double bel[])
{
	for (int i = 0; i < 3; i ++) {
		bel[i] = _bel[i];
	}

	return ;
}

// reset distributed beliefs with forced values
void CBayesFilter::resetBeleifs(double reset_bel[])
{
	for (int i = 0; i < 3; i ++) {
		_bel[i] = reset_bel[i];
	}

	return ;
}