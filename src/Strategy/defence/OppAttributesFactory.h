#ifndef __OPP_ATTRIBUTES_FACTORY_H__
#define __OPP_ATTRIBUTES_FACTORY_H__

#include "Attribute.h"
#include "ClassFactory.h"
#include "singleton.h"
class CAttributeSet;
class CAttributeFactory
{
public:
	CAttributeFactory();
	~CAttributeFactory();
	CAttributeSet* get(){return _attrSet;}
private:
	void configuration();
	CAttributeSet* _attrSet;
};
typedef NormalSingleton<CAttributeFactory> AttributeFactory;

#define DEFINE_ATTRIBUTE(attr_class)	\
DEFINE_CLASS_EX_H(attr_class,CAttribute)	\
{	\
public:	\
	attr_class():CAttribute(#attr_class){};	\
	virtual void evaluate(const CVisionModule* pVision,const int num);	\
};

#define EVALUATE_ATTRIBUTE(attr_class)	\
void attr_class::evaluate(const CVisionModule *pVision, const int num)


//attribute define
DEFINE_ATTRIBUTE(ATest);//for test 
DEFINE_ATTRIBUTE(ATest2);//for test
DEFINE_ATTRIBUTE(ACanBeLeader);//���ڼ��㶨λ��ʱ�����������leader
DEFINE_ATTRIBUTE(ADist2TheirGoal);//return double ���Է��������ĵ�ľ���
DEFINE_ATTRIBUTE(ADist2OurGoal);//return double ���ҷ��������ĵ�ľ���
DEFINE_ATTRIBUTE(ADist2Ball);	//return double	 ����ľ���
DEFINE_ATTRIBUTE(AValid);//return 1 or 0 �Է��Ƿ����
DEFINE_ATTRIBUTE(ABestPlayer);//return 1 or 0 �Ƿ��ǶԷ�BestPlayer
DEFINE_ATTRIBUTE(AShootRange);//return double ���ŽǶ�
DEFINE_ATTRIBUTE(AShootRangeWithoutMarker);//return double ���ҷ�Marker�ų�����ʱ�����ŽǶ�
DEFINE_ATTRIBUTE(AFillingInDist);//return double ��������
DEFINE_ATTRIBUTE(AKeeperDirFactor);//return double �Է�BestPlayer��������Է�BestPlayer�ͶԷ����߳���֮��
DEFINE_ATTRIBUTE(AMarkerDistFactor);//return double ���ҷ�Marker�ľ��룬���˶���ʱ��ֵ����500���ݶ���
DEFINE_ATTRIBUTE(AMarkerDirFactor);	//retrun double �Է����ų���͵��ҷ�Marker�ĳ���֮��,���˶���ʱ��ֵ����PI���ݶ���
DEFINE_ATTRIBUTE(AShootReflectAngle);//return double �Է����������
DEFINE_ATTRIBUTE(AReceiveReflectAngle);//return double or PI��������н����receiver�������
DEFINE_ATTRIBUTE(AImmortalFactor);//return double �������������������򵽸ö�Ա�����ļн�,��̬����
DEFINE_ATTRIBUTE(APassBlock);//return double����·���赲���룬���ҷ��赲�ľ��� ����ע�⣺����200cmʱ��������200
DEFINE_ATTRIBUTE(APosX);//return double λ�õ�Xֵ
DEFINE_ATTRIBUTE(APosY);//return double λ�õ�Yֵ
DEFINE_ATTRIBUTE(AVelX);//return double �ٶȵ�Xֵ
DEFINE_ATTRIBUTE(AVelY);//return double �ٶȵ�Yֵ
DEFINE_ATTRIBUTE(AGoalie);//return 1 or 0 �Է��Ƿ���goalie
DEFINE_ATTRIBUTE(ARecive);//�������ã�����
DEFINE_ATTRIBUTE(ATouchAbility);// return MAX_VALUE or double
DEFINE_ATTRIBUTE(AChaseAbility);// return MAX_VALUE or double 

#endif