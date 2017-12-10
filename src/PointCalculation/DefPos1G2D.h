/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team：		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  DefPos1G2D.h												*/
/* Func:	  1个门将和2个后卫联合封门									*/
/* Author:	  王群 2012-08-18											*/
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
@brief  门将站位点和后卫站位点的结构体 */
typedef struct  
{
	CGeoPoint leftD;	///<左后卫站位点
	CGeoPoint rightD;	///<右后卫站位点
	CGeoPoint middleD;	///<门将站位点
	/** 
	@brief  左后卫站位点的对外次级接口 */
	CGeoPoint getLeftPos(){return leftD;}
	/** 
	@brief  右后卫站位点的对外次级接口 */
	CGeoPoint getRightPos(){return rightD;}
	/** 
	@brief  门将站位点的对外次级接口 */
	CGeoPoint getGoaliePos(){return middleD;}
} defend3;

class CVisionModule;

/**
@brief    1个门将和2个后卫联合封门
@details  本类的计算函数中，是将所有的位置信息取反后再计算的
@note	  注意！在调用本站位点组时，要注册车号，即同时调用setLeftDefender和setRightDefender函数*/
class CDefPos1G2D:public CAtomPos
{
public:
	CDefPos1G2D();
	~CDefPos1G2D();	

	virtual CGeoPoint generatePos(const CVisionModule* pVision);
};

typedef NormalSingleton< CDefPos1G2D > DefPos1G2D;

#endif //_DEFPOS_1G_2D_H_