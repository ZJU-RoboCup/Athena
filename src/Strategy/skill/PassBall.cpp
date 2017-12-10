#include "GDebugEngine.h"
#include "skill/Factory.h"
#include <utils.h>
#include "param.h"
#include <cmath>
#include <utils.h>
#include <VisionModule.h>
#include "BallStatus.h"
#include "PassBall.h"
#include "TouchKickPos.h"
#include "WorldModel.h"

namespace
{
	enum PassBall_State{
		CHASE_KICK = 1,
		INTERCEPT_KICK,
	};
	bool verBos = false;
	int Touch2ChaseCnt = 0;
	int Touch2InterCnt = 0;
};

CPassBall::CPassBall()
{
	_lastCycle = 0;
}

void CPassBall::plan(const CVisionModule* pVision)
{
	const int runner = task().executor;
	int flags = task().player.flag;
	double finalDir = task().player.angle;
	const PlayerVisionT& self = pVision->OurPlayer(runner);
	const MobileVisionT& ball = pVision->Ball();
	CVector self2ball = ball.Pos() - self.Pos();
	const double ballVelDir = ball.Vel().dir();
	double antiBallVelDir = Utils::Normalize(ballVelDir + Param::Math::PI);
	double ballSpeed = ball.Vel().mod();
	CGeoSegment shootSeg(self.Pos(), self.Pos() + Utils::Polar2Vector(1500, finalDir));
	CGeoPoint ballShootSegInterP = shootSeg.projection(ball.Pos());
	CGeoLine ballMovingLine = CGeoLine(ball.Pos(),ball.Pos()+Utils::Polar2Vector(1000,ball.Vel().dir()));
	CGeoPoint projMePos = ballMovingLine.projection(self.Pos());
	double me2ballMovingLineDist = projMePos.dist(self.Pos());

	bool  isBallOnShootSeg = shootSeg.IsPointOnLineOnSegment(ballShootSegInterP);
	double a_m2t_2_m2b = fabs(Utils::Normalize(self2ball.dir() - finalDir)) * 180 / Param::Math::PI;
	double a_m2t_2_bsr = fabs(Utils::Normalize(antiBallVelDir - finalDir)) * 180 / Param::Math::PI;
	double a_m2t_2_bsl = fabs(Utils::Normalize(ballVelDir - finalDir)) * 180 / Param::Math::PI; 
	double a_m2b_2_bsr = fabs(Utils::Normalize(self2ball.dir() - antiBallVelDir)) * 180 / Param::Math::PI;

	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1
		|| BallStatus::Instance()->IsBallKickedOut(runner)
		|| _lastRunner != runner){
		cout<<"Clear: get into PassBall !"<<endl;
		setState(BEGINNING);
		Touch2ChaseCnt = 0;
		Touch2InterCnt = 0;
	}

	int new_state = state(), old_state = state();
	do {
		old_state = new_state;
		switch (old_state){
			case BEGINNING:
				{
					if( ballSpeed<25 || (a_m2t_2_m2b<45 && a_m2t_2_bsl<60 && fabs(ballVelDir)<Param::Math::PI / 3.5)){
						if(verBos) cout<<"BEGINNING-->Chase"<<endl;
						new_state = CHASE_KICK;
					} else{
						if(verBos) cout<<"BEGINNING-->Inter"<<endl;
						new_state = INTERCEPT_KICK;
					}
					break;
				}
			case CHASE_KICK:
				if( ballSpeed>100 && (!isBallOnShootSeg 
					|| (a_m2t_2_m2b>110 && self2ball.mod()>50) 
					|| (fabs(ballVelDir)>Param::Math::PI / 4 && me2ballMovingLineDist > 60))){
					if(verBos) cout<<"Chase-->Inter"<<endl;
					new_state = INTERCEPT_KICK;
				} 
				break;
			case INTERCEPT_KICK:
				if(ballSpeed<15 || (a_m2t_2_m2b<45 && a_m2t_2_bsl<60 && fabs(ballVelDir)<Param::Math::PI / 10)){
					if(verBos) cout<<"Inter-->Chase"<<endl;
					new_state = CHASE_KICK;
				} 
				break;
			default:
				new_state = BEGINNING;
				break;
		}
	} while (false);
	setState(new_state);

	flags = flags | PlayerStatus::DODGE_OUR_DEFENSE_BOX;
	switch (state()){
		case CHASE_KICK:
			{
				setSubTask(PlayerRole::makeItNoneTrajGetBall(runner, finalDir, CVector(0,0), flags, -2));
			}
			break;
		case INTERCEPT_KICK:
			{	
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0,0),"AAAAAAAAAAAAAAAAAAAA");
				setSubTask(PlayerRole::makeItInterKickV3(runner,antiBallVelDir,flags));
			}
			break;
		default:
			setSubTask(PlayerRole::makeItChaseKickV1(runner, finalDir, flags));
			break;
	}

	GDebugEngine::Instance()->gui_debug_line( self.Pos(),self.Pos()+Utils::Polar2Vector(1000,finalDir),COLOR_BLACK);

	_lastCycle = pVision->Cycle();
	_lastRunner = runner;

	CPlayerTask::plan(pVision);
}

CPlayerCommand* CPassBall::execute(const CVisionModule* pVision)
{
	if (_directCommand) {
		return _directCommand;
	}

	if (subTask()) {
		return subTask()->execute(pVision);
	}

	return NULL;
}
