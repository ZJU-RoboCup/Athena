/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team：		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  ReceivePass.cpp											*/
/* Func:	  专门接球用的skill，接住球后传给目标朝向					*/
/* Author:	  zhaoyue, 2012.8.19										*/
/* E-mail:	  zhyaic@gmail.com											*/
/* Version:	  0.0.1														*/
/************************************************************************/

#ifndef _RECEIVE_PASS__
#define _RECEIVE_PASS__
#include <skill/PlayerTask.h>

/**  
@brief    专门接球用的skill，接住球后传给目标朝向
@details  2012墨西哥比赛时用于传球的Challenge，效果不错*/
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