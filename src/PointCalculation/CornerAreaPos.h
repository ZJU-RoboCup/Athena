/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team：		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  CornerAreaPos.h											*/
/* Func:	  球在我半场底角，对手挑球传中时，用于防守对方接应队员头球的点*/
/* Author:	  王群 2012-08-18											*/
/* Refer:	  ###														*/
/* E-mail:	  wangqun1234@zju.edu.cn									*/
/* Version:	  0.0.1														*/
/************************************************************************/

#ifndef _CORNER_AREA_POS_H_
#define _CORNER_AREA_POS_H_

#include <singleton.h>
#include "DefPos1G2D.h"

class CVisionModule;

/**
@brief    在我方禁区附近，防范头球攻门的对手的后卫站位点
@details  本站位点取自增加一定裕度的禁区线上，在对手开角球时使用。本站位点可自主选取最有危险头球
攻门的对方队员，在盯人防守被对方通过跑位破防时，可以积极补救。
@note	  本站位点与盯人防守站位点可相互识别并且协调位置，即不会发生盯人小车和防头球小车的挤撞，
但是本点与封门后卫之间并无此机制，极有可能发生相互碰撞，这里千万要注意！！！！当前模式下，在角
球开出120帧之后，一定不能够再使用本站位点。这里的问题将在未来通过统一的协调机制来解决。*/
class CCornerAreaPos:public CDefPos1G2D
{
public:
	CCornerAreaPos();
	~CCornerAreaPos();	

	/**
	@brief 生成防范头球站位点
	*/
	virtual CGeoPoint generatePos(const CVisionModule* pVision);
	
	/**
	@brief 查询该点是否被使用，跟盯人配合防止冲突
	*/
	//bool isUsed(const CVisionModule* pVision);

	/**
	@brief 将防头球站位点的位置，告知计算盯人点的模块
	*/
	//CGeoPoint getMarkingPosByCornerArea(){return _forMarkingPos;}

	/**
	@brief 取得本点防守的敌方队员号码
	*/
//	int getHeadEnemyNum(){return _headAttackEnemy;}

	/**
	@brief 是否已经进入位置协调模式
	*/
//	bool willHit(){return _dealWithHit;}

	/**
	@brief 对外的取点接口
	*/
	CGeoPoint getCornerAreaPos(const CVisionModule* pVision);

private:
	CGeoPoint _CAPos;		 ///<防范对手头球的点
	CGeoPoint _lastCAPos;	 ///<前一帧的防范对手头球的点
	int _lastCycle;
//	CGeoPoint _forMarkingPos;///<与盯人联动时，传给盯人的点
//	int _headAttackEnemy;	 ///<所防守的对手小车的车号
//	bool _dealWithHit;		 ///<ture为即将自我队员碰撞
};

typedef NormalSingleton< CCornerAreaPos > CornerAreaPos;

#endif //_CORNER_AREA_POS_H_