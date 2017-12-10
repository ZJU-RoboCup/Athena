#ifndef __OPP_ROLE_MATCHER_H__
#define __OPP_ROLE_MATCHER_H__
//��ɫƥ��Ľӿڣ�������ɾ��ɫ
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
	void readInConfig(std::string fileName);//ѡ�����п���ʹ�õĸ���!!���������ļ�

private:
	COppRole* change(OppRoleVector::iterator it, int num);

private:
	OppRoleVector _oppRolePriList;//�������ȼ���list������ǰ������ȼ���,���Ҿ���config�ĵ�����
	COppRole * _role[Param::Field::MAX_PLAYER + 1];//1~6 ��Ӧ1~6�ų�
};

typedef NormalSingleton<COppRoleMatcher> OppRoleMatcher;

#endif