/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team:			SSL-ZJUNlict										*/
/* HomePage: http://www.nlict.zju.edu.cn/ssl/WelcomePage.html			*/
/************************************************************************/
/* File:	BasicPlay.h													*/
/* Brief:	C++ Interface: Base class for play 							*/
/* Func:	Provide an base class for stated play						*/
/* Author:	cliffyin, 2012, 08											*/
/* Refer:	NONE														*/
/* E-mail:	cliffyin007@gmail.com										*/
/* Version:	0.0.1														*/
/* Changelog:	2012-08-17	note the code								*/
/************************************************************************/

#ifndef _BASIC_PLAY_H_
#define _BASIC_PLAY_H_

//////////////////////////////////////////////////////////////////////////
// forward declaration
class COptionModule;
class CVisionModule;
class CPlayerTask;

/**
@brief		�೵������ϵĻ��ࡣ
@details	��Ҫʵ��
*/
class CBasicPlay {
public:
	/**
	@brief	���캯��
	*/
	CBasicPlay() { _score = 0; _done = true; }

	/**
	@brief	ִ�в��ԣ���Ҫ��ʵ��
	@param	pOption	��ǰ��������ģʽ
	@param	pVision	��ǰ�����Ӿ�ģ��
	*/
	virtual void executePlay(const COptionModule* pOption, const CVisionModule* pVision) = 0;

	/**
	@brief	�л����ԣ���Ҫ��ʵ��
	@param	pOption	��ǰ��������ģʽ
	@param	pVision	��ǰ�����Ӿ�ģ��
	*/
	virtual CBasicPlay* transmitPlay(const COptionModule* pOption, const CVisionModule* pVision) = 0;
	
	/**
	@brief	��ȡ������
	*/
	virtual const char* playName() const = 0;

	/**
	@brief	�����Ƿ�ʹ��
	*/
	virtual bool applicable() const { return false; }
	
	/**
	@brief	�����Ƿ����
	*/
	virtual bool done() const { return _done; }

	/**
	@brief	���Է�ֵ
	*/
	double score() const { return _score; }
	
	/**
	@brief	����ˢ��
	*/
	virtual void refresh() { _done = false; }
	
	/**
	@brief	�����˳�
	*/
	virtual void quit() {}

protected:
	/**
	@brief	��С��������
	@param	num ִ��С����
	@param	pTask ��ǰִ�еĶ���
	@param	priority �������ȼ�
	*/
	void setTask(const int num, CPlayerTask* pTask, const int priority);

	/**
	@brief	���Ե�ǰ��ֵ
	*/
	double _score;

	/**
	@brief	�����Ƿ����
	*/
	bool _done;
};

#endif // ~_BASIC_PLAY_H_