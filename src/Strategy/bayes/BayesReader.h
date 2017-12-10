//////////////////////////////////////////////////////////////////////////
/*
��ȡ��Ҷ˹������
huangqiangsheng9@gmail.com
readOppBayesParam()��ȡĳ�����������39������
readBayesParam()��ȡһ���б�������������������1ֻ����
getBayesParam ���һֻ����Ĳ���
*/
//////////////////////////////////////////////////////////////////////////

#ifndef BAYESREADER_H_
#define BAYESREADER_H_

//////////////////////////////////////////////////////////////////////////
// include files
#include "BayesParam.h"
#include "singleton.h"
using namespace std;

//////////////////////////////////////////////////////////////////////////
// define the CBayesReader class for reading parameters
// from external files
class CBayesReader {
public:
	// constructor
	CBayesReader();

	// destructor
	~CBayesReader() {};

	// read in parameters from list file
	void readBayesParam(const string& opplist);

	// read in parameters from single external file
	void readOppBayesParam(const string& oppName);

	// get parameters with specified file
	const CBayesParam& getBayesParam(const string& oppName);

	// debug print
	void printBayesParam(const string& oppName);

private:
	// key-value structure to store parameters
	map<std::string, CBayesParam> _opp_list;

	// parameters external files' path
	static const string SCRIPT_PATH;
};

// singleton class
typedef NormalSingleton< CBayesReader > BayesReader; 

#endif // ~BAYESREADER_H_