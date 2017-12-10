#ifndef __ATTRIBUTE_SET_H__
#define __ATTRIBUTE_SET_H__
//！！！注意：角色名字固有不变
#include <iostream>
#include <string>
#include <map>
#include "Attribute.h"
using namespace std;
typedef std::map<string,CAttribute*> AttributeMap;//！！！如果是CAttribute* 则需要写拷贝构造函数！！！
class CVisionModule;
//属性集合，包含一个车号。通过这个添加删除属性
class CAttributeSet
{
public:
	CAttributeSet();
	CAttributeSet(const CAttributeSet& attr);
	~CAttributeSet();
	
	void setNum(int num){_num = num;}
	int getNum(){return _num;}
	bool add(CAttribute* attr);
	bool remove(const string name);
	double getValue(const string name);

	void evaluate(const CVisionModule *pVision);
protected:
private:
	AttributeMap _attributeMap;	//属性集
	int _num;									//车号
};

#endif