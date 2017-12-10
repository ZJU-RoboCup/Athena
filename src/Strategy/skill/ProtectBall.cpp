#include "GDebugEngine.h"
#include "skill/Factory.h"
#include <utils.h>
#include "param.h"
#include <cmath>
#include "utils.h"
#include <VisionModule.h>
#include "BallStatus.h"
#include "ProtectBall.h"
#include "TouchKickPos.h"
#include "WorldModel.h"
#include "BestPlayer.h"
#include "BallSpeedModel.h"

namespace
{
	enum ProtectBall_State{
		Slow_Ball=1,
		Approach_Ball = 2,
		Transit_State=3,
		Protect_Ball = 4,
		Shoot_Ball=5,
		Enter_DefendArea=6,
	};
	bool verBos = false;
	const int State_Counter_Num=3;
};

CProtectBall::CProtectBall()
{
	_lastCycle = 0;
	_stateCounter=0;
}

void CProtectBall::plan(const CVisionModule* pVision)
{

	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1){
		setState(BEGINNING);
		_stateCounter=0;
		//Í¼Ïñ»á²»ÎÈ¶¨
	}
	const int runner = task().executor;
	int flags = task().player.flag;

	const PlayerVisionT& self = pVision->OurPlayer(runner);
	const MobileVisionT& ball = pVision->Ball();
	CVector self2ball = ball.Pos() - self.Pos();
	const double ballVelDir = ball.Vel().dir();
	double antiBallVelDir = Utils::Normalize(ballVelDir + Param::Math::PI);
	double ballSpeed = ball.Vel().mod();
	double balltoSelfDir=(self.Pos()-ball.Pos()).dir();
	double balltoSelfDist=(self.Pos()-ball.Pos()).mod();

	CGeoLine ballMovingLine = CGeoLine(ball.Pos(),ball.Pos()+Utils::Polar2Vector(1000,ball.Vel().dir()));

	double CrossThreld=20;
	CGeoSegment ballMovingSeg = CGeoSegment(ball.Pos()+Utils::Polar2Vector(CrossThreld,ball.Vel().dir()),ball.Pos()+Utils::Polar2Vector(1000,ball.Vel().dir()));
	CGeoPoint projMePos = ballMovingLine.projection(self.Pos());
	double me2ballMovingLineDist = projMePos.dist(self.Pos());

	const int theirBestPlayer=BestPlayer::Instance()->getTheirBestPlayer();

	CGeoPoint ourGoalPos=CGeoPoint(-Param::Field::PITCH_LENGTH/2,0);
	double ourGoaltoBallDir=(ball.Pos()-ourGoalPos).dir();
	CGeoPoint theirGoalPos=CGeoPoint(Param::Field::PITCH_LENGTH/2,0);
	double ballToTheirGoaldir=(theirGoalPos-ball.Pos()).dir();

	double predictTime=60;
	if (ballSpeed>250){
		predictTime=60;
	}else if (ballSpeed>200){
		predictTime=40;
	}else if (ballSpeed>100){
		predictTime=20;
	}else{
		predictTime=10;
	}
	CGeoPoint predictBallPosSmallCycle=BallSpeedModel::Instance()->posForTime(20,pVision);
	CGeoPoint predictBallPos=BallSpeedModel::Instance()->posForTime(predictTime,pVision);
	CGeoSegment predictBallMovSeg=CGeoSegment(predictBallPos,predictBallPos+Utils::Polar2Vector(1000,ball.Vel().dir()));

	bool isBallCross=!ballMovingSeg.IsPointOnLineOnSegment(projMePos);
	bool canJumpTransit=fabs(Utils::Normalize(self2ball.dir()-ballVelDir))>=Param::Math::PI/6 && isBallCross;
	bool isBallEntireCross=fabs(Utils::Normalize(self2ball.dir()-ballVelDir))<Param::Math::PI/6 && isBallCross &&me2ballMovingLineDist>30;

	////judge inOtherSide
	bool inOtherSide=false;
	if (verBos)
		std::cout<<balltoSelfDir<<" "<<ballVelDir<<" "<<Utils::Normalize(ourGoaltoBallDir+Param::Math::PI)<<std::endl;
	if (abs(balltoSelfDir-ballVelDir)>Param::Math::PI*10/180&&
		Utils::Normalize(balltoSelfDir-ballVelDir)/Utils::Normalize(ourGoaltoBallDir+Param::Math::PI-ballVelDir)<0){
			inOtherSide=true;
	}else{
		inOtherSide=false;
	}


	bool canEnterShoot= ballSpeed > 50 && me2ballMovingLineDist < 25;
	CGeoLine defendLine=CGeoSegment(CGeoPoint(-Param::Field::PITCH_LENGTH/2,-Param::Field::PENALTY_AREA_WIDTH/2.5),CGeoPoint(-Param::Field::PITCH_LENGTH/2,Param::Field::PENALTY_AREA_WIDTH/2.5));
	CGeoSegment defendSeg=CGeoSegment(CGeoPoint(-Param::Field::PITCH_LENGTH/2,-Param::Field::PENALTY_AREA_WIDTH/2.5),CGeoPoint(-Param::Field::PITCH_LENGTH/2,Param::Field::PENALTY_AREA_WIDTH/2.5));
	CGeoLineLineIntersection inter=CGeoLineLineIntersection(defendLine,ballMovingLine);
	bool canEnterDefendArea=false;
	if (ballSpeed>100)
	{
		if (inter.Intersectant()){
			CGeoPoint interPoint=inter.IntersectPoint();
			if (defendSeg.IsPointOnLineOnSegment(interPoint))
			{
				canEnterDefendArea=true;
			}
		}
	}
	//canEnterDefendArea=false;
	//cout<<"newState"<<state()<<" "<<"defend"<<canEnterDefendArea<<endl;
	//canEnterShoot=false;
	int new_state = state(), old_state = state();
	do {
		old_state = new_state;
		switch (old_state){
		case BEGINNING:
			{
				if (ballSpeed<30){
					new_state =Slow_Ball;
				}else if(canEnterShoot){
					new_state=Shoot_Ball;
				}else if (canEnterDefendArea){
					new_state=Enter_DefendArea;
				}
				else if(!isBallCross){
					if(verBos) cout<<"BEGINNING-->Approach_Ball"<<endl;
					new_state = Approach_Ball;
				}else{
					if (!isBallEntireCross){
						new_state=Transit_State;
					}else{
						if(verBos) cout<<"BEGINNING-->Protect_Ball"<<endl;
						new_state = Protect_Ball;
					}
				}
				break;
			}
		case Approach_Ball:
			if (ballSpeed<30){
				new_state =Slow_Ball;
			}else if (canEnterDefendArea){
				new_state=Enter_DefendArea;
			}else if(canJumpTransit){
				new_state=Transit_State;
			}else if(isBallEntireCross){
				if(verBos) cout<<"Approach-->Protect"<<endl;
				new_state = Protect_Ball;
			} 
			break;
		case Transit_State:
			if (ballSpeed<30){
				new_state =Slow_Ball;
			}else if (canEnterDefendArea){
				new_state=Enter_DefendArea;
			}else if(!isBallCross){
				new_state=Approach_Ball;
			}else if(isBallEntireCross){
				if(verBos) cout<<"Approach-->Protect"<<endl;
				new_state = Protect_Ball;
			} 
			break;
		case Protect_Ball:
			if (ballSpeed<30){
				new_state =Slow_Ball;
			}else if (canEnterDefendArea){
				new_state=Enter_DefendArea;
			}else if(!isBallCross){
				if(verBos) cout<<"Protect-->Approach"<<endl;
				new_state = Approach_Ball;
			} 
			break;
		case Shoot_Ball:
			if (!canEnterShoot&&ballSpeed>50)
			{
				new_state=Approach_Ball;
			}
			break;
		case Enter_DefendArea:
			if(!canEnterDefendArea&&ballSpeed>80){
				new_state=Approach_Ball;
			}
			break;
		default:
			new_state = BEGINNING;
			break;
		}
	} while (false);
	if (_stateCounter==0){
		setState(new_state);
		_stateCounter++;
	}else{
		if (new_state==state()){
			_stateCounter=min(State_Counter_Num,_stateCounter+1);
		}else{
			_stateCounter=max(0,_stateCounter-1);
		}
	}

	TaskT protectTask(task());

	double approachBallDir=Utils::Normalize(ourGoaltoBallDir);
	if (ballSpeed>30){
		approachBallDir=Utils::Normalize(ballVelDir+Utils::Normalize(ourGoaltoBallDir-ballVelDir)/2);
	}
	if (inOtherSide){
		approachBallDir=(projMePos-self.Pos()).dir();
	}


	double ProtectBallDir=ourGoaltoBallDir;
	if (ballSpeed>30 &&ball.Valid()){
		ProtectBallDir=ballVelDir;
	}
	if (pVision->TheirPlayer(theirBestPlayer).Valid()){
		ProtectBallDir=(ball.Pos()-pVision->TheirPlayer(theirBestPlayer).Pos()).dir();
	}
	if (fabs(ballVelDir)<Param::Math::PI/3){
		ProtectBallDir=ballVelDir;
	}

	double SetAcc=600-ballSpeed;

	switch (state()){
	case Slow_Ball:
		{
			protectTask.player.pos=ball.Pos()+Utils::Polar2Vector(3*Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER,Utils::Normalize(ProtectBallDir+Param::Math::PI));;
			protectTask.player.angle=ProtectBallDir;
			protectTask.player.vel=CVector(0,0);
			protectTask.player.rotvel=0;
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(protectTask));
		}
		break;
	case Approach_Ball:
		{
			//if (ballSpeed>250){
			//	predictBallPos=ball.Pos();
			//}else if(ballSpeed<50){
			//	predictBallPos=ball.Pos();
			//}

			if (ballSpeed>50&&fabs(Utils::Normalize(self.Vel().dir()-ballVelDir))<Param::Math::PI/2){
				predictBallPos=ball.Pos();
			}
			double distFactor=5;
			if (fabs(ballVelDir)>Param::Math::PI*60/180&&fabs(ballVelDir)<Param::Math::PI*120/180){
				distFactor=8;
			}
			//GDebugEngine::Instance()->gui_debug_x(predictBallPos,COLOR_RED);
			CGeoPoint approachBallPos=predictBallPos+Utils::Polar2Vector(distFactor*Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER,Utils::Normalize(approachBallDir+Param::Math::PI)); 

			protectTask.player.pos=approachBallPos;
			protectTask.player.angle=approachBallDir;
			protectTask.player.vel=CVector(0,0);
			protectTask.player.rotvel=0;
			protectTask.player.max_acceleration=SetAcc;
			protectTask.player.max_deceleration=1000;

			setSubTask(TaskFactoryV2::Instance()->GotoPosition(protectTask));
			//setSubTask(PlayerRole::makeItGoto(runner,approachBallPos,approachBallDir,CVector(0,0),0,flags));
		}
		break;
	case Transit_State:
		{
			CGeoPoint centerPoint=CGeoPoint((self.Pos().x()+predictBallPosSmallCycle.x())/2,(self.Pos().y()+predictBallPosSmallCycle.y())/2);
			if (self2ball.mod()<15)
			{
				protectTask.player.pos=ball.Pos()+Utils::Polar2Vector(2*Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER,Utils::Normalize(ballVelDir+Param::Math::PI));
			}
			protectTask.player.pos=centerPoint;
			protectTask.player.angle=ballVelDir;
			protectTask.player.vel=CVector(0,0);
			protectTask.player.rotvel=0;
			protectTask.player.max_acceleration=400;
			protectTask.player.max_deceleration=1000;
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(protectTask));
		}
		break;
	case Protect_Ball:
		{

			CGeoPoint ProtectBallPos=ball.Pos()+Utils::Polar2Vector(3*Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER,Utils::Normalize(ProtectBallDir+Param::Math::PI)); 
			protectTask.player.pos=ProtectBallPos;
			protectTask.player.angle=ProtectBallDir;
			protectTask.player.vel=CVector(0,0);
			protectTask.player.rotvel=0;
			//protectTask.player.max_acceleration=SetAcc;
			//protectTask.player.max_deceleration=1000;
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(protectTask));
		}
		break;
	case Shoot_Ball:
		{
			setSubTask(PlayerRole::makeItShootBall(runner,ballToTheirGoaldir,false,Param::Math::PI*10/180,1200,0));
		}
		break;
	case Enter_DefendArea:
		{
			setSubTask(PlayerRole::makeItShootBall(runner,ballToTheirGoaldir,false,Param::Math::PI*10/180,1200,0));
		}
		break;
	default:
		break;
	}
	//if(self2ball.mod()>100 && me2ballMovingLineDist>100){
	//	setSubTask(PlayerRole::makeItNoneTrajGetBall(runner, approachBallDir, CVector(0,0), 0, 10));
	//}

	//GDebugEngine::Instance()->gui_debug_line( self.Pos(),self.Pos()+Utils::Polar2Vector(1000,finalDir),COLOR_BLACK);

	_lastCycle = pVision->Cycle();

	CPlayerTask::plan(pVision);
}

CPlayerCommand* CProtectBall::execute(const CVisionModule* pVision)
{
	if (_directCommand) {
		return _directCommand;
	}

	if (subTask()) {
		return subTask()->execute(pVision);
	}

	return NULL;
}
