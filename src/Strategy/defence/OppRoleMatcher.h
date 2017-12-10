#ifndef __OPP_ROLE_MATCHER_H__
#define __OPP_ROLE_MATCHER_H__
//角色匹配的接口，可以增删角色
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include "singleton.h"
#include "param.h"
using namespace std;
class COppPlayer;
class COppRole;

typedef std::vector<COppRole*> OppRoleVector;

class COppRoleMatcher
{
public:
	COppRoleMatcher();
	~COppRoleMatcher();
	COppRole* matchRole(COppPlayer& oplayer);	
	void readInConfig(std::string fileName);//选入所有可能使用的副本!!读入配置文件

private:
	COppRole* change(OppRoleVector::iterator it, int num);

private:
	OppRoleVector _oppRolePriList;//带有优先级的list，排在前面的优先级高,并且经过config文档配置
	COppRole * _role[Param::Field::MAX_PLAYER + 1];//1~6 对应1~6号车
};

typedef NormalSingleton<COppRoleMatcher> OppRoleMatcher;

#endif