/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team:			SSL-ZJUNlict										*/
/* HomePage: http://www.nlict.zju.edu.cn/ssl/WelcomePage.html			*/
/************************************************************************/
/* File:	ActionModule.h												*/
/* Brief:	C++ Implementation: Action	execution						*/
/* Func:	Provide an action command send interface					*/
/* Author:	cliffyin, 2012, 08											*/
/* Refer:	NONE														*/
/* E-mail:	cliffyin007@gmail.com										*/
/************************************************************************/	

#include "ActionModule.h"
#include <tinyxml/ParamReader.h>
#include <KickStatus.h>
#include <DribbleStatus.h>
#include <TaskMediator.h>
#include <PlayerCommandV2.h>
#include <CommandSender.h>
#include <PlayInterface.h>
#include <CommandFactory.h>
#include <PathPlanner.h>
#include <BallStatus.h>
#include "Semaphore.h"
extern Semaphore decision_to_action;

namespace {

	/// ����ʵ���־
	bool IS_SIMULATION = true;

}

CActionModule::CActionModule(const COptionModule* pOption,CVisionModule* pVision,const CDecisionModule* pDecision)
: _pOption(pOption),_pVision(pVision),_pDecision(pDecision)
{
	{
		DECLARE_PARAM_READER_BEGIN(General)
			READ_PARAM(IS_SIMULATION)
		DECLARE_PARAM_READER_END
	}
}

CActionModule::~CActionModule(void)
{

}

// ���ڵ������еĳ��Ŵ���5�����
bool CActionModule::sendAction(unsigned char robotIndex[])
{
	decision_to_action.Wait();
	/************************************************************************/
	/* ��һ��������С����ִ�и�����������ɶ���ָ��                       */
	/************************************************************************/
	for (int vecNum = 1; vecNum <= Param::Field::MAX_PLAYER; ++ vecNum) {
		// ��ȡ��ǰС������
		CPlayerTask* pTask = TaskMediator::Instance()->getPlayerTask(vecNum);
		// û����������
		if (NULL == pTask) {
			continue;
		}

		// ִ��skill�����������ִ�У��õ����յ�ָ�<vx vy w> + <kick dribble>
		// ִ�еĽ��������ӿڣ�����-DCom��ʵ��-CommandSender�� + ָ���¼���˶�-Vision���߿�-PlayInterface)
		bool dribble = false;
		CPlayerCommand* pCmd = NULL;
		pCmd = pTask->execute(_pVision); 

		// �ܣ���Ч���˶�ָ��
		if (pCmd) {
			dribble = pCmd->dribble() > 0;
			// �·��˶� <vx vy w>
			pCmd->execute(IS_SIMULATION, robotIndex[vecNum-1]);
			// ��¼ָ��
			_pVision->SetPlayerCommand(pCmd->number(), pCmd);
		}

		// �ߣ���Ч���߿�ָ��
		double kickPower = 0.0;
		double chipkickDist = 0.0;
		double passdist = 0.0;
		if (KickStatus::Instance()->needKick(vecNum)) {
			// ��������ز���
			kickPower = KickStatus::Instance()->getKickPower(vecNum);
			chipkickDist = KickStatus::Instance()->getChipKickDist(vecNum);
			passdist = KickStatus::Instance()->getPassDist(vecNum);
			// �漰��ƽ/����ֵ�������ֻ��ϵ��ز�����ʵ�ʷֵ����ע CommandSender
			CPlayerKickV2 kickCmd(vecNum, kickPower, chipkickDist, passdist, dribble);
			// �������� <kick dribble>
			kickCmd.execute(IS_SIMULATION);
		}

		// ��¼����
		BallStatus::Instance()->setCommand(vecNum, kickPower, chipkickDist, dribble, _pVision->Cycle());
	}
	
	/************************************************************************/
	/* �ڶ�����ָ����մ���                                                 */
	/************************************************************************/
	// �����һ���ڵ�����ָ��
	KickStatus::Instance()->clearAll();
	// �����һ���ڵĿ���ָ��
	DribbleStatus::Instance()->clearDribbleCommand();
	// �����һ���ڵ��ϰ�����
	CPathPlanner::resetObstacleMask();

	// ʵ��ģʽ�趨ģʽ����ָ���·�
	if (! IS_SIMULATION) {
		// ģʽ�趨
		CommandSender::Instance()->setMode();
		// �����·���ע�⴫�������ʵ���ţ��Ҷ�Ӧ1-6��
		CommandSender::Instance()->sendCommand(robotIndex);
		// �����һ���ڵ�������ָ��
		for(int vecNum = 1; vecNum <= Param::Field::MAX_PLAYER; ++ vecNum){
			CommandSender::Instance()->setGyro(vecNum, 0, 0, 0, 0);
			CommandSender::Instance()->setstop(vecNum, false);
		}
	}

	return true;
}

bool CActionModule::sendNoAction(unsigned char robotIndex[])
{
	for (int vecNum = 1; vecNum <= Param::Field::MAX_PLAYER; ++ vecNum) {
		// ����ֹͣ����
		CPlayerCommand *pCmd = CmdFactory::Instance()->newCommand(CPlayerSpeedV2(vecNum,0,0,0,0));
		// ִ�����·�
		pCmd->execute(IS_SIMULATION);
		// ��¼ָ��
		_pVision->SetPlayerCommand(pCmd->number(), pCmd);

		if (! IS_SIMULATION) {
			CommandSender::Instance()->setMode();
			CommandSender::Instance()->sendCommand(robotIndex);
		}
	}

	return true;
}

void CActionModule::stopAll()
{
	for (int vecNum = 1; vecNum <= Param::Field::MAX_PLAYER; ++ vecNum) {
		// ����ֹͣ����
		CPlayerCommand *pCmd = CmdFactory::Instance()->newCommand(CPlayerSpeedV2(vecNum,0,0,0,0));
		// ִ�����·�
		pCmd->execute(IS_SIMULATION);
		// ָ���¼
		_pVision->SetPlayerCommand(pCmd->number(), pCmd);

		if (! IS_SIMULATION) {
			CommandSender::Instance()->setMode(MODE_DEBUG_Check, vecNum);
			CommandSender::Instance()->sendCommand();
		}
	}

	return ;
}