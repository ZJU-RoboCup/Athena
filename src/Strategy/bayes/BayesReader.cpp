//////////////////////////////////////////////////////////////////////////
/*
读取贝叶斯参数类
huangqiangsheng9@gmail.com
readOppBayesParam()读取某个队伍的先验39个参数
readBayesParam()读取一个列表的先验参数，包含至少1只队伍
getBayesParam 输出一只队伍的参数
*/
//////////////////////////////////////////////////////////////////////////

#include "BayesReader.h"
#include "math.h"
#include<fstream>
#include<sstream>

const string CBayesReader::SCRIPT_PATH = "params\\GameFilterParam\\"; 

//////////////////////////////////////////////////////////////////////////
// define some helper functions in default namespace
namespace {
	// 一些helper函数
	bool isBlankLine(const string& line) {
		bool blank = true;
		for (unsigned int i = 0; i < line.size(); i++) {
			if (line[i] != ' ' &&
				line[i] != '\n' &&
				line[i] != '\r' &&
				line[i] != '\t')
				blank = false;
		}
		return blank;
	}

	bool isCommentLine(const string& line) {
		if (line.size() < 1 ||
			line[0] == '#')
			return true;
		return false;
	}

	int string2state(string s)
	{//将字符串转化成相应的数值
		if (s == "ATTACK" ||
			s == "Attack" ||
			s == "attack")
		{
			return MS_ATTACK;
		}
		else if( s == "DEADLOCK" ||
			s == "deadlock" ||
			s == "deadlock")
		{
			return MS_DEADLOCK;
		}
		else if(s == "DEFEND" ||
			s == "defend" ||
			s == "Defend")
		{
			return MS_DEFEND;
		}
		else
		{
			cerr<< "ERROR IN BayesReader."<<endl;
			cerr<< "ERROR: "<<s<<endl;
		}
		return -1;
	}
};

// constructor
CBayesReader::CBayesReader()
{

}

// read in parameters from list file
void CBayesReader::readBayesParam(const std::string &opplist)
{
	string suffix = ".list";
	string fullName = SCRIPT_PATH + opplist+suffix;
	ifstream infile(fullName.c_str());
	if (!infile) {
		cerr << "Bayes Param Reading : error opening file " << opplist<<suffix << endl;
		//exit(-1);
	}
	//doCheck if is loaded before,avoid reload
	//TODO
	if ( _opp_list.count(opplist) > 1 ){
		cerr << "Reload "<<opplist<<suffix<<endl;
		return;
	}
	string line;
	string temp_opp;
	while (getline(infile,line)) {
		if (isBlankLine(line) || isCommentLine(line))
			continue;
		istringstream lineStream(line);
		lineStream >> temp_opp;
		readOppBayesParam(temp_opp);
	}
}

// read in parameters from single external file
void CBayesReader::readOppBayesParam(const string& oppName)
{
	string suffix = ".bayes";
	string fullName = SCRIPT_PATH + oppName+suffix;
	ifstream infile(fullName.c_str());
	if (!infile) {
		cerr << "error opening file " << oppName<<suffix << endl;
		exit(-1);
	}
	//doCheck if is loaded before,avoid reload
	//TODO
	if ( _opp_list.count(oppName) > 1 ){
		cerr << "Reload "<<oppName<<suffix<<endl;
		return;
	}
	string line;
	string temp_state;
	CBayesParam temp_bayes;
	while (getline(infile,line)) {
		if (isBlankLine(line) || isCommentLine(line))
			continue;
		istringstream lineStream(line);
		lineStream >> temp_state;
		if (temp_state == "PXUX" || temp_state == "pxux" ||temp_state == "Pxux")
		{
			int numstate = 0;
			while( numstate <9)
			{
				getline(infile,line);
				while (isBlankLine(line) || isCommentLine(line)) {	//  [3/19/2011 cliffyin]
					getline(infile,line);
				}
				istringstream lineStream2(line);
				string temp_gamestate1,temp_gamestate2;
				lineStream2>>temp_gamestate1>>temp_gamestate2;
				int last_x = string2state(temp_gamestate1);
				int u = string2state(temp_gamestate2);
				if (last_x == -1 || u == -1)
				{
					cerr << "ERROR IN BayesReader."<<endl;
					cerr<<oppName<<suffix<<" has error"<<endl;
					cerr<<"PXUX"<<" "<<temp_gamestate1<<" "<<temp_gamestate2<<endl;
					system("pause");
					exit(-1);
				}
				getline(infile,line);
				while (isBlankLine(line) || isCommentLine(line)) {	//  [3/19/2011 cliffyin]
					getline(infile,line);
				}
				istringstream lineStream3(line);
				double temp_val[3];
				lineStream3>>temp_val[0]>>temp_val[1]>>temp_val[2];
				temp_bayes.InPxux(last_x,u,MS_ATTACK,temp_val[0]);
				temp_bayes.InPxux(last_x,u,MS_DEADLOCK,temp_val[1]);
				temp_bayes.InPxux(last_x,u,MS_DEFEND,temp_val[2]);
				if (fabs(temp_val[0]+temp_val[1]+temp_val[2] - 1.0f)> 1e-5)
				{
					cout << "Error In BayesParam."<<endl;
					cout <<"FileName:"<<oppName<<endl;
					cout <<"STATE:" <<"Pxux:"<<temp_gamestate1<<" "<< temp_gamestate2<<endl;
				}
				numstate++;
			}
		}
		else if(temp_state == "PZX" || temp_state == "Pzx" || temp_state == "pzx")
		{
			int numstate = 0;
			while( numstate <3)
			{
				getline(infile,line);
				while (isBlankLine(line) || isCommentLine(line)) {	//  [3/19/2011 cliffyin]
					getline(infile,line);
				}
				istringstream lineStream2(line);
				string temp_gamestate;
				lineStream2>>temp_gamestate;
				int z = string2state(temp_gamestate);
				if (z == -1 )
				{
					cerr << "ERROR IN BayesReader."<<endl;
					cerr<<oppName<<suffix<<" has error!"<<endl;
					cerr<<"PZ"<<" "<<temp_gamestate<<endl;
					system("pause");
					exit(-1);
				}
				getline(infile,line);
				while (isBlankLine(line) || isCommentLine(line)) {	//  [3/19/2011 cliffyin]
					getline(infile,line);
				}
				istringstream lineStream3(line);
				double temp_val[3];
				lineStream3>>temp_val[0]>>temp_val[1]>>temp_val[2];
				temp_bayes.InPzx(z,MS_ATTACK,temp_val[0]);
				temp_bayes.InPzx(z,MS_DEADLOCK,temp_val[1]);
				temp_bayes.InPzx(z,MS_DEFEND,temp_val[2]);
				if (fabs(temp_val[0]+temp_val[1]+temp_val[2] -  1.0f) > 1e-5)
				{
					cout << "Error In BayesParam."<<endl;
					cout <<"FileName:"<<oppName<<endl;
					cout <<"STATE:" <<"Pzx:"<<temp_gamestate<<endl;
				}

				numstate++;
			}
		}
		else if (temp_state == "INIT" || temp_state == "Init" || temp_state == "init")
		{
			int numstate = 0;
			while( numstate <3)
			{
				getline(infile,line);
				while (isBlankLine(line) || isCommentLine(line)) {	//  [3/19/2011 cliffyin]
					getline(infile,line);
				}
				istringstream lineStream2(line);
				string temp_gamestate;
				lineStream2>>temp_gamestate;
				int x = string2state(temp_gamestate);
				if (x == -1 )
				{
					cerr << "ERROR IN BayesReader."<<endl;
					cerr<<oppName<<suffix<<" has error!"<<endl;
					cerr<<"PZ"<<" "<<temp_gamestate<<endl;
					system("pause");
					exit(-1);
				}
				getline(infile,line);
				while (isBlankLine(line) || isCommentLine(line)) {	//  [3/19/2011 cliffyin]
					getline(infile,line);
				}
				istringstream lineStream3(line);
				double temp_val;
				lineStream3>>temp_val;
				temp_bayes.InInit(x,temp_val);

				numstate++;
			}
			if (fabs(temp_bayes.GetInit(0) + temp_bayes.GetInit(1) + temp_bayes.GetInit(2) -1.0f)>1e-5)
			{
				cout << "Error In BayesParam."<<endl;
				cout <<"FileName:"<<oppName<<endl;
				cout <<"STATE:" <<"INIT"<<endl;
			}
		}
		else{
			cerr << "ERROR IN BayesReader."<<endl;
			cerr<<oppName<<suffix<<" has error!"<<endl;
			cerr<<temp_state<<endl;
			system("pause");
			exit(-1);
		}
		
	}
	_opp_list[oppName] = temp_bayes;
	return;
}

// get parameters with specified file
const CBayesParam& CBayesReader::getBayesParam(const std::string &oppName)
{
	return _opp_list[oppName];
}

// debug print
void  CBayesReader::printBayesParam(const string& oppName)
{
	cout<<oppName<<endl;
	_opp_list[oppName].printDebug();
}