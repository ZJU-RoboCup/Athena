#include "GDebugEngine.h"
#include "skill/Factory.h"
#include <utils.h>
#include "param.h"
#include <cmath>
#include "utils.h"
#include <VisionModule.h>
#include "BallStatus.h"
#include "ForceStartRush.h"
#include "TouchKickPos.h"
#include "WorldModel.h"

namespace
{
	enum ForceStartRush_State{
		CrazyRush=1,
		AdjustShoot=2,
	};
	bool verBos = false;
	const int State_Counter_Num=3;
};

CForceStartRush::CForceStartRush()
{
	_lastCycle = 0;
	_stateCounter=0;
}

void CForceStartRush::plan(const CVisionModule* pVision)
{
	const int runner = task().executor;
	int flags = task().player.flag;
	double shootDir =task().player.angle;

	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1){
		setState(BEGINNING);
		_stateCounter=0;
	}

	const PlayerVisionT& self = pVision->OurPlayer(runner);
	const MobileVisionT& ball = pVision->Ball();
	CVector self2ball = ball.Pos() - self.Pos();
	double selftoballDist=self2ball.mod();
	const double ballVelDir = ball.Vel().dir();
	double antiBallVelDir = Utils::Normalize(ballVelDir + Param::Math::PI);
	double ballSpeed = ball.Vel().mod();
	double balltoSelfDir=(self.Pos()-ball.Pos()).dir();


	CGeoLine ballMovingLine = CGeoLine(ball.Pos(),ball.Pos()+Utils::Polar2Vector(1000,ball.Vel().dir()));

	double CrossThreld=10;
	CGeoSegment ballMovingSeg = CGeoSegment(ball.Pos()+Utils::Polar2Vector(CrossThreld,ball.Vel().dir()),ball.Pos()+Utils::Polar2Vector(1000,ball.Vel().dir()));
	CGeoPoint projMePos = ballMovingLine.projection(self.Pos());
	double me2ballMovingLineDist = projMePos.dist(self.Pos());
	bool isBallCross=!ballMovingSeg.IsPointOnLineOnSegment(projMePos);

	CGeoPoint ourGoalPos=CGeoPoint(-Param::Field::PITCH_LENGTH/2,0);
	double ourGoaltoBallDir=(ball.Pos()-ourGoalPos).dir();
	CGeoPoint theirGoalPos=CGeoPoint(Param::Field::PITCH_LENGTH/2,0);
	double ballToTheirGoaldir=(theirGoalPos-ball.Pos()).dir();
	CGeoPoint kickerHeaderPos = self.Pos() + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER,self.Dir());


	double meHeadtoBallDir=(ball.Pos()-kickerHeaderPos).dir();
	double correctDir=Utils::Normalize(meHeadtoBallDir-self.Dir());


	int AccSpeed=1000;
	
	double adjustDir=shootDir;//meHeadtoBallDir;

	//if (abs(correctDir)>Param::Math::PI*5/180)
	//{
	//	correctDir=correctDir/abs(correctDir)*min(abs(correctDir*5),Param::Math::PI*90/180);
	//	//cout<<correctDir<<" "<<adjustDir+min(correctDir*5,Param::Math::PI*60/180)<<endl;
	//	adjustDir=Utils::Normalize(adjustDir+correctDir);
	//}
	//GDebugEngine::Instance()->gui_debug_line(self.RawPos(),self.RawPos()+Utils::Polar2Vector(1000,adjustDir),COLOR_BLUE);
	//GDebugEngine::Instance()->gui_debug_line(self.RawPos(),self.RawPos()+Utils::Polar2Vector(1000,self.Dir()),COLOR_BLACK);
	TaskT rushTask(task());
	rushTask.player.pos=ball.Pos()+Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER,Utils::Normalize(adjustDir+Param::Math::PI));
	rushTask.player.angle=adjustDir;
	rushTask.player.vel=Utils::Polar2Vector(1000,adjustDir);
	rushTask.player.rotvel=0;
	rushTask.player.max_acceleration=AccSpeed;




	TaskT adjustTask(task());
	adjustTask.player.pos=ball.Pos()+Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER,Utils::Normalize(adjustDir+Param::Math::PI));
	adjustTask.player.angle=adjustDir;
	adjustTask.player.vel=Utils::Polar2Vector(1000,adjustDir);
	adjustTask.player.rotvel=0;
	adjustTask.player.max_acceleration=AccSpeed;

	int new_state = state(), old_state = state();
	do {
		old_state = new_state;
		switch (old_state){
		case BEGINNING:
			{
				if (selftoballDist>20){
					if(verBos) cout<<"BEGINNING-->CrazyRush"<<endl;
					new_state =CrazyRush;
				}else{
					if(verBos) cout<<"BEGINNING-->AdjustShoot"<<endl;
					new_state = AdjustShoot;
				}
				break;
			}
		case CrazyRush:
			if (selftoballDist<=20){
				if(verBos) cout<<"CrazyRush-->AdjustShoot"<<endl;
				new_state =AdjustShoot;
			}
			break;
		case AdjustShoot:
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

	//double rotVel=(adjustDir-self.Dir())*120;
	switch (state()){
	case CrazyRush:
		{
			//setSubTask(TaskFactoryV2::Instance()->GotoPosition(rushTask));
			setSubTask(TaskFactoryV2::Instance()->NoneTrajGetBall(rushTask));
// 			CVector rushVel(ball.Pos()-self.Pos());
// 			rushVel=rushVel/rushVel.mod()*350;
// 			setSubTask(PlayerRole::makeItRun(runner,rushVel.x(),rushVel.y(),rotVel));
		}
		break;
	case AdjustShoot:
		{
			//setSubTask(TaskFactoryV2::Instance()->GotoPosition(adjustTask));
			setSubTask(TaskFactoryV2::Instance()->NoneTrajGetBall(adjustTask));
			//setSubTask(PlayerRole::makeItNoneTrajGetBall(runner,shootDir,CVector(0,0),0,-2));
			//setSubTask(TaskFactoryV2::Instance()->NoneTrajGetBall(adjustTask));
// 			CVector rushVel(ball.Pos()-self.Pos());
// 			rushVel=rushVel/rushVel.mod()*350;
// 			setSubTask(PlayerRole::makeItRun(runner,rushVel.x(),rushVel.y(),0));
		}
		break;
	default:
		break;
	}

	_lastCycle = pVision->Cycle();

	CPlayerTask::plan(pVision);
}

CPlayerCommand* CForceStartRush::execute(const CVisionModule* pVision)
{
	if (_directCommand) {
		return _directCommand;
	}

	if (subTask()) {
		return subTask()->execute(pVision);
	}

	return NULL;
}
