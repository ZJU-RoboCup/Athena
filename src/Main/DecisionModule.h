/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team:			SSL-ZJUNlict										*/
/* HomePage: http://www.nlict.zju.edu.cn/ssl/WelcomePage.html			*/
/************************************************************************/
/* File:	DecisionModule.h											*/
/* Brief:	C++ Interface: Decision	execution 							*/
/* Func:	Provide a decision interface for Strategy, Play selecting	*/
/* Author:	cliffyin, 2012, 08											*/
/* Refer:	NONE														*/
/* E-mail:	cliffyin007@gmail.com										*/
/* Version:	0.0.1														*/
/* Changelog:	2012-08-17	note the code								*/
/************************************************************************/

#ifndef _DECISION_MODULE_H_
#define _DECISION_MODULE_H_

//////////////////////////////////////////////////////////////////////////
// include files
#include <OptionModule.h>					/// Options for the match
#include <VisionModule.h>					/// Vision for the match

//////////////////////////////////////////////////////////////////////////
// forward declaration
class CPlayerTask;

/**
@brief		����������ģʽ���Ŷ�ģʽ������ģ�飬�ṩ�ⲿ���ýӿڡ�
@details	���ݵ�ǰ�ı���ģʽ������Ӿ���ģ��ѡȡ���ʵĶ೵Play����������
*/
class CDecisionModule {
public:

	/// <summary>	���캯����Constructor. </summary>
	///
	/// <remarks>	ZjuNlict, 2011-7-25. </remarks>
	///
	/// <param name="pOption">	The running option. </param>
	/// <param name="pVision">	[in,out] If non-null, the vision. </param>

	CDecisionModule(const COptionModule* pOption, CVisionModule* pVision);

	/// <summary>	����������Finaliser. </summary>
	///
	/// <remarks>	ZjuNlict, 2011-7-25. </remarks>

	~CDecisionModule(void);

	/// <summary>	�ⲿִ�нӿڣ�Executes the decision operation. </summary>
	///
	/// <remarks>	ZjuNlict, 2011-7-25. </remarks>
	///
	/// <param name="visualStop">	The visual stop flag. </param>

	void DoDecision(const bool visualStop);
protected:

	/// <summary>	ֹͣ��Ӧ��С�����˶���Stop the robot. </summary>
	///
	/// <remarks>	ZjuNlict, 2011-7-25. </remarks>
	///
	/// <param name="num">	Vehicle num. </param>

	void GenerateTasks(const bool visualStop);

	/// <summary>	�೵���ģʽ��Executes the team mode operation. </summary>
	///
	/// <remarks>	ZjuNlict, 2011-7-25. </remarks>

	void DoTeamMode();

	/// <summary>	����С����������滮��Plan tasks for every task-assigned vehicle. </summary>
	///
	/// <remarks>	ZjuNlict, 2011-7-25. </remarks>

	void PlanTasks();

private:
	/// <summary> ��ǰ��������ģʽ��The running option </summary>
	const COptionModule* _pOption;

	/// <summary> ��ǰ�����Ӿ�ģ�飬The vision module </summary>
	CVisionModule* _pVision;
};

#endif // ~_DECISION_MODULE_H_