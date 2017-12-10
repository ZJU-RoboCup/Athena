#ifndef _PLAY_INTERFACE_H_
#define _PLAY_INTERFACE_H_
#include <param.h>
#include <singleton.h>
#include <misc_types.h>
#include <map>
#include <string>
using namespace std;
#define MAX_ROBOTS_NUM 12

class CPlayInterface{
public:
	CPlayInterface();
	~CPlayInterface();

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
	// 真实车号和决策车号的对应
public:
	void setRealIndex(int Num, int realNum);
	int getRealIndexByNum(int Num);
	int getNumbByRealIndex(int realNum);
	void clearRealIndex();
	void setRoleNameAndNum(string, int);
	int getNumByRoleName(string);

private:
	int _realIndex[Param::Field::MAX_PLAYER]; // 保存我的真实号码
	int _visionIndex[MAX_ROBOTS_NUM]; // 真实车号对应的图像传过来的号码
	map<string, int> _roleNum;
};

typedef NormalSingleton< CPlayInterface > PlayInterface; // singleton pattern
#endif//_PLAY_INTERFACE_H_