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