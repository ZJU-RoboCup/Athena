/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team��		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  MarkingPosV2.h												*/
/* Func:	  ����վλ��												*/
/* Author:	  ��Ⱥ 2013-06-20											*/
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
@brief    ����վλ��
@details  ����ļ��㺯���У�����ͨ��������в���ȼ����ж��ˣ�Ҳ����ֱ�Ӵ���Է����Ŷ���*/
class CMarkingPosV2 :public CDefPos1G2D
{
public:
	CMarkingPosV2();
	~CMarkingPosV2();
	/** 
	@brief  ����վλ��ĺ������ڲ�ʹ�� */
	virtual CGeoPoint generatePos(const CVisionModule* pVision);
	/** 
	@brief  ͨ���������ȼ�����
	@param	pri ���˵����ȼ�����ǰ��ȡ1~5*/
	CGeoPoint getMarkingPos(const CVisionModule* pVision , const int pri);
	/** 

	@brief  ͨ���������ȼ�����
	@param	pri ���˵����ȼ�����ǰ��ȡ0~5*/
	CGeoPoint getMarkingPosByAbsolutePri(const CVisionModule* pVision , const int pri);
	/** 

	@brief  ͨ��������ֳ��Ŷ���
	@param	num ���ֵĲ��Գ���*/
	CGeoPoint getMarkingPosByNum(const CVisionModule* pVision , const int num);

	//������receiverʱ���ҵ����������receiver���������Ƿ���ǰ����
	bool isNearestBallReceiverBeDenied(const CVisionModule* pVision);

	//���skuba���Ƿ������������BACK_LINEģʽ
	bool isInSpecialAreaBackLineMode(const CVisionModule *pVision,const int num);

	//���������ж�1��д�ϱ������Ϊ�����ж�������жϣ�
	bool checkInSpecialArea_A(const CGeoPoint p,const CGeoPoint ballPos);

	//������е�markingPos�Ƿ����ײ
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