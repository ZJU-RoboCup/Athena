#include "DribbleTurn.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "RobotSensor.h"
#include "skill/Factory.h"
#include "ControlModel.h"
#include <utils.h>
#include <DribbleStatus.h>
#include <ControlModel.h>
#include "GDebugEngine.h"
#include "TaskMediator.h"

//****************************/
/* 2014-05-26 yys            */

/*****************************/
namespace {
	int sinT = 30;
	const int dribblePower = 3;
	const int maxFrared = 100;  // 最大红外帧数
	const int maxMeHasBall = 50;
	int meHasBallMaxCnt = 2;
}

CDribbleTurn::CDribbleTurn() {
	DECLARE_PARAM_READER_BEGIN(General)
	DECLARE_PARAM_READER_END
}

void CDribbleTurn::plan(const CVisionModule* pVision) {
	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1) {
		setState(BEGINNING);
		count = 0;
	}
	int vecNumber       = task().executor;
	double finalDir     = task().player.angle;
	double adjustPre    = task().player.speed_x;     // 精度

	//视觉初步处理
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos   = ball.Pos();
	const int robotNum        = task().executor;
	const PlayerVisionT& me   = pVision->OurPlayer(robotNum);
	double faceDir            = Utils::Normalize((ball.Pos() - me.Pos()).dir());

	//CPlayerTask* pTask = TaskMediator::Instance()->getPlayerTask(vecNumber);
	//CStatedTask* pstateTask = (CStatedTask*)pTask;
	//cout << "current state: " << pstateTask->state() << endl;

	int flag = task().player.flag | PlayerStatus::DODGE_OUR_DEFENSE_BOX;

	//红外信息
	bool frared = RobotSensor::Instance()->IsInfraredOn(vecNumber);
	//检测有连续多少帧有红外
	if (frared) { infraredOn = infraredOn >= maxFrared ? maxFrared : infraredOn + 1; }
	else { infraredOn = 0; }

	//自已有球的判断
	bool isMeHasBall;
	bool isMechHasBall = infraredOn >= 1;                         // 红外检测到球
	bool visionHasBall = isVisionHasBall(pVision, vecNumber);     // 视觉信息检测到球
	isMeHasBall = isMechHasBall || visionHasBall;
	if (isMeHasBall) {
		meHasBall = meHasBall >= maxMeHasBall ? maxMeHasBall : meHasBall + 1;
		meLoseBall = 0;
	}
	else {
		meHasBall = 0;
		meLoseBall = meLoseBall >= maxMeHasBall ? maxMeHasBall : meLoseBall + 1;
	}

	int new_state = state();
	switch (state()) {
		case BEGINNING:
			new_state = GET;
			break;
		case GET:
			if (meHasBall > meHasBallMaxCnt) {
				new_state = TURN;
			}
			break;
		case TURN:
			if (abs(Utils::Normalize(me.Dir() - finalDir)) < adjustPre) {
				new_state = END;
			}
			break;
		case END:
			break;
		default:
			new_state = BEGINNING;
			break;
		}

	setState(new_state);

	//cout << "state: " << new_state << endl;

	DribbleStatus::Instance()->setDribbleCommand(robotNum, dribblePower);
	if (GET == state()) {
		setSubTask(PlayerRole::makeItSlowGetBall(robotNum, faceDir, flag | PlayerStatus::DRIBBLING));
	}
	else if (TURN == state()) {
		setSubTask(PlayerRole::makeItSimpleGoto(robotNum, me.Pos(), finalDir, CVector(0,0), 3.5, flag | PlayerStatus::DRIBBLING));
	}
	else if (END == state()) {
		setSubTask(PlayerRole::makeItSlowGetBall(robotNum, finalDir, flag | PlayerStatus::DRIBBLING));
	}
	//cout << "state : " << state() << endl;
	_lastCycle = pVision->Cycle();
	return CStatedTask::plan(pVision);
}

CPlayerCommand* CDribbleTurn::execute(const CVisionModule* pVision) {
	if(subTask()) {
		return subTask()->execute(pVision);
	}
	return NULL;
}

bool CDribbleTurn::isVisionHasBall(const CVisionModule* pVision, const int vecNumber) {
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const MobileVisionT& ball = pVision->Ball();
	double visionJudgDist = 10;
	bool distVisionHasBall = CVector(me.Pos() - ball.Pos()).mod() <= visionJudgDist;
	// cout << CVector(me.Pos() - ball.Pos()).mod() << endl;
	bool dirVisionHasBall;
	double meDir = me.Dir();
	double me2Ball = (ball.Pos() - me.Pos()).dir();
	double meDir_me2Ball_Diff = abs(Utils::Normalize((meDir - me2Ball)));
	if (meDir_me2Ball_Diff < Param::Math::PI / 9.0) {
		dirVisionHasBall = true;
	}
	else {
		dirVisionHasBall = false;
	}
	bool isVisionPossession = dirVisionHasBall && distVisionHasBall;
	return isVisionPossession;
}