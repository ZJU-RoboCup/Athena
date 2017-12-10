#include "GoAndTurnKickV3.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include "ControlModel.h"
#include "KickStatus.h"
#include <utils.h>
#include <DribbleStatus.h>
#include <ControlModel.h>
#include "GDebugEngine.h"

//****************************
// 2014-03-21 yys

namespace {
	//配置状态机
	enum TestCircleBall {
		GO = 1,
		GOAROUND,
		ADJUST,
		KICK,
	};
	bool IS_SIMULATION = false;
	int sinT = 30;
}

CGoAndTurnKickV3::CGoAndTurnKickV3() {
	DECLARE_PARAM_READER_BEGIN(General)
	READ_PARAM(IS_SIMULATION)
	DECLARE_PARAM_READER_END
	//_lastCycle = 0;
	count = 0;
	angeDiff_Use = Param::Math::PI * 2;
	CircleCounter = 0.0;
	adjustCount = 0;
}

void CGoAndTurnKickV3::plan(const CVisionModule* pVision) {
	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1) {
		setState(BEGINNING);
		count = 0;
	}
	const int vecNumber = task().executor;
	circleNum = (int) task().ball.angle;
	double finalDir = task().player.angle;  // 球员全局目标朝向点
	double fixAngle = task().player.rotate_speed;
	double maxAcc =  task().player.max_acceleration;
	double maxDcc = task().player.max_acceleration;
	double radius = task().player.rotvel;  // 绕球转动半径
	double numPerCir = task().player.chipkickpower;
	double gotoPre = task().player.kickpower;
	double gotoDist = task().player.max_deceleration;
	double adjustPre = task().player.speed_x;

	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const CVector self2ball = ballPos - me.Pos();
	const CVector ball2self = me.Pos() - ballPos;
	double faceDir = Utils::Normalize((ball.Pos() - me.Pos()).dir());

	TaskT myTask;

	myTask.player.flag = myTask.player.flag | PlayerStatus::DODGE_OUR_DEFENSE_BOX;
	
	int new_state = state();
	int old_state;
	DribbleStatus::Instance()->setDribbleCommand(vecNumber, 0);
	switch (state()) {
		case BEGINNING:		
			new_state = GO;
			target = ball.Pos() + Utils::Polar2Vector(radius, Utils::Normalize(CVector(me.Pos() - ball.Pos()).dir()));
			break;
		case GO:
			if (me.Pos().dist(target) < 10) {
				new_state = GOAROUND;
			}
			break;
		case GOAROUND:
			if (CircleCounter >= 65 * circleNum && abs(Utils::Normalize((ball.Pos() - me.Pos()).dir() - finalDir)) < gotoPre) {
				new_state = ADJUST;
			}
			else {
				CircleCounter++;
				//std::cout << "CircleCounter : " << CircleCounter << std::endl;
			}
			break;
		case ADJUST:
			if (abs(Utils::Normalize(me.Dir() - finalDir)) < adjustPre || adjustCount >= 30) {
				new_state = KICK;
			}
			adjustCount++;
			CircleCounter = 0;
			//std::cout << "adjustCount : " << adjustCount << std::endl;
			break;
		case KICK: 
			CircleCounter = 0;
			adjustCount = 0;
			break;
		default:
			new_state = BEGINNING;
			break;
		}

	setState(new_state);

	if (GO == state()) {
		setSubTask(PlayerRole::makeItGoto(vecNumber, target, faceDir, CVector(0, 0), 0, task().player.flag, 0));
	}
	else if (GOAROUND == state()) {
		if (circleNum == 0) {
			if (Utils::Normalize(finalDir - self2ball.dir()) > 0) {
				faceDir = faceDir + Param::Math::PI / 3.7;
			}
			else {
				faceDir = faceDir - Param::Math::PI / 3.7;
			}
			{
				myTask.executor = vecNumber;
				myTask.player.pos = target;
				myTask.player.angle = faceDir;
				myTask.player.max_acceleration = maxAcc;
				myTask.player.max_deceleration = maxDcc;
			}
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
			if (Utils::Normalize(finalDir - self2ball.dir()) > 0) {
				target = ball.Pos() + Utils::Polar2Vector(18, Utils::Normalize((me.Pos() - ball.Pos()).dir() + Param::Math::PI / numPerCir));
			} 
			else {
				target = ball.Pos() + Utils::Polar2Vector(18, Utils::Normalize((me.Pos() - ball.Pos()).dir() - Param::Math::PI / numPerCir));
			}
		}
		else {
			if (ball.Pos().y() < 0) {
				faceDir = faceDir + fixAngle;
				target = ball.Pos() + Utils::Polar2Vector(gotoDist, Utils::Normalize((me.Pos() - ball.Pos()).dir() + Param::Math::PI / numPerCir));
			}
			else {
				faceDir = faceDir - fixAngle;
				target = ball.Pos() + Utils::Polar2Vector(gotoDist, Utils::Normalize((me.Pos() - ball.Pos()).dir() - Param::Math::PI / numPerCir));
			}
			{
				myTask.executor = vecNumber;
				myTask.player.pos = target;
				myTask.player.angle = faceDir;
				myTask.player.max_acceleration = maxAcc;
				myTask.player.max_deceleration = maxDcc;
			}
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
		}
	}
	else if (ADJUST == state()) {
		{
			myTask.executor = vecNumber;
			myTask.player.pos = target;
			myTask.player.angle = faceDir;
			myTask.player.max_acceleration = maxAcc;
			myTask.player.max_deceleration = maxDcc;
		}
		setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
	}
	else if (KICK == state()) {
		myTask.executor = vecNumber;
		myTask.player.angle = finalDir;
		myTask.player.max_acceleration = maxAcc;
		myTask.player.max_deceleration = maxDcc;

		setSubTask(TaskFactoryV2::Instance()->NoneTrajGetBall(myTask));
	}
	//cout << "state : " << state() << endl;
	_lastCycle = pVision->Cycle();
	return CStatedTask::plan(pVision);
}

CPlayerCommand* CGoAndTurnKickV3::execute(const CVisionModule* pVision) {
	if(subTask()) {
		return subTask()->execute(pVision);
	}
	return NULL;
}