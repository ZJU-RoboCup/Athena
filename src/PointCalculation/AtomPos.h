/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team：		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  AtomPos.h													*/
/* Func:	  计算一个策略点											*/
/* Author:	  王群 2012-08-18											*/
/* Refer:	  ###														*/
/* E-mail:	  wangqun1234@zju.edu.cn									*/
/* Version:	  0.0.1														*/
/************************************************************************/

#ifndef _ATOM_POS_H_
#define _ATOM_POS_H_

#include "geometry.h"

namespace Pos{
	const CGeoPoint invalidPos = CGeoPoint(9999,9999); ///<默认的错误点
}

class CVisionModule;

/**
@brief    策略点的基类
@details  本基类的子类用于计算单个策略点，类中给出了最基础的操作Point的函数，其关键函数generatePos
需要在子类中定义*/
class CAtomPos
{
public:
	CAtomPos();
	virtual ~CAtomPos();

	/**
	@brief 取得计算好的策略点
	@note  调用此函数必须在至少调用过一次generatePos之后
	*/
	CGeoPoint getPos();

	/**
	@brief 设定策略点的值
	*/
	void setPos(const CGeoPoint& pos);

	/**
	@brief 清除策略点的值
	*/
	void clearPos();

	/**
	@brief 判断策略点是否符合要求
	*/
	virtual bool isAvailable();

	/**
	@brief 打印出策略点的值
	*/
	virtual void printDebug();

	/**
	@brief 策略点的计算函数
	*/
	virtual CGeoPoint generatePos(const CVisionModule* pVision) = 0;

private:
	CGeoPoint _pos;
};

#endif //_ATOM_POS_H_