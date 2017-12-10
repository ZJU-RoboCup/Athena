#include "InterceptBallV2.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include <utils.h>
#include <ControlModel.h>
#include <cornell/Trajectory.h>
#include <RobotCapability.h>
#include <CMmotion.h>
#include <TimeCounter.h>
#include <RobotSensor.h>
#include "utils.h"
#include "ShootRangeList.h"
#include <CommandFactory.h>
#include "TouchKickPos.h"
#include "BallSpeedModel.h"
#include <GDebugEngine.h>
#include "PlayInterface.h"
#include "KickStatus.h"
#include "WorldModel.h"
#include <ConfigReader.h>
#include <PlayInterface.h>
#include "commandsender.h"
#include <cmath>
#include <DribbleStatus.h>
#include <BestPlayer.h>
#include "SupportPos.h"

using namespace Param;
namespace{
	enum{
		RUSH_TO_BALL=1,
		SPEED_UP,
		STABLE_BALL,
		TOUCH_KICK,
		TURN_KICK,
		CHASE_KICK,
		GET_BALL
	};
	bool IS_SIMULATION = false;
	double MAX_TRANSLATION_ACC = 650;
	double MAX_TRANSLATION_DEC = 650;
	const bool  NEED_CHIP_KICK_IN_BACK = true;
	const bool debugEngine = true;
	const double ALLOWANCE_DIST_TO_DRIBBLE = Param::Vehicle::V2::PLAYER_SIZE ;
	const double math_pi = Param::Math::PI;
	const double math_onedir = math_pi / 180;

	//The distance to surpass the ball in the state "SPEED_UP"
	const double ALLOWANCE_DIST_AVOID_BALL = Param::Vehicle::V2::PLAYER_SIZE*2;

	const double ALLOWANCE_DIST_TO_STABLE = Param::Vehicle::V2::PLAYER_SIZE*3;   

	//If the difference between the facing angle and the final kick angle is smaller than the angle, use get ball, otherwise drrible turn.
	const double ALLOWANCE_DIR_TO_DRRIBLE = math_onedir * 60;

	//The speed to get ball
	const double BALL_SLOW_SPEED = 50;

	//The stable slack time
	const double MAX_CHANGE_OF_SLACK_TIME = 0.2;
	const double STABLE_SLACK_TIME_FRONT = 0.4;
	const double STABLE_SLACK_TIME_BACK = 0.9;

	//Time to hold a state before a state change really takes place
	const int State_Counter_Num = 5;

	//Used in the state "RUSH_TO_BALL" to calculate the distance the robot allows to move before the ball reaches the robot   
	const double distSub = 60;                    
	const double distBack = 100;
	const CGeoPoint LEFT_GOAL_POST = CGeoPoint(-Field::PITCH_LENGTH / 2, -Field::GOAL_WIDTH / 2);
	const CGeoPoint RIGHT_GOAL_POST = CGeoPoint(-Field::PITCH_LENGTH / 2, Field::GOAL_WIDTH / 2);
}

CInterceptBallV2::CInterceptBallV2(){
//params initialize
	_directCommand = NULL;
	_drribleCounter = 0;
	_lastCycle = 0;
	_stateCounter = 0;
	_hasBallCounter = 0;
	_touchBallFlag = false;
	_drribleKickCounter = 0;
	_normalKickCounter = 0;
	{
		DECLARE_PARAM_READER_BEGIN(CGotoPositionV2)
			READ_PARAM(MAX_TRANSLATION_ACC)
			READ_PARAM(MAX_TRANSLATION_DEC)
		DECLARE_PARAM_READER_END
	}


}



void CInterceptBallV2::plan(const CVisionModule* pVision){
	
	//[[--1.Const Variables Initialization & Sensor Information Capture--]]
	if (pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1){
		setState(BEGINNING);
		//Params Initialize
		_drribleCounter = 0;
		_stateCounter = 0;
		_hasBallCounter = 0;
		_touchBallFlag = false;
		_drribleKickCounter = 0;
		_normalKickCounter = 0;
	}
	_directCommand = NULL;
	//Normal Variables
	const int robotNum = task().executor;
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const int playerFlag = task().player.flag;
	const bool chipFlag = task().player.ischipkick;
	const double ballSpeed = ball.Vel().mod();
	const CGeoPoint ballPos = ball.Pos();
	const CGeoPoint ballRawPos = ball.RawPos();
	const double meSpeed = me.Vel().mod();
	const double meDir = me.Dir();
	const double finalKickDir = task().player.angle;           
	const bool isSensored = RobotSensor::Instance()->IsInfoValid(robotNum) && RobotSensor::Instance()->IsInfraredOn(robotNum);	//If Infrared on 
	const double distToBall = (me.Pos() - ballRawPos).mod();
	const double dirToBall = Utils::Normalize((ballRawPos - me.Pos()).dir());
	const double dirFaceDiffsBall = fabs(me.Dir() - dirToBall);                          //Angle between the facing angle of the robot and the direct angle of the ball to the robot.
	const double dirFaceDIffsKick = fabs(me.Dir() - finalKickDir);
	const CGeoPoint ballPos_predict = BallSpeedModel::Instance()->posForTime(20, pVision);          //Predicted position of the ball based on the speed model
	const double maxAcc = MAX_TRANSLATION_ACC;
	const double maxDcc = MAX_TRANSLATION_DEC;
	const double BallVelDir = Utils::Normalize(ball.Vel().dir());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + math_pi);
	const int kickPower = 790;
	bool isBallInVision =  isVisionHasBall(pVision, robotNum);
	int enemyNum = checkEnemy(pVision);                                   //If there is an enemy, show the enemy number, otherwise zero
	//SlackTime Calculation
	CGeoLine& ballVelLine = CGeoLine(ballRawPos, ball.Vel().dir());
	CGeoPoint projPoint = ballVelLine.projection(me.Pos());
	const CGeoSegment ballVelSeg = CGeoSegment(ballPos, ballPos + Utils::Polar2Vector(800, ball.Vel().dir()));
	double me2projDist = projPoint.dist(me.Pos());										//The distance between the car and the projection
	double ball2projDist = projPoint.dist(ballRawPos);							//The distance between the ball and the projection
	double facedir = Utils::Normalize(ball.Vel().dir() + Param::Math::PI);      //Facing dir at the target position

	
	double ball2projTime = BallSpeedModel::Instance()->timeForDist(ball2projDist, pVision);
	if (ball2projTime == -1){                           //When the ball can not reach the proj-position
		ball2projTime = 2.5;
	}
	bool isMeInDec = meSpeed*meSpeed > (2 * maxAcc*me2projDist);
	bool isMeInFront = me.Pos().x() > 0;
	double me2projTime = 0;
	if (isMeInDec){
		me2projTime = meSpeed / maxAcc;
	}
	else{
		//me2projTime = sqrt(me2projDist / maxAcc + meSpeed*meSpeed / (2 * maxAcc)) - meSpeed / maxAcc + sqrt(me2projDist / maxAcc + meSpeed*meSpeed / (2 * maxAcc*maxAcc));
		me2projTime = 2 * sqrt(me2projDist / maxAcc + meSpeed*meSpeed / (2 * maxAcc*maxAcc)) - meSpeed / maxAcc;
	}
	double slackTime = ball2projTime - me2projTime;    //The key value in the skill, which represents the time the robot could wait before the ball finally reaches the robot
													   //The slacktime<0 represents an impossible interception for a robot. Therefore, we must keep the slacktime at least bigger than 0. The bigger the slacktime
													   //is, the more stable the robot could intercept the ball.
	double ballSpeedFactor = 0;

	double stableSlackTime = isMeInFront ? STABLE_SLACK_TIME_FRONT : STABLE_SLACK_TIME_BACK;
	if (enemyNum != 0){
		stableSlackTime = stableSlackTime - enemyMenacePoint(pVision, enemyNum);
	}


	if (slackTime > 2.5){                          //Avoid crush into the ball, set the max value of slackTime
		slackTime = 2.5;
	}
	CGeoPoint targetPoint = projPoint;             //The key point in the skill, which is a basic position for the robot to go. Based on the slacktime, the skill will decide whether the robot will go forward 
												   //or backward of the point.
	CGeoPoint ballDeviantPos = ballPos;
	CGeoRectangle field = CGeoRectangle(CGeoPoint(Param::Field::PITCH_LENGTH / 2.0, -Param::Field::PITCH_WIDTH / 2.0),
		CGeoPoint(-Param::Field::PITCH_LENGTH / 2.0, Param::Field::PITCH_WIDTH / 2.0));
	CGeoLineRectangleIntersection inter = CGeoLineRectangleIntersection(ballVelLine, field);


	//[[--2. Boolean Variables Definition--]]
	bool isNeedGetBall = ballSpeed<BALL_SLOW_SPEED||!ball.Valid();
	bool canStableBall = me2projDist < 5 && distToBall<ALLOWANCE_DIST_TO_STABLE && fabs(Utils::Normalize(meDir-dirToBall))<math_onedir*5;
	bool isNeedRush2Ball = distToBall > 100;
	bool isAbleTurnKick = distToBall<ALLOWANCE_DIST_TO_DRIBBLE*2;
	bool isNeedSpeedUp = !(ballVelSeg.IsPointOnLineOnSegment(projPoint));       //if the ball is getting away from the robot, the robot need to speed up to block the ball
	
	bool canTouchKick = fabs(Utils::Normalize(antiBallVelDir - finalKickDir)) < math_onedir * 5 && me2projDist < 5;
	bool canNormalKick = false;
	bool isNeedChipKick = NEED_CHIP_KICK_IN_BACK&&(me.Pos().x() < -80);


	// canNormalKick = (fabs(Utils::Normalize(meDir - finalKickDir)) < math_onedir * 2) || faceTheirGoal(pVision, robotNum)&&me.Pos().x()>Param::Field::PITCH_LENGTH/4;
	double inter_precise = math_onedir * 5;
	if(ball.Pos().x() > 0){
		inter_precise = math_onedir * 3; // 5
	}else{
		inter_precise = math_onedir * 2;
	}
	canNormalKick = (fabs(Utils::Normalize(meDir - finalKickDir)) < math_onedir * inter_precise);
	
	bool canTurnKick = fabs(Utils::Normalize(meDir - finalKickDir))>ALLOWANCE_DIR_TO_DRRIBLE && distToBall < ALLOWANCE_DIST_TO_DRIBBLE*2 &&ballSpeed<30;
	bool isMeHasBall = isBallInVision || isSensored;
	bool isNeedChaseKick = !ballVelSeg.IsPointOnLineOnSegment(projPoint);

	if (IS_SIMULATION == false){
		isMeHasBall = isSensored;
	}
	if (isMeHasBall || distToBall < Param::Vehicle::V2::PLAYER_SIZE / 2){
		_touchBallFlag = true;
	}
	
	//[[--3. State Definition & Switch--]]
	int old_state = state(), new_state = state();
	switch (old_state){
	case BEGINNING:
		new_state = RUSH_TO_BALL;
		break;
	case RUSH_TO_BALL:
		if (isNeedGetBall){
			new_state = GET_BALL;
		}
		if (canStableBall){
			new_state = STABLE_BALL;
			_drribleCounter = 0;
		}
		if (isNeedSpeedUp){
			if (_touchBallFlag){
				new_state = CHASE_KICK;
			}
			else{
				new_state = SPEED_UP;
			}
		}
		if (canTouchKick){
			new_state = TOUCH_KICK;
		}
		break;
	case STABLE_BALL:
		if (isMeHasBall){
			_hasBallCounter++;
		}
		else{
			_hasBallCounter--;
		}
		if(isNeedGetBall&&_hasBallCounter==-120){
				new_state = GET_BALL;
				_hasBallCounter = 0;
			}
		else if (distToBall>=ALLOWANCE_DIST_TO_STABLE){
			_hasBallCounter = 0;
			if (isNeedChaseKick){
				//cout << "JUMP CHASE" << endl;
				new_state = CHASE_KICK;
			}
			else{
				//cout << "JUMP RUSH TO BALL" << endl;
				new_state = RUSH_TO_BALL;
			}
		}
		else if(_hasBallCounter == 20||canTurnKick){
			//cout << "JUMP TURN" << endl;
			new_state = TURN_KICK;
		}
		break;
	case TURN_KICK:
		if (isNeedRush2Ball||!isAbleTurnKick){
			new_state = RUSH_TO_BALL;
		}
		else if (isNeedSpeedUp){
			if (_touchBallFlag){
				new_state = CHASE_KICK;
			}
			else
			new_state = SPEED_UP;
		}
		break;
	case GET_BALL:
		//cout << "GET_BALL" << endl;
		if (!isNeedGetBall){
			if (canStableBall){
				new_state = STABLE_BALL;
			}
			else if (isNeedRush2Ball){
				new_state = RUSH_TO_BALL;
			}
			else if (isNeedSpeedUp){
				if (_touchBallFlag){
					new_state = CHASE_KICK;
				}
				else
					new_state = SPEED_UP;
			}
		}
		break;
	case SPEED_UP:
		if (isNeedGetBall){
			new_state = GET_BALL;
		}
		else if (!isNeedSpeedUp && ball2projDist > 30){
			new_state = RUSH_TO_BALL;
		}
		break;
	case TOUCH_KICK:
		if (!canTouchKick){
			if (isNeedGetBall){
				new_state = GET_BALL;
			}
			else
				new_state = RUSH_TO_BALL;
		}
		break;
	case CHASE_KICK:
		//cout << "CHASE_KICK" << endl;
		if (!isNeedChaseKick&&ballSpeed>100){
			_touchBallFlag = false;
			new_state = RUSH_TO_BALL;
		}
		break;
	default:
		new_state = RUSH_TO_BALL;
		break;
	}
	if (state() == BEGINNING){
		setState(new_state);
	}
	else{
		if (_stateCounter == 0){
			setState(new_state);
			_stateCounter++;
		}
		else{
				if (new_state == state()) {
					_stateCounter = min(State_Counter_Num, _stateCounter + 1);
				}
				else {
					_stateCounter = max(0, _stateCounter - 1);
				}
			
			
		}
	}


	//[[--4. State Execution--]]
	TaskT interTask(task());
	//cout << "fuck:"<<state()<< endl;
	switch (state()){
		case GET_BALL:
			if (debugEngine)
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0, 0), "GET_BALL");
			if (ballSpeed < 100){
				setSubTask(PlayerRole::makeItNoneTrajGetBall(task().executor, finalKickDir, CVector(0, 0), task().player.flag, -2));
			}
			else
				setSubTask(PlayerRole::makeItChaseKickV2(robotNum, finalKickDir, playerFlag));
			break;
		case RUSH_TO_BALL:
			if (debugEngine)
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0,0), "RUSH_TO_BALL");

			if (slackTime > stableSlackTime + 0.2){
				ballSpeedFactor = 1 * (slackTime - stableSlackTime);
			}
			else if (slackTime < stableSlackTime - 0.2){
				ballSpeedFactor = 3 * (stableSlackTime - 0.2 - slackTime);
			}
			if (slackTime > stableSlackTime+0.2&&distToBall>distSub){
					targetPoint = projPoint + Utils::Polar2Vector(max((250-ballSpeed),0)*ballSpeedFactor, antiBallVelDir);    //TODO 关键在于如何使得迭代结果收敛于slackTime=0的点
			}
			else if (slackTime < stableSlackTime - 0.2&&(me2projDist>15||distToBall>distBack&&ballSpeed>310)){
					targetPoint = projPoint + Utils::Polar2Vector(ballSpeed*ballSpeedFactor, BallVelDir);

			}
		

			//In case that the robot has to get out of the field to intercept the ball
				if (Utils::OutOfField(targetPoint, 10)&& ballSpeed>230){
					if (inter.intersectant()){
						if (BallVelDir > 0){
							targetPoint.setY(max(inter.point1().y(), inter.point2().y()));
						}
						else{
							targetPoint.setY(min(inter.point1().y(), inter.point2().y()));
						}
						if (fabs(BallVelDir) < Param::Math::PI / 2)
						{
							targetPoint.setX(max(inter.point1().x(), inter.point2().x()));
						}
						else{
							targetPoint.setX(min(inter.point1().x(), inter.point2().x()));
						}
					}
				}
			targetPoint = checkPointAvoidOurPenalty(pVision, targetPoint);
			setSubTask(PlayerRole::makeItGoto(robotNum, targetPoint, facedir, CVector(0, 0), 0, playerFlag));
			
			break;
		case SPEED_UP:
			if (debugEngine)
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0, 0), "SPEED_UP");
			if (ballSpeed<200||fabs(BallVelDir)>math_onedir*60){
				if ((ballVelSeg.IsPointOnLineOnSegment(projPoint)) && ball2projDist >ALLOWANCE_DIST_AVOID_BALL){
					ballSpeedFactor = 2 * (ballSpeed - meSpeed) - ball2projDist;
					targetPoint = ballPos + Utils::Polar2Vector(250 + ballSpeedFactor*2, BallVelDir);
				}
				else{
					ballSpeedFactor = ball2projDist + (ballSpeed - meSpeed);
					if (Utils::Normalize(dirToBall - BallVelDir) > 0){
						ballDeviantPos = ballPos + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_SIZE * 3 + Param::Field::BALL_SIZE, Utils::Normalize(BallVelDir - math_pi / 2));
					}
					else{
						ballDeviantPos = ballPos + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_SIZE * 3 + Param::Field::BALL_SIZE, Utils::Normalize(BallVelDir + math_pi / 2));
					}
					targetPoint = ballDeviantPos + Utils::Polar2Vector(400 + ballSpeedFactor * 4, BallVelDir);
				}
				if (Utils::OutOfField(targetPoint, 10) && ballSpeed > 230){
					if (inter.intersectant()){
						if (BallVelDir > 0){
							targetPoint.setY(max(inter.point1().y(), inter.point2().y()));
						}
						else{
							targetPoint.setY(min(inter.point1().y(), inter.point2().y()));
						}
						if (fabs(BallVelDir) < Param::Math::PI / 2)
						{
							targetPoint.setX(max(inter.point1().x(), inter.point2().x()));
						}
						else{
							targetPoint.setX(min(inter.point1().x(), inter.point2().x()));
						}
					}
				}
				targetPoint = checkPointAvoidOurPenalty(pVision, targetPoint);
				setSubTask(PlayerRole::makeItGoto(robotNum, targetPoint, dirToBall, CVector(0, 0), 0, playerFlag));
			}
			else{
				//cout <<pVision->Cycle()<< ":in"<< endl;
				setSubTask(PlayerRole::makeItChaseKickV2(robotNum, finalKickDir, playerFlag));
			}
	
			break;
		case STABLE_BALL:
			if (debugEngine)
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0, 0), "STABLE_BALL");
			if (canNormalKick){
				if (task().player.ischipkick){
					KickStatus::Instance()->setChipKick(robotNum, 1200);
				}else{
					KickStatus::Instance()->setKick(robotNum, kickPower);
				}
				if (!isMeHasBall){
					interTask.player.rotvel =0.0;
					interTask.player.vel = Utils::Polar2Vector(25, task().player.angle);
					interTask.player.angle = task().player.angle;
					interTask.player.flag = task().player.flag;
					DribbleStatus::Instance()->setDribbleCommand(robotNum, 0);
					setSubTask(TaskFactoryV2::Instance()->NoneTrajGetBall(interTask));
				}
			}
			if (ballSpeed > 100 && _drribleCounter<120){
				_drribleCounter++;//TODO
				DribbleStatus::Instance()->setDribbleCommand(robotNum, 3);
			}
			else if (ballSpeed <=50 && distToBall < ALLOWANCE_DIST_TO_STABLE){
				if (fabs(Utils::Normalize(meDir - finalKickDir)) < ALLOWANCE_DIR_TO_DRRIBLE){
					interTask.player.rotvel = Param::Vehicle::V2::PLAYER_SIZE / 5;
					interTask.player.vel = Utils::Polar2Vector(25, task().player.angle);
					interTask.player.angle = task().player.angle;
					interTask.player.flag = task().player.flag;
					DribbleStatus::Instance()->setDribbleCommand(robotNum, 0);
					setSubTask(TaskFactoryV2::Instance()->NoneTrajGetBall(interTask));
				}
				else{
					DribbleStatus::Instance()->setDribbleCommand(robotNum, 3);
					setSubTask(PlayerRole::makeItDribbleTurn(robotNum, finalKickDir, Param::Math::PI / 90));
				}
			}
			else{
				targetPoint = checkPointAvoidOurPenalty(pVision, targetPoint);
				setSubTask(PlayerRole::makeItGoto(robotNum, me.Pos(), dirToBall, CVector(0, 0), 0, playerFlag));
			}
			break;
		case TOUCH_KICK:
			if (debugEngine)
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0, 0), "TOUCH_KICK");
			setSubTask(PlayerRole::makeItTouchKick(robotNum, finalKickDir, false, playerFlag));
			break;
		case TURN_KICK:
			if (debugEngine)
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0, 0), "TURN_KICK");
			DribbleStatus::Instance()->setDribbleCommand(robotNum, 3);
			if (canNormalKick){
				if (task().player.ischipkick){
					KickStatus::Instance()->setChipKick(robotNum,1200);
				}
				else{
					KickStatus::Instance()->setKick(robotNum, kickPower);
				}
				if (!isMeHasBall){
					interTask.player.rotvel = 0.0;
					interTask.player.vel = Utils::Polar2Vector(25, task().player.angle);
					interTask.player.angle = task().player.angle;
					interTask.player.flag = task().player.flag;
					DribbleStatus::Instance()->setDribbleCommand(robotNum, 0);
					setSubTask(TaskFactoryV2::Instance()->NoneTrajGetBall(interTask));
				}
			}
			if (distToBall > ALLOWANCE_DIST_TO_DRIBBLE*3){
				interTask.player.rotvel = 0.0;
				interTask.player.vel = Utils::Polar2Vector(25, task().player.angle);
				interTask.player.angle = task().player.angle;
				interTask.player.flag = task().player.flag;
				setSubTask(TaskFactoryV2::Instance()->NoneTrajGetBall(interTask));
			}
			else{
				setSubTask(PlayerRole::makeItDribbleTurn(robotNum, finalKickDir, Param::Math::PI / 90));
			}
			break;
		case CHASE_KICK:
			if (debugEngine)
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0, 0), "CHASE_KICK");
			setSubTask(PlayerRole::makeItChaseKickV2(robotNum, finalKickDir, playerFlag));
			break;
	}
	
	_lastCycle = pVision->Cycle();
	CPlayerTask::plan(pVision);
}


CPlayerCommand* CInterceptBallV2::execute(const CVisionModule* pVision)
{
	if (subTask()){
		return subTask()->execute(pVision);
	}
	if (_directCommand != NULL){
		return _directCommand;
	}
	return NULL;
}

int CInterceptBallV2::checkEnemy(const CVisionModule* pVision){
	const CBestPlayer::PlayerList& oppList = BestPlayer::Instance()->theirFastestPlayerToBallList();
	if (oppList.size() > 0)
		return oppList[0].num;
	else
		return 0;
}

double CInterceptBallV2::enemyMenacePoint(const CVisionModule* pVision, const int enemyNum){
	int roboNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(roboNum);
	const PlayerVisionT& enemy = pVision->TheirPlayer(enemyNum);
	const MobileVisionT& ball = pVision->Ball();
	CGeoPoint enemyPos = enemy.Pos();
	double enemyDistToBall = (enemy.Pos() - ball.RawPos()).mod();
	double meDistToBall = (me.Pos() - ball.RawPos()).mod();
	if (enemyDistToBall < meDistToBall){
		if (meDistToBall - enemyDistToBall>150)
			return MAX_CHANGE_OF_SLACK_TIME;
		else if (meDistToBall - enemyDistToBall > 75)
			return MAX_CHANGE_OF_SLACK_TIME / 2;
		else if (meDistToBall - enemyDistToBall > 25)
			return MAX_CHANGE_OF_SLACK_TIME / 4;
		else
			return MAX_CHANGE_OF_SLACK_TIME / 8;
	}
	else
		return 0;
}


bool CInterceptBallV2::faceTheirGoal(const CVisionModule* pVision, const int executor) {
	double dist = Param::Field::PITCH_LENGTH / 2 - pVision->OurPlayer(executor).Pos().x();
	double buffer = 0.05 * dist + 10;
	const CGeoPoint theirLeft = CGeoPoint(Param::Field::PITCH_LENGTH / 2, -Param::Field::GOAL_WIDTH / 2 - buffer);
	const CGeoPoint theirRight = CGeoPoint(Param::Field::PITCH_LENGTH / 2, Param::Field::GOAL_WIDTH / 2 + buffer);
	double leftAngle = Utils::Normalize((theirLeft - pVision->OurPlayer(executor).Pos()).dir());
	double rightAngle = Utils::Normalize((theirRight - pVision->OurPlayer(executor).Pos()).dir());
	double myDir = pVision->OurPlayer(executor).Dir();
	double rawLeftAngle = Utils::Normalize((theirLeft - pVision->OurPlayer(executor).RawPos()).dir());
	double rawRifhtAngle = Utils::Normalize((theirRight - pVision->OurPlayer(executor).RawPos()).dir());
	double myRawDir = pVision->OurRawPlayer(executor).dir;

	bool isToTheirGoal = ((myDir > leftAngle) && (myDir < rightAngle));
	bool isRawToTheirGoal = ((myRawDir > rawLeftAngle) && (myRawDir < rawRifhtAngle));

	if (debugEngine) {
		GDebugEngine::Instance()->gui_debug_line(theirLeft, pVision->OurPlayer(executor).Pos());
		GDebugEngine::Instance()->gui_debug_line(theirRight, pVision->OurPlayer(executor).Pos());
		GDebugEngine::Instance()->gui_debug_line(pVision->OurPlayer(executor).Pos() + Utils::Polar2Vector(100, myDir), pVision->OurPlayer(executor).Pos());

		GDebugEngine::Instance()->gui_debug_line(theirLeft, pVision->OurPlayer(executor).RawPos());
		GDebugEngine::Instance()->gui_debug_line(theirRight, pVision->OurPlayer(executor).RawPos());
		GDebugEngine::Instance()->gui_debug_line(pVision->OurPlayer(executor).RawPos() + Utils::Polar2Vector(100, myRawDir), pVision->OurPlayer(executor).RawPos());
	}
	return (isToTheirGoal || isRawToTheirGoal);
}



bool CInterceptBallV2::isVisionHasBall(const CVisionModule* pVision, const int vecNumber)
{

	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const MobileVisionT& ball = pVision->Ball();
	double visionJudgDist = Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + Param::Field::BALL_SIZE / 2 + 6;
	bool distVisionHasBall = CVector(me.Pos() - ball.Pos()).mod() < visionJudgDist;
	bool dirVisionHasBall;
	double meDir = me.Dir();
	double me2Ball = (ball.Pos() - me.Pos()).dir();
	double meDir_me2Ball_Diff = Utils::Normalize(abs(meDir - me2Ball));
	if (meDir_me2Ball_Diff < Param::Math::PI / 4.0)
	{
		dirVisionHasBall = true;
	}
	else
		dirVisionHasBall = false;
	bool isVisionPossession = dirVisionHasBall && distVisionHasBall;
	return isVisionPossession;
}


CGeoPoint CInterceptBallV2::checkPointAvoidOurPenalty(const CVisionModule* pVision, CGeoPoint targetPoint){
	CGeoPoint recalcpoint;
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint leftCircleCenter(-Field::PITCH_LENGTH / 2, -Field::PENALTY_AREA_L / 2);
	const CGeoPoint rightCircleCenter(-Field::PITCH_LENGTH / 2, Field::PENALTY_AREA_L / 2);
	const CGeoCirlce leftPenaltyCircle(leftCircleCenter, Field::PENALTY_AREA_R);
	const CGeoCirlce rightPenaltyCircle(rightCircleCenter, Field::PENALTY_AREA_R);
	const CGeoPoint leftPenaltySegmentEnd(-Field::PITCH_LENGTH / 2 + Field::PENALTY_AREA_DEPTH, -Field::PENALTY_AREA_L / 2);
	const CGeoPoint rightPenaltySegmentEnd(-Field::PITCH_LENGTH / 2 + Field::PENALTY_AREA_DEPTH, Field::PENALTY_AREA_L / 2);
	const CGeoLine middlePenaltySegment(leftPenaltySegmentEnd, rightPenaltySegmentEnd);

	const CGeoLine ballVelLine(ball.Pos(), targetPoint);

	const CGeoLineLineIntersection penaltyLineIntersection(ballVelLine, middlePenaltySegment);
	const CGeoLineCircleIntersection leftPenaltyIntersection(ballVelLine, leftPenaltyCircle);
	const CGeoLineCircleIntersection rightPenaltyIntersection(ballVelLine, rightPenaltyCircle);


	if (!Utils::InOurPenaltyArea(targetPoint, 0)){
		recalcpoint = targetPoint;
	}
	else{
		const CGeoLine ballVelLine(ball.Pos(), targetPoint);
		const CGeoPoint leftCircleCenter(-Field::PITCH_LENGTH / 2, -Field::PENALTY_AREA_L / 2);
		const CGeoPoint rightCircleCenter(-Field::PITCH_LENGTH / 2, Field::PENALTY_AREA_L / 2);
		const CGeoCirlce leftPenaltyCircle(leftCircleCenter, Field::PENALTY_AREA_R);
		const CGeoCirlce rightPenaltyCircle(rightCircleCenter, Field::PENALTY_AREA_R);
		const CGeoPoint leftPenaltySegmentEnd(-Field::PITCH_LENGTH / 2 + Field::PENALTY_AREA_DEPTH, -Field::PENALTY_AREA_L / 2);
		const CGeoPoint rightPenaltySegmentEnd(-Field::PITCH_LENGTH / 2 + Field::PENALTY_AREA_DEPTH, Field::PENALTY_AREA_L / 2);
		const CGeoLine middlePenaltySegment(leftPenaltySegmentEnd, rightPenaltySegmentEnd);

		const CGeoLineLineIntersection penaltyLineIntersection(ballVelLine, middlePenaltySegment);
		const CGeoLineCircleIntersection leftPenaltyIntersection(ballVelLine, leftPenaltyCircle);
		const CGeoLineCircleIntersection rightPenaltyIntersection(ballVelLine, rightPenaltyCircle);

		/* 球速线和禁区边界最多三个交点 */
		const CGeoPoint* points[3];
		points[0] = 0;
		points[1] = 0;
		points[2] = 0;

		/* 交点在中间直线上 */
		if (penaltyLineIntersection.Intersectant()) {
			const CGeoPoint& point = penaltyLineIntersection.IntersectPoint();
			if (point.y() <= rightPenaltySegmentEnd.y() && point.y() >= leftPenaltySegmentEnd.y()) {
				points[0] = &point;
			}
		}

		/* 交点在两段圆弧上 */
		if (leftPenaltyIntersection.intersectant() || rightPenaltyIntersection.intersectant()) {
			const CGeoPoint* pointL[2] = { 0 };
			const CGeoPoint* pointR[2] = { 0 };
			if (leftPenaltyIntersection.intersectant()) {
				pointL[0] = &leftPenaltyIntersection.point1();
				pointL[1] = &leftPenaltyIntersection.point2();
				if (pointL[0]->x() < leftCircleCenter.x() || pointL[0]->y() > leftCircleCenter.y()) pointL[0] = 0;
				if (pointL[1]->x() < leftCircleCenter.x() || pointL[1]->y() > leftCircleCenter.y()) pointL[1] = 0;
			}
			if (rightPenaltyIntersection.intersectant()) {
				pointR[0] = &rightPenaltyIntersection.point1();
				pointR[1] = &rightPenaltyIntersection.point2();
				if (pointR[0]->x() < rightCircleCenter.x() || pointR[0]->y() < rightCircleCenter.y()) pointR[0] = 0;
				if (pointR[1]->x() < rightCircleCenter.x() || pointR[1]->y() < rightCircleCenter.y()) pointR[1] = 0;
			}
			if (pointL[0] != 0) {
				if (points[0] == 0) points[0] = pointL[0];
				else                     points[1] = pointL[0];
			}
			if (pointL[1] != 0) {
				if (points[0] == 0) points[0] = pointL[1];
				else if (points[1] == 0) points[1] = pointL[1];
				else                     points[2] = pointL[1];
			}
			if (pointR[0] != 0) {
				if (points[0] == 0) points[0] = pointR[0];
				else if (points[1] == 0) points[1] = pointR[0];
				else if (points[2] == 0) points[2] = pointR[0];
			}
			if (pointR[1] != 0) {
				if (points[0] == 0) points[0] = pointR[1];
				else if (points[1] == 0) points[1] = pointR[1];
				else if (points[2] == 0) points[2] = pointR[1];
			}
		}


		/* 综合考虑交点 找出最合适的 */
		const CGeoPoint* defendTarget = 0;
		if (ball.VelY() > 0) {
			int count = 0;
			for (int i = 0; i < 3; ++i) {
				if (points[i] != 0)
					++count;
			}
			if (count > 0) {
				const CGeoPoint* tmpRightest = points[0];
				for (int i = 1; i < count; ++i) {
					if (points[i]->y() > tmpRightest->y())
						tmpRightest = points[i];
				}
				if (tmpRightest->y() > ball.RawPos().y())
					defendTarget = tmpRightest;
			}
			/*
			if (points[0] != 0) {
			if (points[0]->y() > ball.RawPos().y()) {
			defendTarget = points[0];
			if (points[1] != 0) {
			if (points[1]->y() > defendTarget->y()) {
			defendTarget = points[1];
			if (points[2] != 0) {
			if (points[2]->y() > defendTarget->y())
			defendTarget = points[2];
			}
			}
			}
			}
			}
			*/
		}
		else {
			int count = 0;
			for (int i = 0; i < 3; ++i) {
				if (points[i] != 0)
					++count;
			}
			if (count > 0) {
				const CGeoPoint* tmpLeftest = points[0];
				for (int i = 1; i < count; ++i) {
					if (points[i]->y() < tmpLeftest->y())
						tmpLeftest = points[i];
				}
				if (tmpLeftest->y() < ball.RawPos().y())
					defendTarget = tmpLeftest;
			}
			/*
			if (points[0] != 0) {
			if (points[0]->y() < ball.RawPos().y()) {
			defendTarget = points[0];
			if (points[1] != 0) {
			if (points[1]->y() < defendTarget->y()) {
			defendTarget = points[1];
			if (points[2] != 0) {
			if (points[2]->y() < defendTarget->y())
			defendTarget = points[2];
			}
			}
			}
			}
			}
			*/
		}
		if (defendTarget != 0) {
			/* 防角平分线 */
			double leftPostToBallDir = (*defendTarget - LEFT_GOAL_POST).dir();
			double rightPostToBallDir = (*defendTarget - RIGHT_GOAL_POST).dir();
			double defendDir = Utils:: Normalize((leftPostToBallDir + rightPostToBallDir) / 2 + Math::PI);

			/* 角度修正 */
			if (defendDir > 30 / 180 * Math::PI)
				defendDir -= 10 / 180 * Math::PI;
			else if (defendDir < -30 / 180 * Math::PI)
				defendDir += 10 / 180 * Math::PI;
			recalcpoint = *defendTarget + Utils::Polar2Vector(Vehicle::V2::PLAYER_SIZE*2.5, defendDir);
		}
	}
	return recalcpoint;
}

