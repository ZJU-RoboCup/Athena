/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team��		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  CornerAreaPos.h											*/
/* Func:	  �����Ұ볡�׽ǣ�����������ʱ�����ڷ��ضԷ���Ӧ��Աͷ��ĵ�*/
/* Author:	  ��Ⱥ 2012-08-18											*/
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
@brief    ���ҷ���������������ͷ���ŵĶ��ֵĺ���վλ��
@details  ��վλ��ȡ������һ��ԣ�ȵĽ������ϣ��ڶ��ֿ�����ʱʹ�á���վλ�������ѡȡ����Σ��ͷ��
���ŵĶԷ���Ա���ڶ��˷��ر��Է�ͨ����λ�Ʒ�ʱ�����Ի������ȡ�
@note	  ��վλ���붢�˷���վλ����໥ʶ����Э��λ�ã������ᷢ������С���ͷ�ͷ��С���ļ�ײ��
���Ǳ�������ź���֮�䲢�޴˻��ƣ����п��ܷ����໥��ײ������ǧ��Ҫע�⣡��������ǰģʽ�£��ڽ�
�򿪳�120֮֡��һ�����ܹ���ʹ�ñ�վλ�㡣��������⽫��δ��ͨ��ͳһ��Э�������������*/
class CCornerAreaPos:public CDefPos1G2D
{
public:
	CCornerAreaPos();
	~CCornerAreaPos();	

	/**
	@brief ���ɷ���ͷ��վλ��
	*/
	virtual CGeoPoint generatePos(const CVisionModule* pVision);
	
	/**
	@brief ��ѯ�õ��Ƿ�ʹ�ã���������Ϸ�ֹ��ͻ
	*/
	//bool isUsed(const CVisionModule* pVision);

	/**
	@brief ����ͷ��վλ���λ�ã���֪���㶢�˵��ģ��
	*/
	//CGeoPoint getMarkingPosByCornerArea(){return _forMarkingPos;}

	/**
	@brief ȡ�ñ�����صĵз���Ա����
	*/
//	int getHeadEnemyNum(){return _headAttackEnemy;}

	/**
	@brief �Ƿ��Ѿ�����λ��Э��ģʽ
	*/
//	bool willHit(){return _dealWithHit;}

	/**
	@brief �����ȡ��ӿ�
	*/
	CGeoPoint getCornerAreaPos(const CVisionModule* pVision);

private:
	CGeoPoint _CAPos;		 ///<��������ͷ��ĵ�
	CGeoPoint _lastCAPos;	 ///<ǰһ֡�ķ�������ͷ��ĵ�
	int _lastCycle;
//	CGeoPoint _forMarkingPos;///<�붢������ʱ���������˵ĵ�
//	int _headAttackEnemy;	 ///<�����صĶ���С���ĳ���
//	bool _dealWithHit;		 ///<tureΪ�������Ҷ�Ա��ײ
};

typedef NormalSingleton< CCornerAreaPos > CornerAreaPos;

#endif //_CORNER_AREA_POS_H_