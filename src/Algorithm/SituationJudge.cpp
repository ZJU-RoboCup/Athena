#include "SituationJudge.h"
#include "VisionModule.h"
using namespace std;
CSituationJudge::CSituationJudge() {

}

CSituationJudge::~CSituationJudge() {
	
}

int CSituationJudge::checkAttackerAmount(const CVisionModule* pVision, int meNum) {
	const MobileVisionT& ball = pVision->Ball();
	const int frontX = 250;
	const int backX = -150;
	const int slack = 30;
	static int field = BackField;
	int attackerAmount;
	switch (field) {
	case FrontField:
		if (ball.X() < backX) {
			field = BackField;
		}
		else if (ball.X() < frontX - slack) {
			field = MiddleField;
		}
		break;
	case MiddleField:
		if (ball.X() > frontX + slack) {
			field = FrontField;
		}
		else if (ball.X() < backX - slack) {
			field = BackField;
		}
		break;
	case BackField:
		if (ball.X() > frontX) {
			field = FrontField;
		}
		else if (ball.X() > backX + slack) {
			field = MiddleField;
		}
		break;
	}
	if (field == FrontField) {
		attackerAmount = 4;
	}
	else if (field == MiddleField) {
		attackerAmount = 3;
	}
	else {
		attackerAmount = 2;
	}
	return attackerAmount;
}