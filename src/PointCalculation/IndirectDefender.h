/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team��		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  IndirectDefender.h										*/
/* Func:	  ���ضԷ���λ����С����վλ��							*/
/* Author:	  ��Ⱥ 2012-08-18											*/
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
@brief    ���ضԷ���λ����С����վλ��*/
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
	@brief �ⲿȡ��ӿ� */
		//DefendPos �������򣬰뾶��ֻ���������ڵз����Ŀ���
	//modeΪ1ָ���з������attacker��modeΪ2��ֱֹ�����ŵ÷�
	CGeoPoint getDefPos(const CVisionModule* pVision,double radius, CGeoPoint leftUp, CGeoPoint rightDown,int mode);

	TwoKickPos getTwoDefPos(const CVisionModule* pVision,double radius1,CGeoPoint left1,CGeoPoint right1,double radius2,CGeoPoint left2,CGeoPoint right2);

	void generateTwoDefPos(const CVisionModule* pVision);
	/** 
	@brief  ����վλ��ĺ������ڲ�ʹ�� */
	virtual CGeoPoint generatePos(const CVisionModule* pVision);

	/** 
	@brief  ����վλ��ĺ������ڲ�ʹ�� */

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