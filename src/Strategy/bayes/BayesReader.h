//////////////////////////////////////////////////////////////////////////
/*
读取贝叶斯参数类
huangqiangsheng9@gmail.com
readOppBayesParam()读取某个队伍的先验39个参数
readBayesParam()读取一个列表的先验参数，包含至少1只队伍
getBayesParam 输出一只队伍的参数
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