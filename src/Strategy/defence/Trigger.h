#ifndef __TRIGGER_H__
#define __TRIGGER_H__
#include <iostream>
using namespace std;
// 形势变化信号触发链
class CVisionModule;
class CTrigger
{
public:
	CTrigger():_next(NULL){}
	~CTrigger(){delete _next;}

	void setNext(CTrigger* tri){_next = tri;}
	bool handler(const CVisionModule* pVision);
	bool virtual trigger(const CVisionModule* pVision){return false;}
protected:
	CTrigger * _next;
};
//////////////////////////////////////////////////////////////////////////
#define DEFINE_TRIGGER(triggerName)	\
class triggerName : public CTrigger	\
{	\
public:	\
	bool virtual trigger(const CVisionModule* pVision);	\
};																					

#define DESCRIPT_TRIGGER(triggerName)	\
	bool triggerName::trigger(const CVisionModule* pVision)
//////////////////////////////////////////////////////////////////////////
//define trigger
DEFINE_TRIGGER(testTrigger1);
DEFINE_TRIGGER(testTrigger2);
DEFINE_TRIGGER(TKeeperChanged);
//DEFINE_TRIGGER(TKeeperPassMe);//待加入
DEFINE_TRIGGER(TAttackerChanged);
DEFINE_TRIGGER(TReceiverNoMarked);//待修改
DEFINE_TRIGGER(TReceiverNoMarked2);//待修改
DEFINE_TRIGGER(MTPnoMarked);//Most threatening player is not marked    by gty 17.7.18
DEFINE_TRIGGER(MarkingValueDifference);//by gty 17.7.18
#endif