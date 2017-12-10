#ifndef __OPP_ROLE_H__
#define __OPP_ROLE_H__
//对手角色,角色多时，后期可构建oppRoleFactory
//角色名字固有！！不可改变！！
//角色威胁度可配置，0为没有威胁，数字越大则威胁越大
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

	//计算隶属程度,通过vision，refreebox，传感器等输入数据
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
	double _subValue;//隶属程度，表征属于这个角色的程度

	double _mustChangeIn;//这四个参数读取配置文件
	double _canChangeIn;
	double _canChangeOut;
	double _mustChangeOut;
};

#endif