#ifndef __ATTRIBUTE_H__
#define __ATTRIBUTE_H__
#include <iostream>
using namespace std;
class CVisionModule;
//抽象的属性 包含属性评价方法、属性值，属性的名称只能在初始化时指定
class CAttribute
{
public:
	CAttribute():_value(0){}
	CAttribute(const string name):_name(name),_value(0){}
	~CAttribute(){}

	//评价的接口,num是对手车的车号
	virtual void evaluate(const CVisionModule* pVision,const int num) = 0;

	//获取评价值
	double getValue(){return _value;}
	string getName(){return _name;}

protected:
	void setValue(double theValue){_value = theValue;}

private:
	double _value;
	string _name;
};

#endif