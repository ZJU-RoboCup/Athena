/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team��		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  ReceivePass.cpp											*/
/* Func:	  ר�Ž����õ�skill����ס��󴫸�Ŀ�곯��					*/
/* Author:	  zhaoyue, 2012.8.19										*/
/* E-mail:	  zhyaic@gmail.com											*/
/* Version:	  0.0.1														*/
/************************************************************************/

#ifndef _RECEIVE_PASS__
#define _RECEIVE_PASS__
#include <skill/PlayerTask.h>

/**  
@brief    ר�Ž����õ�skill����ס��󴫸�Ŀ�곯��
@details  2012ī�������ʱ���ڴ����Challenge��Ч������*/
class CReceivePass:public CStatedTask {
public:
	CReceivePass();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: ReceivePass\n"; }

private:
	int _lastCycle;
	int _lastRunner;
	CGeoPoint _mePos;
	CPlayerCommand * _directCommand;
};

#endif //_DRIBBLE_PULL__