#ifndef __OPP_ROLE_FACTORY_H__
#define __OPP_ROLE_FACTORY_H__

#include "OppRole.h"
#include "ClassFactory.h"
#include "singleton.h"
class CAttributeSet;
typedef std::map<std::string,COppRole*> OppRoleMap;

class COppRoleFactory
{
public:
	COppRoleFactory();
	~COppRoleFactory();
	OppRoleMap* get(){return _oppRoleBackMap;}
private:
	void configuration();
	OppRoleMap* _oppRoleBackMap;//保存所有role的一个副本，所有参数均为初始化参数
};
typedef NormalSingleton<COppRoleFactory> OppRoleFactory;

#define DEFINE_ROLE(role_name,pri,mustIn,canIn,canOut,mustOut)				\
DEFINE_CLASS_EX_H(role_name,COppRole)														\
{																													\
public:																											\
	role_name():COppRole(#role_name,pri,mustIn,canIn,canOut,mustOut){}		\
	virtual void calcSubValue(CAttributeSet* attrSet);										\
};

#define CALC_ROLE(role_name)										\
void role_name::calcSubValue(CAttributeSet* attrSet)		

///>角色定义
//特定角色 NULL_ROLE
DEFINE_ROLE(RNull,PRIORITY_LAST,0.9,0.6,0.4,0.1)
//测试的oppRole
DEFINE_ROLE(RTest,PRIORITY_ATTACK,0.9,0.6,0.4,0.1)
DEFINE_ROLE(RTest2,PRIORITY_ATTACK,0.9,0.6,0.4,0.1)
DEFINE_ROLE(RTest3,PRIORITY_ATTACK,0.9,0.6,0.4,0.1)
DEFINE_ROLE(RLeader,PRIORITY_ATTACK,0.9,0.6,0.4,0.1)
DEFINE_ROLE(RReceiver,PRIORITY_ATTACK,0.9,0.6,0.4,0.1)
DEFINE_ROLE(RSoldier,PRIORITY_ATTACK,0.9,0.6,0.4,0.1)
DEFINE_ROLE(RAttacker,PRIORITY_ATTACK,0.9,0.6,0.4,0.1)
DEFINE_ROLE(RGoalie,PRIORITY_DEFENCE,0.9,0.6,0.4,0.1)
DEFINE_ROLE(RDefender,PRIORITY_DEFENCE,0.9,0.6,0.4,0.1)
DEFINE_ROLE(RRusherFront,PRIORITY_ATTACK,0.9,0.6,0.4,0.1)
DEFINE_ROLE(RRusherBack,PRIORITY_DEFENCE,0.9,0.6,0.4,0.1)

#endif