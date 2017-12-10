#ifndef _IEVALUATOR_H_
#define _IEVALUATOR_H_
#include <geometry.h>

/*
* ��ͬ���۷����Ľӿ���
*/
class CVisionModule;
class IEvaluator{
public:
	virtual bool isEligible(const CVisionModule* pVision, const CGeoPoint& evaluatedPos) = 0;
	virtual double getPotential(const CVisionModule* pVision, const CGeoPoint& evaluatedPos) = 0;
	virtual void returnGrade() = 0;//�鿴������
};
#endif