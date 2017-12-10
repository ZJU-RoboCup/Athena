#include "ReceivePass.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include <utils.h>
#include <DribbleStatus.h>
#include <RobotSensor.h>
#include <CommandFactory.h>

namespace{	
	enum receivePassState {
		GOTOPROJ =1,
		GETBALL,
	};
	int infrareCnt = 0;
	int prj2waitCnt = 0;
	int MAX_INFRARE_CNT = 2;
	CGeoPoint projMe = CGeoPoint(0,0);
	bool VerBose = false;
}

CReceivePass::CReceivePass()
{
	_lastCycle = 0;
	_directCommand = NULL;
}

void CReceivePass::plan(const CVisionModule* pVision)
{
	_directCommand = NULL;

	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const int vecNumber = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const CVector self2ball = ballPos - me.Pos();
	const CVector ball2self = me.Pos() - ballPos;
	const double finalDir = task().player.angle;
	CGeoLine ballMoveingLine(ball.RawPos(), ball.Vel().dir());
	CGeoPoint projMe = ballMoveingLine.projection(me.Pos());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + Param::Math::PI);

	
	bool isInfraredOn = RobotSensor::Instance()->IsInfraredOn(vecNumber);
	int flags = task().player.flag | PlayerStatus::DODGE_OUR_DEFENSE_BOX | PlayerStatus::ALLOW_DSS | PlayerStatus::DODGE_BALL;

	bool isBallMovingToMe;
	isBallMovingToMe = abs(Utils::Normalize((me.Pos() - ball.RawPos()).dir() - ball.Vel().dir())) < Param::Math::PI / 6;

	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1
		|| _lastRunner != vecNumber){
		setState(BEGINNING);
		infrareCnt = 0;
		prj2waitCnt = 0;
	}

	int new_state = state();
	int old_state;
	_mePos=me.RawPos();
	do{
		old_state = new_state;
		switch ( old_state )
		{
			case BEGINNING:
				{		
					 if (ball.Vel().mod() > 50 && isBallMovingToMe)
					 {
						  new_state = GOTOPROJ;
						 _mePos=me.RawPos();
					 }
					 else{
						 new_state=GETBALL;
					 }
				}
				break;
			case GOTOPROJ:
				break;
			case GETBALL:
				if (ball.Vel().mod() > 50 && isBallMovingToMe)
				{
					new_state = GOTOPROJ;
					_mePos=me.RawPos();
				}	
				break;
			default:
				new_state = GOTOPROJ;
				_mePos=me.RawPos();
				break;
		}
	} while ( old_state != new_state );

	setState(new_state);

	//open dribble when the ball is 100cm from the car 8/12/2016 Alan
	int power = 0;
	CGeoPoint newPos;
	power = (me.Pos() - ballPos).mod()<100 ? 3 : 0;

	DribbleStatus::Instance()->setDribbleCommand(vecNumber,power);
	
	if (GOTOPROJ == state()){
		if (ball.Vel().mod() > 50 && isBallMovingToMe){
			if (VerBose) {
				cout << "in ReceivePass : 1" << endl;
			}
			CGeoPoint receivePos=ballMoveingLine.projection(_mePos);
			setSubTask(PlayerRole::makeItGoto(vecNumber, receivePos, antiBallVelDir, flags, 0));
		} 
		else{
			if (VerBose) {
				cout << "in ReceivePass : 2" << endl;
			}
			//setSubTask(PlayerRole::makeItInterKickV6(vecNumber, finalDir, flags));
			setSubTask(PlayerRole::makeItNoneTrajGetBall(vecNumber, finalDir, CVector(0,0), flags,1));
			//setSubTask(PlayerRole::makeItChaseKickV2(vecNumber, finalDir, flags));
			
		}
	} else if (GETBALL == state()){
		if (VerBose) {
			cout << "in ReceivePass : 3" << endl;
		}
		setSubTask(PlayerRole::makeItInterKickV3(vecNumber, finalDir,flags));
	} else{
		if (VerBose) {
			cout << "in ReceivePass : 4" << endl;
		}
		setSubTask(PlayerRole::makeItNoneTrajGetBall(vecNumber, finalDir, CVector(0,0), flags));
	}


	_lastCycle = pVision->Cycle();
	_lastRunner = vecNumber;
	CStatedTask::plan(pVision);
}

CPlayerCommand* CReceivePass::execute(const CVisionModule* pVision)
{
	if( _directCommand ){
		return _directCommand;
	} else if (subTask())
	{
		return subTask()->execute(pVision);
	}
	return NULL;
}
