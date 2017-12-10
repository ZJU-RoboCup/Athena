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
DEFINE_ATTRIBUTE(ACanBeLeader);//用于计算定位球时两车开球的真leader
DEFINE_ATTRIBUTE(ADist2TheirGoal);//return double 到对方球门中心点的距离
DEFINE_ATTRIBUTE(ADist2OurGoal);//return double 到我方球门中心点的距离
DEFINE_ATTRIBUTE(ADist2Ball);	//return double	 到球的距离
DEFINE_ATTRIBUTE(AValid);//return 1 or 0 对方是否存在
DEFINE_ATTRIBUTE(ABestPlayer);//return 1 or 0 是否是对方BestPlayer
DEFINE_ATTRIBUTE(AShootRange);//return double 射门角度
DEFINE_ATTRIBUTE(AShootRangeWithoutMarker);//return double 将我方Marker排除在外时的射门角度
DEFINE_ATTRIBUTE(AFillingInDist);//return double 补防距离
DEFINE_ATTRIBUTE(AKeeperDirFactor);//return double 对方BestPlayer朝向线与对方BestPlayer和对方连线朝向之差
DEFINE_ATTRIBUTE(AMarkerDistFactor);//return double 和我方Marker的距离，无人盯防时该值返回500（暂定）
DEFINE_ATTRIBUTE(AMarkerDirFactor);	//retrun double 对方到门朝向和到我方Marker的朝向之差,无人盯防时该值返回PI（暂定）
DEFINE_ATTRIBUTE(AShootReflectAngle);//return double 对方射门折射角
DEFINE_ATTRIBUTE(AReceiveReflectAngle);//return double or PI传球过程中接球的receiver的折射角
DEFINE_ATTRIBUTE(AImmortalFactor);//return double 球到我门中心向量，与球到该队员向量的夹角,静态意义
DEFINE_ATTRIBUTE(APassBlock);//return double传球路线阻挡距离，被我方阻挡的距离 ！！注意：大于200cm时即锁定在200
DEFINE_ATTRIBUTE(APosX);//return double 位置的X值
DEFINE_ATTRIBUTE(APosY);//return double 位置的Y值
DEFINE_ATTRIBUTE(AVelX);//return double 速度的X值
DEFINE_ATTRIBUTE(AVelY);//return double 速度的Y值
DEFINE_ATTRIBUTE(AGoalie);//return 1 or 0 对方是否是goalie
DEFINE_ATTRIBUTE(ARecive);//将逐步弃用！！！
DEFINE_ATTRIBUTE(ATouchAbility);// return MAX_VALUE or double
DEFINE_ATTRIBUTE(AChaseAbility);// return MAX_VALUE or double 

#endif