#ifndef __ATTRIBUTE_H__
#define __ATTRIBUTE_H__
#include <iostream>
using namespace std;
class CVisionModule;
//��������� �����������۷���������ֵ�����Ե�����ֻ���ڳ�ʼ��ʱָ��
class CAttribute
{
public:
	CAttribute():_value(0){}
	CAttribute(const string name):_name(name),_value(0){}
	~CAttribute(){}

	//���۵Ľӿ�,num�Ƕ��ֳ��ĳ���
	virtual void evaluate(const CVisionModule* pVision,const int num) = 0;

	//��ȡ����ֵ
	double getValue(){return _value;}
	string getName(){return _name;}

protected:
	void setValue(double theValue){_value = theValue;}

private:
	double _value;
	string _name;
};

#endif