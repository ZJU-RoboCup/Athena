#include "GDebugEngine.h"
#include "skill/Factory.h"
#include <utils.h>
#include "param.h"
#include <cmath>
#include <utils.h>
#include <VisionModule.h>
#include "BallStatus.h"
#include "ShootBall.h"
#include "TouchKickPos.h"
#include "KickDirection.h"
#include "WorldModel.h"
#include <KickStatus.h>
#include "RobotSensor.h"
#include "DribbleStatus.h"
	

using namespace Param::Field;
using namespace Param::Vehicle::V2;
namespace
{
	enum ShootBall_State{
		TOUCH_KICK = 1,
		CHASE_KICK,
		INTERCEPT_KICK,
		DEFEND_GOAL
	};
	bool verBos = false;
	int Touch2ChaseCnt = 0;
	int Touch2InterCnt = 0;
	int Chase2TouchCnt = 0;
	int Chase2InterCnt = 0;
	int Inter2ChaseCnt = 0;
	int Inter2TouchCnt = 0;
	int MaxOutInterDribbleCycle = 60;
	const double BALL_SLOW_SPEED = 100;
	const double math_onedir = Param::Math::PI/180;
	const double math_pi = Param::Math::PI;
	const double DEFEND_GOAL_DISTANCE = 150;
};

CShootBall::CShootBall()
{
	_lastCycle = 0;
	_out_inter_cnt = 0;
	_backShootDir=0;
	_out_inter = false;
	_savedCycle = 0;
	_ballVelChangeCounter = 0;
	_lastBallVelDir = 0;
	_defendGoalChangeCounter = 0;
}

// 传进来的朝向是一个比较稳定的朝向，适用于TouchKick，但是对
// 于Chase和Intercept来说需要时时计算朝向，Touch是静态动作，而
// Chase和Intercept是动态动作！！！！
void CShootBall::plan(const CVisionModule* pVision)
{	
	//cout << "shootSafe" << endl;
	const int runner = task().executor;
	int flags = task().player.flag;
	double finalDir = task().player.angle;
	const PlayerVisionT& self = pVision->OurPlayer(runner);
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint rawBallPos = pVision->RawBall().Pos();
	CVector self2ball = rawBallPos - self.Pos();
	const double ballVelDir = ball.Vel().dir();
	double antiBallVelDir = Utils::Normalize(ballVelDir + Param::Math::PI);
	double ballSpeed = ball.Vel().mod();
	CGeoSegment shootSeg(self.Pos(), self.Pos() + Utils::Polar2Vector(1500, finalDir));
	CGeoPoint ballShootSegInterP = shootSeg.projection(rawBallPos);
	CGeoLine ballMovingLine = CGeoLine(rawBallPos, rawBallPos + Utils::Polar2Vector(1000, ball.Vel().dir()));
	CGeoPoint projMePos = ballMovingLine.projection(self.Pos());
	double me2ballMovingLineDist = projMePos.dist(self.Pos());
	const bool isSensored = RobotSensor::Instance()->IsInfoValid(runner) && RobotSensor::Instance()->IsInfraredOn(runner);	//If Infrared on 
	CGeoPoint defendPos = CGeoPoint(-PITCH_LENGTH / 2 + DEFEND_GOAL_DISTANCE, 0);
	const CGeoSegment ballMovingSeg = CGeoSegment(rawBallPos, rawBallPos + Utils::Polar2Vector(800, Utils::Normalize(ballVelDir)));
	const CGeoPoint projMe = ballMovingSeg.projection(self.Pos());					//小车在球移动线上的投影点
	double projDist = projMe.dist(self.Pos());										//小车到投影点的距离
	const double ball2projDist = projMe.dist(rawBallPos);							//投影点到球的距离
	const bool meOnBallMovingSeg = ballMovingSeg.IsPointOnLineOnSegment(projMe);	//投影点是否在球速线段上面

	bool  isBallOnShootSeg = shootSeg.IsPointOnLineOnSegment(ballShootSegInterP);
	double a_m2t_2_m2b = fabs(Utils::Normalize(self2ball.dir() - finalDir)) * 180 / Param::Math::PI;//车球连线朝向 与 车朝向的夹角
	double a_m2t_2_bsr = fabs(Utils::Normalize(antiBallVelDir - finalDir)) * 180 / Param::Math::PI;//
	double a_m2t_2_bsl = fabs(Utils::Normalize(ballVelDir - finalDir)) * 180 / Param::Math::PI;
	double a_m2b_2_bsr = fabs(Utils::Normalize(self2ball.dir() - antiBallVelDir)) * 180 / Param::Math::PI;
	double meBall_ballVelDiff = fabs(Utils::Normalize(self2ball.dir() - ballVelDir)) * 180 / Param::Math::PI;
	//cout << ballVelDir << endl;
	static bool isInBack = false;
	if (self.Pos().x() < -150) {
		isInBack = true;
	}if (self.Pos().x() > 50) {
		isInBack = false;
	}


	bool isEnemyChipKick = false;
	//当球速线变化频繁，或者球消失时，说明对方进行了挑球，此时应该进行球门连线防守
	if (flags == PlayerStatus::ACCURATELY) {
		if (_savedCycle == 0) {
			_savedCycle = pVision->Cycle();
			_ballVelChangeCounter = 0;
			_lastBallVelDir = ball.Vel().dir();
		}
		else {
			if (fabs(ball.Vel().dir() - _lastBallVelDir) > math_onedir * 5 || !ball.Valid()) {

				_ballVelChangeCounter++;
			}
			_lastBallVelDir = ball.Vel().dir();
		}
		if (pVision->Cycle() - _savedCycle > 60)
		{
			if (_ballVelChangeCounter > 9) {
				isEnemyChipKick = true;
			}
			else if (_ballVelChangeCounter <= 3) {
				isEnemyChipKick = false;
			}
			_savedCycle = pVision->Cycle();
			_ballVelChangeCounter = 0;
		}
	}


	//[--------Some Additional Boolean Variebales In State Switch, added by Fantasy at 2015.6.10--------] 
	bool chaseLineFlag = false;
	const CGeoSegment chaseTargetSeg = CGeoSegment(CGeoPoint(Param::Field::PITCH_LENGTH / 2, -Param::Field::PITCH_WIDTH / 2), CGeoPoint(Param::Field::PITCH_LENGTH / 2, Param::Field::PITCH_WIDTH / 2));
	CGeoLineLineIntersection chaseLineInter = CGeoLineLineIntersection(chaseTargetSeg, ballMovingLine);
	if (chaseLineInter.Intersectant()) {
		chaseLineFlag = true;
	}
	//cout<<fabs(ballVelDir)<<endl;
	bool isBallSlow = ballSpeed < 120;
	bool isNeedTouchKick = ballSpeed > 120 && ballSpeed < 550 && a_m2t_2_m2b < 60 && a_m2t_2_bsr < 60 && self2ball.mod()>100 && a_m2b_2_bsr < 20 && ball2projDist / projDist>2 && projDist < 75;
	//cout << ballSpeed<<" "<< a_m2t_2_m2b <<" "<< a_m2t_2_bsr <<" "<< a_m2b_2_bsr << endl;
	bool isNeedInterKick = ballSpeed > 120 && ballSpeed < 450 || (ballSpeed>450 && projDist < 20) || (ballSpeed<120 && meOnBallMovingSeg && self2ball.mod()<100);
	bool isNeedChaseKick = chaseLineFlag&&(projDist<150 && self.Pos().x() - ball.Pos().x()<=150) || !meOnBallMovingSeg && ballVelDir<math_onedir * 110 || isBallSlow;//fabs(ballVelDir) <= math_onedir * 100 &&
	bool isNeedInterKickForCrossBall = fabs(ballVelDir) >= 90 && fabs(ballVelDir) <= 100 && self2ball.mod()>200&&meOnBallMovingSeg;
	//cout << ballSpeed<< endl;
	//cout << "bool : "<<isNeedTouchKick <<" "<< isNeedInterKick <<" "<<isNeedChaseKick<< endl;

	CGeoSegment defendSeg=CGeoSegment(CGeoPoint(-Param::Field::PITCH_LENGTH/2,-Param::Field::PENALTY_AREA_WIDTH/3),CGeoPoint(-Param::Field::PITCH_LENGTH/2,Param::Field::PENALTY_AREA_WIDTH/3)); //球门
	CGeoLineLineIntersection inter=CGeoLineLineIntersection(defendSeg,ballMovingLine);
	bool canEnterDefendArea=false;
	if (ballSpeed>200)
	{
		if (inter.Intersectant()){
			CGeoPoint interPoint=inter.IntersectPoint();
			if (defendSeg.IsPointOnLineOnSegment(interPoint))
			{
				canEnterDefendArea=true;
			}
		}
	}
	canEnterDefendArea=canEnterDefendArea&&Utils::InOurPenaltyArea(self.Pos(),40);

	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1
		//|| BallStatus::Instance()->IsBallKickedOut(runner)
		|| _lastRunner != runner){
		setState(BEGINNING);
		Touch2ChaseCnt = 0;
		Touch2InterCnt = 0;
		Chase2TouchCnt = 0;
		_backShootDir=0;
	}
	int new_state = state(), old_state = state();
	
		old_state = new_state;
		switch (old_state){
			case BEGINNING:
				{
					if( isNeedTouchKick){
						if(verBos) cout<<"BEGINNING-->Touch"<<endl;
						new_state = TOUCH_KICK;
					} else if( isNeedInterKick){
						if(verBos) cout<<"BEGINNING-->Inter"<<endl;
						new_state = INTERCEPT_KICK;
					} else{
						if(verBos) cout<<"BEGINNING-->Chase"<<endl;
						new_state = CHASE_KICK;
					}
					break;
				}
			case TOUCH_KICK:
				if (!isSensored && (!meOnBallMovingSeg ||ballSpeed>500||ballSpeed<100)){//&& fabs(ballVelDir) <= math_onedir * 95
					if( Touch2ChaseCnt > 5){
						if(verBos) cout<<"Touch-->Chase"<<endl;
						new_state = CHASE_KICK;
						Touch2ChaseCnt = 0;
					} else{
						//cout << "Ballspeed:"<<ballSpeed<< endl;
						Touch2ChaseCnt++;
					}
				}
				else if (!isSensored && (!meOnBallMovingSeg || ballSpeed > 500 || ballSpeed<100) ){// && fabs(ballVelDir) > math_onedir *95
					if( Touch2InterCnt > 5){
						if(verBos) cout<<"Touch-->Inter"<<endl;
						new_state = INTERCEPT_KICK;
						Touch2InterCnt = 0;
					} else{
						Touch2InterCnt++;
					}
				}
				if (isEnemyChipKick){
					new_state = DEFEND_GOAL;
				}
				break;
			case CHASE_KICK:
				if (ballSpeed > BALL_SLOW_SPEED){
					if (isNeedTouchKick){
						if (Chase2TouchCnt > 1){
							if (verBos) cout << "Chase-->Touch" << endl;
							//cout << isNeedTouchKick << " " << isNeedChaseKick << " " << isNeedChaseKick << endl;
							new_state = TOUCH_KICK;
							Chase2TouchCnt = 0;
						}
						else{
							Chase2TouchCnt++;
						}
					}
					else if (isNeedInterKick || (projDist > 150 && fabs(ballVelDir - self2ball.dir()) > math_onedir * 75) || isNeedInterKickForCrossBall){ //TO CHANGE
						if (Chase2InterCnt > 8){
							if (verBos) cout << "Chase-->Inter" << endl;
							//cout << isNeedTouchKick << " " << isNeedChaseKick << " " << isNeedChaseKick << endl;
							new_state = INTERCEPT_KICK;
							Chase2InterCnt = 0;
						}
						else{
							Chase2InterCnt++;
						}
					}
				}
				if (isEnemyChipKick){
					new_state = DEFEND_GOAL;
				}
				break;
			case INTERCEPT_KICK:
				if (chaseLineFlag
					&&!isNeedInterKickForCrossBall&&self.Pos().x() - ball.Pos().x() <= 200
					&&ballSpeed>150&&projDist>100 
					||(ballSpeed<BALL_SLOW_SPEED &&ball.Pos().dist(self.Pos())>150)){  //fabs(ballVelDir) <= math_onedir * 80 &&  && fabs(self.Dir())<Param::Math::PI * 80/180 ||ball.Pos().x()>300&&fabs(ballVelDir)<math_onedir*100
					if (Inter2ChaseCnt > 2) {
						if (verBos) cout << "Inter-->Chase" << endl;
						//_out_inter = true;
						new_state = CHASE_KICK;
						Inter2ChaseCnt = 0;
					}
					else {
						//cout << chaseLineFlag << " " << isNeedInterKickForCrossBall << " " << self.Pos().x() - ball.Pos().x() << " " << ballSpeed << " " << projDist << endl;
						Inter2ChaseCnt++;
					}
				}
				if (ballSpeed > 100 && ballSpeed<500 && a_m2t_2_m2b < 35+40*isInBack && a_m2t_2_bsr<35 +50*isInBack&& self2ball.mod()>100 && a_m2b_2_bsr<20+10*isInBack && ball2projDist / projDist>2 && projDist < 75){
					if (verBos) cout << "Inter-->Touch" << endl;
					new_state = TOUCH_KICK;
				}
				if (isEnemyChipKick){
					new_state = DEFEND_GOAL;
				}
				break;
			case DEFEND_GOAL:
				if (!isEnemyChipKick){
					_defendGoalChangeCounter++;
				}
				if (_defendGoalChangeCounter > 120||(ball.Pos()-self.Pos()).mod()<50&&self.Pos().x()<-300){
					new_state = INTERCEPT_KICK;
					_defendGoalChangeCounter = 0;
				}
				break;
			/*ase INTER_TO_TOUCH:
				if( ballSpeed<100 && !RobotSensor::Instance()->IsInfraredOn(runner)
					|| meBall_ballVelDiff<Param::Math::PI/3||a_m2t_2_bsr > 75 ||a_m2t_2_m2b>75){
					if(verBos) cout<<"INTER_TO_TOUCH-->Chase"<<endl;
					new_state = CHASE_KICK;
					//fixPos = CGeoPoint(0,0);
				}
				break;*/
			default:
				new_state = BEGINNING;
				break;
		}
	setState(new_state);
	//if(verBos) cout<<"shoot"<<new_state<<endl;
	if(_out_inter){
		_out_inter_cnt++;
		if (_out_inter_cnt < MaxOutInterDribbleCycle){
			//DribbleStatus::Instance()->setDribbleCommand(runner,3);
		} else{
			_out_inter = false;
			_out_inter_cnt = 0;
			DribbleStatus::Instance()->setDribbleCommand(runner,0);
		}
	}
	

	flags = flags | PlayerStatus::DODGE_OUR_DEFENSE_BOX;
	KickDirection::Instance()->GenerateShootDir(runner,pVision->OurPlayer(runner).Pos());

	if (state()==CHASE_KICK || state()==INTERCEPT_KICK){
		finalDir = 	KickDirection::Instance()->getRawKickDir();
	}

	bool back_chip = false;
	if (flags & PlayerStatus::ACCURATELY && isInBack){	
		_backShootDir=(CGeoPoint(Param::Field::PITCH_LENGTH/2,0)-self.Pos()).dir();
		finalDir=_backShootDir;
		back_chip = true;
	}

	////////////////////////////////////////////////////////
	bool changeKickDir=false;
	if (rawBallPos.x()<-350){
		CGeoSegment shootSeg=CGeoSegment(rawBallPos,rawBallPos+Utils::Polar2Vector(120,finalDir));
		for (int i=1;i<=Param::Field::MAX_PLAYER;i++)
		{
			if (pVision->OurPlayer(i).Valid())
			{
				CGeoPoint proj=shootSeg.projection(pVision->OurPlayer(i).Pos());
				double dist = proj.dist(pVision->OurPlayer(i).Pos());
				if (dist<15 && shootSeg.IsPointOnLineOnSegment(pVision->OurPlayer(i).Pos())){
					changeKickDir=true;
					break;
				}
			}
		}
	}

	finalDir = checkShootDirNotOutTheirGoal(pVision, finalDir, 5);
	bool open_compensate = false;
	if (open_compensate){
		double ball2GoalDir = (CGeoPoint(Param::Field::PITCH_LENGTH/2, 0) - ball.Pos()).dir();
		double kick_angle = fabs(ball2GoalDir/Param::Math::PI/45);
		finalDir = finalDir - Utils::Sign(finalDir) * (kick_angle);
	}
	///////////////////////////////////////////////////
	double shootPre=task().player.kickprecision;
	if (ballSpeed<80)
	{
		shootPre= 3 * Param::Math::PI/180;
	}
	//cout<<"changeDir "<<changeKickDir<<endl;
	//GDebugEngine::Instance()->gui_debug_line(self.Pos(), self.Pos() + Utils::Polar2Vector(500, finalDir));
	
	bool is_runner_dir_ok = WorldModel::Instance()->KickDirArrived(pVision->Cycle(),
		finalDir,shootPre,runner);
	if (is_runner_dir_ok || (NormalPlayUtils::faceTheirGoal(pVision,runner) && ballSpeed > 50)) {
		if (changeKickDir || back_chip)
		{
			KickStatus::Instance()->setChipKick(runner,400);
		}else{
			if (!task().player.ischipkick) {
				//cout << state() << endl;
				if (state() == CHASE_KICK) {
					KickStatus::Instance()->setKick(runner, 750);
					//cout <<"setKickChase" << endl;
				}
					
				else
					KickStatus::Instance()->setKick(runner, task().player.kickpower); //
			} else {
				KickStatus::Instance()->setChipKick(runner,task().player.chipkickpower);
			}
		}
	}
	//if(verBos) cout<<"shoot : "<<state()<<endl;
	switch (state()){
		case TOUCH_KICK:
			{
				setSubTask(PlayerRole::makeItTouchKick(runner, finalDir, false, flags));
			}
			break;
		case CHASE_KICK:
			{
				setSubTask(PlayerRole::makeItChaseKickV2(runner, finalDir, flags));
			}
			break;
		case INTERCEPT_KICK:
			{
				setSubTask(PlayerRole::makeItInterKickV2(runner,finalDir,flags, changeKickDir));
			}
			break;
		case DEFEND_GOAL:
			{
				defendPos = CGeoPoint(-PITCH_LENGTH / 2, 0) + Utils::Polar2Vector(DEFEND_GOAL_DISTANCE, (ball.Pos() - CGeoPoint(-PITCH_LENGTH / 2, 0)).dir());
				setSubTask(PlayerRole::makeItGoto(runner,defendPos,0,0,0));
			}
			break;
		/*case INTER_TO_TOUCH:
			{
				TouchKickPos::Instance()->GenerateTouchKickPos(pVision,runner,finalDir);
				CGeoPoint targetPos = TouchKickPos::Instance()->getKickPos();
				setSubTask(PlayerRole::makeItGoto(runner, targetPos, finalDir, flags));
				break;
			}*/
		default:
			finalDir=KickDirection::Instance()->getRawDir();
			setSubTask(PlayerRole::makeItChaseKickV2(runner, finalDir, flags));
			break;
	}
	//GDebugEngine::Instance()->gui_debug_line(self.Pos(),self.Pos()+Utils::Polar2Vector(300, finalDir));


	//GDebugEngine::Instance()->gui_debug_line( self.Pos(),self.Pos()+Utils::Polar2Vector(1000,finalDir));

	_lastCycle =  pVision -> Cycle();
	_lastRunner = runner;
	CPlayerTask::plan(pVision);
}
CPlayerCommand* CShootBall::execute(const CVisionModule* pVision)
{
	if (subTask()) {
		return subTask()->execute(pVision);
	}

	return NULL;
}


double CShootBall::checkShootDirNotOutTheirGoal(const CVisionModule* pVision, const double finalDir,const double buffer){
	const int runner = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(runner);
	const MobileVisionT& ball = pVision->Ball();
	CGeoPoint enemyGoalMiddle = CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0);
	CGeoPoint enemyGoalRight = CGeoPoint(Param::Field::PITCH_LENGTH / 2, Param::Field::PENALTY_AREA_WIDTH/2);
	CGeoPoint enemyGoalLeft = CGeoPoint(Param::Field::PITCH_LENGTH / 2, -Param::Field::PENALTY_AREA_WIDTH/2);

	double changeDir = finalDir;
	if (finalDir > (enemyGoalRight - me.Pos()).dir() + buffer || finalDir < (enemyGoalLeft - me.Pos()).dir() - buffer){
		changeDir = (enemyGoalMiddle - me.RawPos()).dir();
	}

	return changeDir;
}

