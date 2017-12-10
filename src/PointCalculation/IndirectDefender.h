/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team：		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  IndirectDefender.h										*/
/* Func:	  防守对方定位球开球小车的站位点							*/
/* Author:	  王群 2012-08-18											*/
/* Refer:	  ###														*/
/* E-mail:	  wangqun1234@zju.edu.cn									*/
/* Version:	  0.0.1														*/
/************************************************************************/

#ifndef _INDIRECT_DEFENDER_H_
#define _INDIRECT_DEFENDER_H_

#include "DefPos1G2D.h"
#include <singleton.h>
#include <vector>

class CVisionModule;
using namespace std;
/**
@brief    防守对方定位球开球小车的站位点*/
typedef struct{
	CGeoPoint _Left;
	CGeoPoint _Right;
	double baseDir;
	double radius;
}KickOffArea;

typedef struct{
	CGeoPoint onePos;
	CGeoPoint anotherPos;
	CGeoPoint getOnePos(){return onePos;}
	CGeoPoint getAnotherPos(){return anotherPos;}
}TwoKickPos;

class CIndirectDefender:public CDefPos1G2D
{
public:
	CIndirectDefender();
	~CIndirectDefender();

	/** 
	@brief 外部取点接口 */
		//DefendPos 传入区域，半径，只防此区域内敌方车的开球
	//mode为1指防敌方区域的attacker，mode为2防止直接射门得分
	CGeoPoint getDefPos(const CVisionModule* pVision,double radius, CGeoPoint leftUp, CGeoPoint rightDown,int mode);

	TwoKickPos getTwoDefPos(const CVisionModule* pVision,double radius1,CGeoPoint left1,CGeoPoint right1,double radius2,CGeoPoint left2,CGeoPoint right2);

	void generateTwoDefPos(const CVisionModule* pVision);
	/** 
	@brief  生成站位点的函数，内部使用 */
	virtual CGeoPoint generatePos(const CVisionModule* pVision);

	/** 
	@brief  生成站位点的函数，内部使用 */

	CGeoPoint DefendPos(const CGeoPoint ballPos,const double dir,double radius);

	void FindMinMax(int num);

	//void clearKickOffList(){kickOffList.clear();}

private:
	int _lastCycle;
	int _lastTwoKickCycle;
	double _targetDir[Param::Field::MAX_PLAYER+1];
	CGeoPoint _leftUp;
	CGeoPoint _rightDown;
	double _radius;
	int _mode;
	int minnum;
	int maxnum;
	//vector<KickOffArea> kickOffList;
	TwoKickPos _twoKick;
	KickOffArea _oneArea;
	KickOffArea _anotherArea;
};

typedef NormalSingleton< CIndirectDefender > IndirectDefender;

#endif //_INDIRECT_DEFENDER_H_