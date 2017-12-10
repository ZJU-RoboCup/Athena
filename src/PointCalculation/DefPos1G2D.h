/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team��		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  DefPos1G2D.h												*/
/* Func:	  1���Ž���2���������Ϸ���									*/
/* Author:	  ��Ⱥ 2012-08-18											*/
/* Refer:	  ###														*/
/* E-mail:	  wangqun1234@zju.edu.cn									*/
/* Version:	  0.0.1														*/
/************************************************************************/

#ifndef _DEFPOS_1G_2D_H_
#define _DEFPOS_1G_2D_H_

#include "AtomPos.h"
#include <singleton.h>
#include "DefendUtils.h"




/** 
@brief  �Ž�վλ��ͺ���վλ��Ľṹ�� */
typedef struct  
{
	CGeoPoint leftD;	///<�����վλ��
	CGeoPoint rightD;	///<�Һ���վλ��
	CGeoPoint middleD;	///<�Ž�վλ��
	/** 
	@brief  �����վλ��Ķ���μ��ӿ� */
	CGeoPoint getLeftPos(){return leftD;}
	/** 
	@brief  �Һ���վλ��Ķ���μ��ӿ� */
	CGeoPoint getRightPos(){return rightD;}
	/** 
	@brief  �Ž�վλ��Ķ���μ��ӿ� */
	CGeoPoint getGoaliePos(){return middleD;}
} defend3;

class CVisionModule;

/**
@brief    1���Ž���2���������Ϸ���
@details  ����ļ��㺯���У��ǽ����е�λ����Ϣȡ�����ټ����
@note	  ע�⣡�ڵ��ñ�վλ����ʱ��Ҫע�ᳵ�ţ���ͬʱ����setLeftDefender��setRightDefender����*/
class CDefPos1G2D:public CAtomPos
{
public:
	CDefPos1G2D();
	~CDefPos1G2D();	

	virtual CGeoPoint generatePos(const CVisionModule* pVision);
};

typedef NormalSingleton< CDefPos1G2D > DefPos1G2D;

#endif //_DEFPOS_1G_2D_H_