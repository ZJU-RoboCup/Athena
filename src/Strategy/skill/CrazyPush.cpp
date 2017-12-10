#include "CrazyPush.h"
#include <VisionModule.h>
#include <WorldModel.h>
#include <iostream>
#include "utils.h"
#include "skill/Factory.h"

namespace {}
CCrazyPush::CCrazyPush() {}
CCrazyPush::~CCrazyPush() {}

void CCrazyPush::plan(const CVisionModule* pVision) {
	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1) { setState(BEGINNING); }
	int _executor  = task().executor;
	double faceDir = task().player.angle;
	const PlayerVisionT& me = pVision->OurPlayer(_executor);

	CGeoPoint carzyTarget = me.Pos() + Utils::Polar2Vector(3, faceDir);

	int new_state = state();
	switch(state()) {
		case  BEGINNING:
			new_state = CRAZY_PUSH;
			break;
		case  CRAZY_PUSH:
			break;
		default:
			new_state = CRAZY_PUSH;
			break;
	}
	setState(new_state);

	if (CRAZY_PUSH == state()) {
		setSubTask(PlayerRole::makeItSimpleGoto(_executor, carzyTarget, faceDir, CVector(0, 0), 0, 0));
	}
	_lastCycle = pVision->Cycle();	
	return CStatedTask::plan(pVision);
}

CPlayerCommand* CCrazyPush ::execute(const CVisionModule* pVision) {
	if( subTask() ) { return subTask()->execute(pVision); }
	if( _directCommand ) { return _directCommand; }
	return 0;
}