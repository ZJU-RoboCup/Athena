/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team：		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  MarkingPosV2.h												*/
/* Func:	  盯人站位点												*/
/* Author:	  王群 2013-06-20											*/
/* Refer:	  ###														*/
/* E-mail:	  wangqun1234@zju.edu.cn									*/
/* Version:	  0.0.1														*/
/************************************************************************/
#ifndef __MARKING_POS_V2_H__
#define __MARKING_POS_V2_H__

#include "AtomPos.h"
#include "singleton.h"
#include "DefPos1G2D.h"
#include <vector>

using namespace std;

namespace NameSpaceMarkingPosV2{
	extern bool DENY_LOG[Param::Field::MAX_PLAYER+1];
	extern bool BACK_LOG[Param::Field::MAX_PLAYER+1];
};

class CVisionModule;
/**
@brief    盯人站位点
@details  本类的计算函数中，可以通过传入威胁优先级进行盯人，也可以直接传入对方车号盯人*/
class CMarkingPosV2 :public CDefPos1G2D
{
public:
	CMarkingPosV2();
	~CMarkingPosV2();
	/** 
	@brief  生成站位点的函数，内部使用 */
	virtual CGeoPoint generatePos(const CVisionModule* pVision);
	/** 
	@brief  通过传入优先级盯人
	@param	pri 盯人的优先级，当前可取1~5*/
	CGeoPoint getMarkingPos(const CVisionModule* pVision , const int pri);
	/** 

	@brief  通过传入优先级盯人
	@param	pri 盯人的优先级，当前可取0~5*/
	CGeoPoint getMarkingPosByAbsolutePri(const CVisionModule* pVision , const int pri);
	/** 

	@brief  通过传入对手车号盯人
	@param	num 对手的策略车号*/
	CGeoPoint getMarkingPosByNum(const CVisionModule* pVision , const int num);

	//当出现receiver时，找到离球最近的receiver，并看他是否被绕前盯人
	bool isNearestBallReceiverBeDenied(const CVisionModule* pVision);

	//针对skuba，是否采用特殊区域BACK_LINE模式
	bool isInSpecialAreaBackLineMode(const CVisionModule *pVision,const int num);

	//特殊区域判断1（写上编号是因为可能有多个区域判断）
	bool checkInSpecialArea_A(const CGeoPoint p,const CGeoPoint ballPos);

	//检测所有的markingPos是否会碰撞
	void checkAllMarkingPos(const CVisionModule *pVision);
private:
	int oppPriority;
	int oppNum;
	int _logCycle;
	int logCycle[Param::Field::MAX_PLAYER+1];
	CGeoPoint markingPoint[Param::Field::MAX_PLAYER+1];
	vector<int> areaList;
};

typedef NormalSingleton< CMarkingPosV2 > MarkingPosV2;

#endif //_MARKING_POS_H_