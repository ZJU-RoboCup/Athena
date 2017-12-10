//////////////////////////////////////////////////////////////////////////
/*
��Ҷ˹�˲���
huangqiangsheng9@gmail.com
init()Ϊ��ʼ����Ҷ˹��39���������
filter()����u��z��״̬�����x��״̬

����֮������
1. ���߱�ͨ����, Ӧ�ó����ͨ��ģ��ʵ��
2. ȱ�����ݱ߽���
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
	// ��һ����Ԥ�⣬��ȡ���龭��
	for (int iter = 0; iter < 3; iter ++) {
		// ���ܴ������⣬��Ҫ��һ����� ������ cliffyin
		prebel[iter] = 0;
		for (int lastiter = 0; lastiter < 3; lastiter ++) {
			prebel[iter] = prebel[iter] + _filter_param.GetPxux(lastiter,u,iter) * _bel_bar[lastiter];
		} 
	}

	// �ڶ��������£����ݹ۲����
	for (int iter = 0; iter < 3; iter ++) {
		// ���ܴ������⣬��Ҫ��һ����� ������ cliffyin
		_bel[iter] = _filter_param.GetPzx(z,iter) * prebel[iter];
	}

	// �������� ��һ��
	double sumx = 0.0;
	for (int iter = 0; iter < 3; iter ++) {
		sumx += _bel[iter];
	}

	for (int iter = 0; iter < 3; iter ++) {
		_bel[iter] = _bel[iter] / sumx;
		_bel_bar[iter] = _bel[iter];
	}

	// ���Ĳ�������Ŀǰ״̬����Ҫȥ��	cliffyin
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