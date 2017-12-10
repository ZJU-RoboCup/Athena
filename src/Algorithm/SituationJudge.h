#ifndef _SITUATION_JUDGE_H_
#define _SITUATION_JUDGE_H_

#include "singleton.h"
#include "VisionModule.h"
#include "WorldModel.h"
#include "TaskMediator.h"


enum {
	FrontField = 1,
	MiddleField,
	BackField
};
class CSituationJudge {
private:
	const string getBallStatus() {
		string myStatus = WorldModel::Instance()->getBallStatus(VisionModule::Instance()->Cycle(), TaskMediator::Instance()->ballHandler());
		return myStatus ;
	}
public:
	CSituationJudge::CSituationJudge();
	CSituationJudge::~CSituationJudge();
public:
	int checkAttackerAmount(const CVisionModule* pVision, int meNum = 0);
};

typedef NormalSingleton< CSituationJudge > SituationJudge;

#endif
