/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team:			SSL-ZJUNlict										*/
/* HomePage: http://www.nlict.zju.edu.cn/ssl/WelcomePage.html			*/
/************************************************************************/
/* File:	DecisionModule.h											*/
/* Brief:	C++ Implementation: Decision	execution					*/
/* Func:	Provide a decision interface for Strategy, Play selecting	*/
/* Author:	cliffyin, 2012, 08											*/
/* Refer:	NONE														*/
/* E-mail:	cliffyin007@gmail.com										*/
/************************************************************************/

#include "DecisionModule.h"
#include <skill/Factory.h>
#include <TaskMediator.h>
#include <PlayInterface.h>
#include <tinyxml/ParamReader.h>
#include <skill/BasicPlay.h>
#include "LuaModule.h"
#include "Semaphore.h"
extern Semaphore vision_to_decision;
Semaphore decision_to_action(0);
namespace {
	/// �Ƿ�״̬���Ĳ��Կ�
	bool USE_LUA_SCRIPTS = false;
	/// ��ǰ����ϲ���
	CBasicPlay* play = NULL;
}

CDecisionModule::CDecisionModule(const COptionModule* pOption, CVisionModule* pVision): _pOption(pOption),_pVision(pVision)
{
		DECLARE_PARAM_READER_BEGIN(General)
			READ_PARAM(USE_LUA_SCRIPTS)
			DECLARE_PARAM_READER_END
		
		if(USE_LUA_SCRIPTS){
			LuaModule::Instance()->RunScript("./lua_scripts/StartZeus.lua");
		}
}

CDecisionModule::~CDecisionModule(void)
{
	
}

void CDecisionModule::DoDecision(const bool visualStop)
{
	vision_to_decision.Wait();
	/************************************************************************/
	/* �����һ���ڵ���ʷ����                                               */
	/************************************************************************/
	TaskMediator::Instance()->cleanOldTasks();

	/************************************************************************/
	/* ѡȡ���ʵ�Play�������������                                         */
	/************************************************************************/
    GenerateTasks(visualStop);	
	
	/************************************************************************/
	/* ��������Ĺ滮����ν�����������趨                                 */
	/* ÿ����Ա��������skill,����ÿ��skill�����˹滮						*/
	/* (����GotoPosition���������㻹ûִ��,��Ҫ��executeʱִ��)			*/
	/************************************************************************/
	PlanTasks();

	decision_to_action.Signal();
	return ;
}

void CDecisionModule::GenerateTasks(const bool visualStop)
{
	// ͼ��ֹͣ �� �ղ��� ���� ��ͣ����
	if (visualStop) {
		// ÿ��С���·�ֹͣ����
		for (int vecNumber = 1; vecNumber <= Param::Field::MAX_PLAYER; ++ vecNumber) {
			TaskMediator::Instance()->setPlayerTask(vecNumber, PlayerRole::makeItStop(vecNumber), LowestPriority);
		}
		return;
	}

	DoTeamMode();
   
	return ;
}

void CDecisionModule::DoTeamMode()
{
	// ��ǰ��������
	play = NULL;

	// ���ֲ��Կ���о��߹滮
	if (USE_LUA_SCRIPTS){
		LuaModule::Instance()->RunScript("./lua_scripts/SelectPlay.lua");
	} 
	else {
		cout << "NO PLAY!!!" << Param::Output::NewLineCharacter;
	}

	return ;
}

void CDecisionModule::PlanTasks()
{
	/************************************************************************/
	/* ����task�����ȼ�ִ��:�򵥵�ð������                                */
	/************************************************************************/
	// �ṹ����
    typedef std::pair< int, int > TaskPair;
    typedef std::vector< TaskPair > TaskPairList;

	// ���ڴ洢��Ա�ż�����������ȼ�
    TaskPairList taskPairList;	
	taskPairList.clear();

	// ���ݻ�ȡ����С�����񣬽������ȼ�����
    for (int num = 1; num <= Param::Field::MAX_PLAYER; ++ num) {
		// TODO ֻ�г��Ͽ��õ��ұ����������С���Ž������ȼ������Ƿ���������
		if (/*_pVision->OurPlayer(num).Valid() && */TaskMediator::Instance()->getPlayerTask(num)) {
			// �洢�����Ѿ��趨������Ķ�Ա,����skill��priority,��������;
			taskPairList.push_back(TaskPair(num, TaskMediator::Instance()->getPlayerTaskPriority(num)));
		}
    }

    if (! taskPairList.empty()) {
		// ð�ݷ�����,���ȼ��ߵ���ǰ
		for (int i = 0; i < taskPairList.size()-1; ++ i) {
			for (int j = i+1; j < taskPairList.size(); ++ j) {
				if (taskPairList[i].second < taskPairList[j].second){
					std::swap(taskPairList[i],taskPairList[j]); // ����˳��
				}
			}
		}
		
		// �����������ȼ�����Ľ���������Ӧ������滮����Ҫ�������������
		// һ�����⣬�����������ͼ�񶪳������������δ����Ƿ�᲻���ж�Ӧ������滮
		for (int i = 0; i < taskPairList.size(); ++ i) {
			// ����taskPairList�е����ȼ�����,ִ�и���skill��plan����,�õ�����������һ���ڵ�GotoPosition����(�κ�skill���ն���������û���skill��)
			// (���������֮��ActionModule��sendAction����ͨ�����ø�skill��execute�������õ�ִ��,
			// �����ռ��������������һ���ڵ�x,y�ٶ�,�Ƿ�������ŵȵײ�ָ��)
			TaskMediator::Instance()->getPlayerTask(taskPairList[i].first)->plan(_pVision);
		}
	}

	// TODO ����û������ķ��䵽������Դ�ӡ��ʾ�����Ȳ����Դ���
	// ������������Ǹ��輱ͣ��������� MakeUpTask(num)

	return ;
}