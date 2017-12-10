#include "Goalie2013.h"
#include "DefPos2015.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include <utils.h>
#include <DribbleStatus.h>
#include "KickStatus.h"
#include <RobotSensor.h>
#include "param.h"
#include "BallSpeedModel.h"
#include "WorldModel.h"
#include <TaskMediator.h>
#include <atlstr.h>

namespace{
	const double dangerFactor = 2.5;
	const double PENALTY_BUFFER = 15.0;
	const double DEFEND_PENALTY_BUFFER = 25.0;
	//避门柱参数
	CGeoPoint goalLeft;
	CGeoPoint goalRight;
	CGeoPoint goalCenter;
	CGeoPoint goalCenterFrontLeft;
	CGeoPoint goalCenterFrontRight;
}

CGoalie2013::CGoalie2013()
{
	_clearBall = false;
	goalLeft = CGeoPoint(-Param::Field::PITCH_LENGTH/2,-Param::Field::GOAL_WIDTH/2);
	goalRight = CGeoPoint(-Param::Field::PITCH_LENGTH/2,Param::Field::GOAL_WIDTH/2);
	goalCenter = CGeoPoint(-Param::Field::PITCH_LENGTH/2,0);
	goalCenterFrontLeft = CGeoPoint(-Param::Field::PITCH_LENGTH/2 + 50,-10);
	goalCenterFrontRight = CGeoPoint(-Param::Field::PITCH_LENGTH/2 + 50,10);
}

void CGoalie2013::plan(const CVisionModule* pVision)
{
	int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const MobileVisionT& ball = pVision->Ball();
	CGeoPoint taskPoint;
	taskPoint = DefPos2015::Instance()->getDefPos2015(pVision).getGoaliePos();
	CGeoPoint goalCenter = CGeoPoint(-Param::Field::PITCH_LENGTH/2,0);
	TaskT myTask(task());
	myTask.player.pos = taskPoint;
	if (ball.Valid()){
		myTask.player.angle = CVector(ball.Pos() - me.Pos()).dir();
	}else{
		myTask.player.angle = CVector(me.Pos() - goalCenter).dir();
	}
	myTask.player.flag |= PlayerStatus::QUICKLY;
	int preTime = (int)ball.Vel().mod();
	const PlayerVisionT enemy = pVision->TheirPlayer(BestPlayer::Instance()->getTheirBestPlayer());
	

	if ((needtoAttackEnemy(pVision) || _attackEnemy == true) && _clearBall ==false)
	{ //-------------------------ATTACK ENEMY---------------------------
		if ((ball.Vel().mod() < 80 && Utils::InOurPenaltyArea(ball.Pos(),-30))
			||(ball.Vel().mod() < 30 && Utils::InOurPenaltyArea(ball.Pos(),PENALTY_BUFFER))
			&& ball.Pos().dist(enemy.Pos())>30){
			_clearBall = true;
			_attackEnemy = false;
			double clearBallDir = CVector(ball.Pos() - goalCenter).dir();
			setSubTask(PlayerRole::makeItNoneTrajGetBall(robotNum,clearBallDir,CVector(0,0),PlayerStatus::QUICKLY));
		}else{
			myTask.player.pos = enemy.Pos();
			myTask.player.is_specify_ctrl_method = true;
			myTask.player.specified_ctrl_method = NONE_TRAP;
			myTask.player.max_acceleration = 1200;

			_attackEnemy = true;
			//GDebugEngine::Instance()->gui_debug_x(myTask.player.pos,COLOR_WHITE);
			//cout<<"in attack enemy "<<BestPlayer::Instance()->getTheirBestPlayer()<<endl;
			GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(100,0),"in attack enemy");
			if ((!Utils::InOurPenaltyArea(ball.Pos(),PENALTY_BUFFER) && !DefendUtils::BallIsToPenaltyArea()) || 
				(!(Utils::InOurPenaltyArea(enemy.Pos(),PENALTY_BUFFER) && (ball.Pos().dist(enemy.Pos())>30) || fabs((enemy.Pos()-goalCenter).dir()-ball.Vel().dir()) >Param::Math::PI*2/3))){
				_attackEnemy =false;
				myTask.player.pos = taskPoint;
				if (ball.Valid()){
					myTask.player.angle = CVector(ball.Pos() - me.Pos()).dir();
				}else{
					myTask.player.angle = CVector(me.Pos() - goalCenter).dir();
				}
			}
		}
		setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
	}
	else if (needtoClearBall(pVision) || true == _clearBall )
	{//-------------------------CLEAR BALL---------------------------
		//cout<<"in clear ball"<<endl;
		GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(100,0),"in clear ball");
		_clearBall = true;
//考虑清球角度
		double clearBallDir = CVector(ball.Pos() - goalCenter).dir();
		if (TaskMediator::Instance()->singleBack()==0 && TaskMediator::Instance()->leftBack()!=0){
			CGeoPoint leftpos = DefPos2015::Instance()->getDefPos2015(pVision).getLeftPos();
			CGeoPoint rightpos = DefPos2015::Instance()->getDefPos2015(pVision).getRightPos();
			double goal2leftdir = (DefPos2015::Instance()->getDefPos2015(pVision).getLeftPos() - ball.Pos()).dir() - 0.3;
			double goal2rightdir =  (DefPos2015::Instance()->getDefPos2015(pVision).getRightPos() - ball.Pos()).dir() + 0.3;
			GDebugEngine::Instance()->gui_debug_line(ball.Pos(),ball.Pos()+Utils::Polar2Vector(200,goal2leftdir),COLOR_BLACK);
			GDebugEngine::Instance()->gui_debug_line(ball.Pos(),ball.Pos()+Utils::Polar2Vector(200,goal2rightdir),COLOR_BLACK);
			if (Utils::AngleBetween(clearBallDir,goal2leftdir,goal2rightdir)){
			
				if (goal2rightdir >=0 && goal2leftdir >=0 ){
					clearBallDir = Utils::Normalize(goal2leftdir-Param::Math::PI/6);
				}else if (goal2leftdir <= 0 && goal2rightdir<=0){
					clearBallDir = Utils::Normalize(goal2rightdir+Param::Math::PI/6);
				}else{
					double toLeft =Utils::Normalize(clearBallDir - goal2leftdir);
					double toRight = Utils::Normalize(goal2rightdir - clearBallDir);
					if (fabs(toLeft)>fabs(toRight)){
						clearBallDir = Utils::Normalize(goal2rightdir +Param::Math::PI/6);
					}else{
						clearBallDir = Utils::Normalize(goal2leftdir - Param::Math::PI/6);
					}
				}
			}
		}else if (TaskMediator::Instance()->singleBack()!=0)
		{
			double goal2singledir = (DefPos2015::Instance()->getDefPos2015(pVision).getSinglePos() - ball.Pos()).dir();
			if (std::abs(goal2singledir - clearBallDir)<Param::Math::PI/18){
				if (std::abs(clearBallDir)>Param::Math::PI/10){
					clearBallDir =-clearBallDir;
					if (std::abs(clearBallDir)>Param::Math::PI*70/180){
						if (clearBallDir < 0){
							clearBallDir = clearBallDir + Param::Math::PI/9;
						}else{
							clearBallDir = clearBallDir - Param::Math::PI/9;
						}
					}
				}else{
					clearBallDir = clearBallDir+Param::Math::PI*7/18;
					//cout<<"small angle "<<endl;
				}
			}
		}else{
			clearBallDir = CVector(ball.Pos() - goalCenter).dir();
		}
		
		if (enemy.Pos().dist(ball.Pos())<100){
			clearBallDir =  CVector(ball.Pos() - goalCenter).dir();
			//cout<<"Danger, quick clear ball!!!!"<<endl;
		}

		if (clearBallDir >= Param::Math::PI * 80 / 180.0){
			clearBallDir = Param::Math::PI * 80 / 180.0;
		} else if (clearBallDir <= -Param::Math::PI * 80 / 180.0){
			clearBallDir = -Param::Math::PI * 80 / 180.0;
		}

		if (!ball.Valid() && enemy.Pos().dist(ball.Pos())<30 && Utils::InOurPenaltyArea(enemy.Pos(),-10)){
			_clearBall = false;
			_attackEnemy = true;
			myTask.player.pos = enemy.Pos();
			myTask.player.angle = (enemy.Pos() - goalCenter).dir();
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));

		}
		if (WorldModel::Instance()->CurrentRefereeMsg() == "gameStop"){
			_clearBall = false;
			myTask.player.pos = taskPoint;
			if (ball.Valid()){
				myTask.player.angle = CVector(ball.Pos() - me.Pos()).dir();
			}else{
				myTask.player.angle = CVector(me.Pos() - goalCenter).dir();
			}
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
		}
		if (_clearBall == true){
			if (ball.Pos().dist(goalLeft) < Param::Vehicle::V2::PLAYER_SIZE * dangerFactor && ball.Pos().y() >= goalLeft.y()
				&& CVector(ball.Pos() - me.Pos()).dir() > 0){
				//cout<<"111111"<<endl;
				setSubTask(PlayerRole::makeItGoto(robotNum,goalCenterFrontLeft,CVector(ball.Pos()-me.Pos()).dir(),CVector(0,0),0,0));
			} else if (ball.Pos().dist(goalRight) < Param::Vehicle::V2::PLAYER_SIZE * dangerFactor && ball.Pos().y() <= goalRight.y()
				&& CVector(ball.Pos() - me.Pos()).dir() < 0){
				//cout<<"222222"<<endl;
				setSubTask(PlayerRole::makeItGoto(robotNum,goalCenterFrontRight,CVector(ball.Pos()-me.Pos()).dir(),CVector(0,0),0,0));
			} else {
				//cout<<pVision->Cycle()<<" "<<robotNum<<" aaaaaaaaaaaaaaaaaaaaaaaaa"<<endl;
				DribbleStatus::Instance()->setDribbleCommand(robotNum,2);
				GDebugEngine::Instance()->gui_debug_line(ball.Pos(),ball.Pos()+Utils::Polar2Vector(200,clearBallDir),COLOR_PURPLE);
				setSubTask(PlayerRole::makeItNoneTrajGetBall(robotNum,clearBallDir,CVector(0,0),PlayerStatus::QUICKLY | PlayerStatus::DRIBBLING));
			}
		}
	} 
	else
	{//-------------------------NORMAL---------------------------
	    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(100,0),"in normal goalie");
		setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(myTask));
	}
	if (!Utils::InOurPenaltyArea(ball.Pos(),PENALTY_BUFFER)&&ball.Valid()){
		_clearBall = false;
	}
	CStatedTask::plan(pVision);
}

CPlayerCommand* CGoalie2013::execute(const CVisionModule* pVision)
{
	if( subTask() ){
		return subTask()->execute(pVision);
	}
	return NULL;
}

bool	CGoalie2013::needtoClearBall(const CVisionModule* pVision)
{
	bool result = false;
	const MobileVisionT& ball = pVision->Ball();
	int advancer = BestPlayer::Instance()->getOurBestPlayer(); 
	const PlayerVisionT enemy = pVision->TheirPlayer(BestPlayer::Instance()->getTheirBestPlayer());
	if (((ball.Vel().mod() < 150 && Utils::InOurPenaltyArea(ball.Pos(),-30))||
		(ball.Vel().mod() < 50 && Utils::InOurPenaltyArea(ball.Pos(),PENALTY_BUFFER)) && ball.Valid()
		&& !checkWeHaveHelper(pVision) && !Utils::InOurPenaltyArea(enemy.Pos(),DEFEND_PENALTY_BUFFER))||
		((ball.Vel().mod() < 120 && Utils::InOurPenaltyArea(ball.Pos(),-30))||
		(ball.Vel().mod() < 40 && Utils::InOurPenaltyArea(ball.Pos(),PENALTY_BUFFER)) && ball.Valid()
		&& !checkWeHaveHelper(pVision) && Utils::InOurPenaltyArea(enemy.Pos(),DEFEND_PENALTY_BUFFER))){
		result = true;
		GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(50,0),"need to clear ball");
	}
	return result;
}

bool CGoalie2013::needtoAttackEnemy(const CVisionModule* pVision)
{
	bool result = false;
	const PlayerVisionT enemy = pVision->TheirPlayer(BestPlayer::Instance()->getTheirBestPlayer());
	const PlayerVisionT& me = pVision->OurPlayer(task().executor);
	const MobileVisionT& ball = pVision->Ball();
	if ((Utils::InOurPenaltyArea(enemy.Pos(),DEFEND_PENALTY_BUFFER) && ball.Vel().mod()>120 &&  DefendUtils::BallIsToPenaltyArea()
		&& !DefendUtils::isBallShotToTheGoal()) 
		|| (!ball.Valid() && Utils::InOurPenaltyArea(enemy.Pos(),PENALTY_BUFFER))){
		result = true;
		GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0,0),"need to attack enemy");
	}
	return result;
}

bool CGoalie2013::checkWeHaveHelper(const CVisionModule* pVision)
{
	bool result = false;
	int helper[Param::Field::MAX_PLAYER+1]={0};
	int helpernum = 0;
	int bestnum = BestPlayer::Instance()->getOurBestPlayer();
	const MobileVisionT& ball = pVision->Ball();
	for (int i = 1;i<= Param::Field::MAX_PLAYER;i++){
		if (pVision->OurPlayer(i).Pos().dist(ball.Pos())<30){
			helpernum ++;
			helper[helpernum] = i;
		}
	}
	if (Utils::InOurPenaltyArea(ball.Pos(),PENALTY_BUFFER) && !Utils::InOurPenaltyArea(ball.Pos(),0)){
		if (pVision->TheirPlayer(bestnum).X()<ball.X() && 
			abs((goalCenter - ball.Pos()).dir() - (goalCenter - pVision->TheirPlayer(bestnum).Pos()).dir())<Param::Math::PI/4
			&& helpernum>=3){
				result = true;
		}
	}
	return result;
}