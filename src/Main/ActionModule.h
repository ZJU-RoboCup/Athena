/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team:			SSL-ZJUNlict										*/
/* HomePage: http://www.nlict.zju.edu.cn/ssl/WelcomePage.html			*/
/************************************************************************/
/* File:	ActionModule.h												*/
/* Brief:	C++ Interface: Action	execution 							*/
/* Func:	Provide an action command send interface					*/
/* Author:	cliffyin, 2012, 08											*/
/* Refer:	NONE														*/
/* E-mail:	cliffyin007@gmail.com										*/
/* Version:	0.0.1														*/
/* Changelog:	2012-08-17	note the code								*/
/************************************************************************/	

#ifndef _ACTION_MODULE_H_
#define _ACTION_MODULE_H_

//////////////////////////////////////////////////////////////////////////
// include files
#include "DecisionModule.h"						// Decision for the match

/**
@brief		��������ģ�飬�ṩ�ⲿ���ýӿڡ�
@details	���ݾ���ģ���ִ�н�������ν��е����Ķ������ƣ��·��˶�ָ�
*/
class CActionModule {
public:

	/// <summary>	���캯����Constructor. </summary>
	///
	/// <remarks>	cliffyin, 2011/7/25. </remarks>
	///
	/// <param name="pOption">  	The running option. </param>
	/// <param name="pVision">  	[in,out] If non-null, the vision. </param>
	/// <param name="pDecision">	The decision module. </param>

	CActionModule(const COptionModule* pOption,CVisionModule* pVision,const CDecisionModule* pDecision);

	/// <summary>	����������Finaliser. </summary>
	///
	/// <remarks>	cliffyin, 2011/7/25. </remarks>

	~CActionModule(void);

	/// <summary>	С���˶�ָ���·��ӿڣ�Action command send interface, specify, Mainly. </summary>
	///
	/// <remarks>	cliffyin, 2011/7/25. </remarks>
	///
	/// <param name="robotIndex">	Real index of the vehicle. </param>
	///
	/// <returns>	true if it succeeds, false if it fails. </returns>

	bool sendAction(unsigned char robotIndex[]);

	/// <summary>	ֹͣС���˶��·��ӿڣ�Stop action command send interface�� Wireless off,TODO. </summary>
	///
	/// <remarks>	cliffyin, 2011/7/25. </remarks>
	///
	/// <param name="robotIndex">	Real index of the vehicle. </param>
	///
	/// <returns>	true if it succeeds, false if it fails. </returns>

	bool sendNoAction(unsigned char robotIndex[]);

	/// <summary>	ֹͣС���˶��·��ӿڣ�Stop all command send interface, Error Stop, TODO. </summary>
	///
	/// <remarks>	cliffyin, 2011/7/25. </remarks>

	void stopAll();

private:

	/// <summary> ��ǰ��������ģʽ��The running option </summary>
	const COptionModule* _pOption;

	/// <summary> ��ǰ�����Ӿ�ģ�飬The vision module </summary>
	CVisionModule* _pVision;

	/// <summary> ��ǰ��������ģ�飬The specified decision </summary>
	const CDecisionModule* _pDecision;
};

#endif // ~_ACTION_MODULE_H_
