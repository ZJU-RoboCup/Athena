#ifndef __OPP_PLAYER_H__
#define __OPP_PLAYER_H__
//对手队员
#include <iostream>
#include <fstream>
#include <string>
#include "OppRoleMatcher.h"
using namespace std;

namespace Debug{
	bool isWrite = true;
	fstream File;
}

class CVisionModule;
class CAttributeSet;
class COppRole;

class COppPlayer
{
public:
	COppPlayer();
	~COppPlayer();
	//initialize,参数attrSet为模板
	void setProperty(const int num,const CAttributeSet& attrSet);//对每一个player应用该函数，可批量初始化或者动态更换属性集

	//calc function
	void evaluate(const CVisionModule* pVision);		//属性计算
	void matchRole();			//匹配角色,要先调用calcThreaten函数

	//return parameters
	int getNum(){return _num;}
	CAttributeSet* getAttr() {return _attrSetP;}
	int getThreatenPri();
	double getThreatenValue();
	string getRoleName();
	double getAttributeValue(string name);
	//skill call
	bool isTheRole(const string name);

	friend COppRole* COppRoleMatcher::matchRole(COppPlayer&);
	//friend class COppRoleMatcher;
private:
	int _num;							//车号
	CAttributeSet* _attrSetP;	//属性集,用于计算威胁度、匹配角色
	COppRole* _oppRole;		//角色，用于选择应对方法
};




#endif