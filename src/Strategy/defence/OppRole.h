#ifndef __OPP_ROLE_H__
#define __OPP_ROLE_H__
//���ֽ�ɫ,��ɫ��ʱ�����ڿɹ���oppRoleFactory
//��ɫ���ֹ��У������ɸı䣡��
//��ɫ��в�ȿ����ã�0Ϊû����в������Խ������вԽ��
#include <iostream>
#include <string>

using namespace std;

#define PRIORITY_ATTACK		0
#define PRIORITY_DEFENCE	100
#define PRIORITY_LAST		1000

class CAttributeSet;
class COppRoleMatcher;
class COppRole
{
public:
	COppRole(std::string name,int threatenPri,double mustIn,double canIn,double canOut,double mustOut):
		_roleName(name),
		_threatenPri(threatenPri),
		_mustChangeIn(mustIn),
		_canChangeIn(canIn),
		_canChangeOut(canOut),
		_mustChangeOut(mustOut),
		_subValue(0){}
	~COppRole(){}

	//���������̶�,ͨ��vision��refreebox������������������
	virtual void calcSubValue(CAttributeSet* attrSet) = 0;

	void SetProperty(int threaten,double mustIn,double canIn,double canOut,double mustOut)
	{
		_threatenPri = threaten;
		_mustChangeIn = mustIn;
		_canChangeIn = canIn;
		_canChangeOut = canOut;
		_mustChangeOut = mustOut;
	}
	std::string getName(){return _roleName;}
	int getThreatenPri(){return _threatenPri;}
	double getSubValue(){return _subValue;}

	bool mustChangeIn(){return _subValue > _mustChangeIn;}	
	bool canChangeIn(){return _subValue > _canChangeIn;}
	bool canChangeOut(){return _subValue < _canChangeOut;}
	bool mustChangeOut(){return _subValue < _mustChangeOut;}
	
	friend class COppRoleMatcher;
protected:
	void setSubValue(double subValue){_subValue = subValue;}
	
private:
	std::string _roleName;
	int _threatenPri;
	double _subValue;//�����̶ȣ��������������ɫ�ĳ̶�

	double _mustChangeIn;//���ĸ�������ȡ�����ļ�
	double _canChangeIn;
	double _canChangeOut;
	double _mustChangeOut;
};

#endif