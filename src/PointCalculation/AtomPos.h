/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team��		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  AtomPos.h													*/
/* Func:	  ����һ�����Ե�											*/
/* Author:	  ��Ⱥ 2012-08-18											*/
/* Refer:	  ###														*/
/* E-mail:	  wangqun1234@zju.edu.cn									*/
/* Version:	  0.0.1														*/
/************************************************************************/

#ifndef _ATOM_POS_H_
#define _ATOM_POS_H_

#include "geometry.h"

namespace Pos{
	const CGeoPoint invalidPos = CGeoPoint(9999,9999); ///<Ĭ�ϵĴ����
}

class CVisionModule;

/**
@brief    ���Ե�Ļ���
@details  ��������������ڼ��㵥�����Ե㣬���и�����������Ĳ���Point�ĺ�������ؼ�����generatePos
��Ҫ�������ж���*/
class CAtomPos
{
public:
	CAtomPos();
	virtual ~CAtomPos();

	/**
	@brief ȡ�ü���õĲ��Ե�
	@note  ���ô˺������������ٵ��ù�һ��generatePos֮��
	*/
	CGeoPoint getPos();

	/**
	@brief �趨���Ե��ֵ
	*/
	void setPos(const CGeoPoint& pos);

	/**
	@brief ������Ե��ֵ
	*/
	void clearPos();

	/**
	@brief �жϲ��Ե��Ƿ����Ҫ��
	*/
	virtual bool isAvailable();

	/**
	@brief ��ӡ�����Ե��ֵ
	*/
	virtual void printDebug();

	/**
	@brief ���Ե�ļ��㺯��
	*/
	virtual CGeoPoint generatePos(const CVisionModule* pVision) = 0;

private:
	CGeoPoint _pos;
};

#endif //_ATOM_POS_H_