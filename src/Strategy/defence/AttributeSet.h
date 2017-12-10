#ifndef __ATTRIBUTE_SET_H__
#define __ATTRIBUTE_SET_H__
//������ע�⣺��ɫ���ֹ��в���
#include <iostream>
#include <string>
#include <map>
#include "Attribute.h"
using namespace std;
typedef std::map<string,CAttribute*> AttributeMap;//�����������CAttribute* ����Ҫд�������캯��������
class CVisionModule;
//���Լ��ϣ�����һ�����š�ͨ��������ɾ������
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
	AttributeMap _attributeMap;	//���Լ�
	int _num;									//����
};

#endif