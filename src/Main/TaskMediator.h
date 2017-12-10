#ifndef _TASK_MEDIATOR_H_
#define _TASK_MEDIATOR_H_

#include <singleton.h>
#include <geometry.h>
#include <param.h>
#include <skill/PlayerTask.h>
#include "utils.h"
#include "Global.h"
#include <PlayInterface.h>
/**
* task协调类.
* 这个类处理拿球冲突以及传接球等需要多个机器人协调的事情
*/

struct SpecialRole{
	SpecialRole() : num(0), lastCycle(0){}
	int num;
	int lastCycle;
};

class CTaskMediator{
public:
	CTaskMediator() : _ballHandler(0)
	{
		USE_LUA_SCRIPTS = false;
		DECLARE_PARAM_READER_BEGIN(General)
		READ_PARAM(USE_LUA_SCRIPTS)
		DECLARE_PARAM_READER_END
		for(int i=1; i<=Param::Field::MAX_PLAYER; ++i ){
			_playerTask[i] = 0;
		}
	}
	// 拿球队员
	void setBallHandler(int num){ _ballHandler = num; }
	int ballHandler() const { return _ballHandler; }

	// 守门员
    int goalie(){
		if(USE_LUA_SCRIPTS){
			return PlayInterface::Instance()->getRealIndexByNum(_goalie.num);
		}else{
			// need to be checked by zhyaic 2014.2.19
			return PlayInterface::Instance()->getRealIndexByNum(_goalie.num);
			//return PlayInterface::Instance()->getRealIndexByNum(PlayMediator::Instance()->getRealRobotNum("Goalie"));
		}
	}

	// 左后卫
	int leftBack(){
		if(vision->Cycle() - _leftBack.lastCycle > 5){
			_leftBack.num = 0;
		}
		return _leftBack.num;
	}

	// 右后卫
	int rightBack(){
		if(vision->Cycle() - _rightBack.lastCycle > 5){
			_rightBack.num = 0;
		}
		return _rightBack.num;
	}

	// 单后卫
	int singleBack(){
		if(vision->Cycle() - _singleBack.lastCycle > 5){
			_singleBack.num = 0;
		}
		return _singleBack.num;
	}

	// 防守后腰
	int defendMiddle(){
		if(vision->Cycle() - _defendMiddle.lastCycle > 5){
			_defendMiddle.num = 0;
		}
		return _defendMiddle.num;
	}

	//边后卫
	int sideBack(){
		if(vision->Cycle() - _sideBack.lastCycle > 5){
			_sideBack.num = 0;
		}
		return _sideBack.num;
	}

	int advancer(){
		if (vision->Cycle() - _advancer.lastCycle> 5){
			_advancer.num = 0;
		}
		return _advancer.num;
	}
	// 任务
	void setPlayerTask(const int num, CPlayerTask* pTask, const int priority)
	{
		if (Utils::PlayerNumValid(num)){
			_playerTaskPriority[num] = priority;
			_playerTask[num] = pTask;
		}
	}

	CPlayerTask* getPlayerTask(const int num) const 
	{ 
		if (Utils::PlayerNumValid(num)){
			return _playerTask[num]; 
		}else{
			return NULL;
		}
	}

	int getPlayerTaskPriority(const int num) const 
	{ 
		if (Utils::PlayerNumValid(num)){
			return _playerTaskPriority[num]; 
		}else{
			return -1;
		}
	}

	void cleanOldTasks()
	{
		// 执行完该函数后,标志数组都为false, 任务数组则设定了任务的队员位置保留, 其他队员位置清零.
		for(int num=1; num<=Param::Field::MAX_PLAYER; ++num ){
			_playerTask[num] = 0;
		}
	}

	void setRoleInLua(int num, string role)
	{
		if("goalie" == role){
			_goalie.num = num;
			_goalie.lastCycle = vision->Cycle();
		} else if ("leftBack" == role){
			_leftBack.num = num;
			_leftBack.lastCycle = vision->Cycle();
		} else if ("rightBack" == role){
			_rightBack.num = num;
			_rightBack.lastCycle = vision->Cycle();
		} else if ("singleBack" == role){
			_singleBack.num = num;
			_singleBack.lastCycle = vision->Cycle();
		} else if ("defendMiddle" == role){
			_defendMiddle.num = num;
			_defendMiddle.lastCycle = vision->Cycle();
		} else if ("sideBack" == role){
			_sideBack.num = num;
			_sideBack.lastCycle = vision->Cycle();
		}else if ("advancer" == role){
			_advancer.num = num;
			_advancer.lastCycle = vision->Cycle();
		}else{
			cout<<"Error In setRoleInLua!!!  "<<role<<endl;
		}
	}
private:
	bool USE_LUA_SCRIPTS;
	SpecialRole _goalie;
	SpecialRole _leftBack;
	SpecialRole _rightBack;
	SpecialRole _singleBack;
	SpecialRole _defendMiddle;
	SpecialRole _sideBack;
	SpecialRole _advancer;
	int _ballHandler; // 拿球队员
	CPlayerTask* _playerTask[Param::Field::MAX_PLAYER+1]; // 队员的任务
	int _playerTaskPriority[Param::Field::MAX_PLAYER+1]; // 队员任务的优先级
};
typedef NormalSingleton< CTaskMediator > TaskMediator;

#endif // _TASK_MEDIATOR_H_
