#include "AdvanceBallV1.h"
#include <VisionModule.h>
#include <KickStatus.h>
#include "RobotSensor.h"
#include "skill/Factory.h"
#include <WorldModel.h>
#include "DribbleStatus.h"
#include "IndirectDefender.h"
#include <utils.h>
#include <BestPlayer.h>
#include "KickDirection.h"
#include <GDebugEngine.h>
#include <iostream>
#include <BestPlayer.h>

namespace{

	bool DEBUG_ENGINE = false; // 是否在strategy中绘制
	bool WEAK_OPP = true;      // 针对静态队
	bool GENTLE_MODE = false;
	//对方禁区点信息
	const CGeoPoint theirLeft = CGeoPoint(Param::Field::PITCH_LENGTH/2, -Param::Field::GOAL_WIDTH/2 );

	const CGeoPoint theirRight = CGeoPoint(Param::Field::PITCH_LENGTH/2, Param::Field::GOAL_WIDTH/2 );

	const CGeoPoint theirCenter = CGeoPoint(Param::Field::PITCH_LENGTH/2, 0);
	const CGeoPoint ourGoal = CGeoPoint(-Param::Field::PITCH_LENGTH/2, 0);
	const CGeoPoint defenceGoal0(-Param::Field::PITCH_LENGTH/2, 0);
	const int maxFrared = 100;
	const int maxMeHasBall = 50;
	double OPP_HAS_BALL_DIST = 30;
	double CIRCLE_FRONT = true;
	CGeoPoint pullBallTarget = CGeoPoint(0,0);
	int kickPower = 9999;
	int meHasBallMaxCnt = 1;
}
CAdvanceBallV1::CAdvanceBallV1()
{
	DECLARE_PARAM_READER_BEGIN(JAM_AND_PUSH)
	READ_PARAM(WEAK_OPP)
	READ_PARAM(GENTLE_MODE)
	READ_PARAM(OPP_HAS_BALL_DIST)
	READ_PARAM(CIRCLE_FRONT)
	DECLARE_PARAM_READER_END
	dribble_last_state = 0;
	isBallInPenalty = false;
	pullCnt = 0;
	turnCnt = 0;
	pullBackFlag = false;
	dribbleFlag = false;
}

CAdvanceBallV1::~CAdvanceBallV1()
{

}

void CAdvanceBallV1::plan(const CVisionModule* pVision)
{
	//Cycle循环
	if ( pVision->Cycle() - _cycle > 1 ){
		_state = BEGINNING;
	}
	//任务信息
	int _executor = task().executor;
	int flag = task().player.flag | PlayerStatus::DODGE_OUR_DEFENSE_BOX;
	int flag_not_dribble = flag & (~PlayerStatus::DRIBBLING);
	//图像信息
	const PlayerVisionT& me = pVision->OurPlayer(_executor);
	const MobileVisionT& ball = pVision->Ball();
	double kickDir = (CGeoPoint(Param::Field::PITCH_LENGTH / 2.0,0) - pVision->Ball().Pos()).dir();
	//角度信息
	const CVector self2Ball = ball.Pos() - me.Pos();
	CVector ball2TheirCenter = theirCenter - ball.Pos();
	double faceDir = kickDir;
	//球信息
	double ballMovingSpeed = ball.Vel().mod();
	//红外信息
	bool frared = RobotSensor::Instance()->IsInfraredOn(_executor);

	//检测有连续多少帧有红外
	if ( frared ){
		infraredOn = infraredOn >= maxFrared ? maxFrared : infraredOn+1;
		infraredOff = 0;
	}
	else{
		infraredOn = 0;
		infraredOff = infraredOff >= maxFrared ? maxFrared : infraredOff+1;
	}
	bool is_runner_dir_ok = WorldModel::Instance()->KickDirArrived(pVision->Cycle(), kickDir, Param::Math::PI / 36, _executor);
	bool is_runner_dir_ok_penaltyArea = WorldModel::Instance()->KickDirArrived(pVision->Cycle(), kickDir, Param::Math::PI / 9, _executor);
	/*cout << is_runner_dir_ok<<endl;*/
	_pushBallTarget = theirCenter;
	//对敌方的处理1
	bool isOppHasBall = checkOppHasBall(pVision);
	const PlayerVisionT& opp = pVision->TheirPlayer(opponentID);
	const CVector opp2ball = ball.Pos() - opp.Pos();
	double ball2oppDist = opp2ball.mod();
	CVector me2goal = theirCenter - me.Pos();
	double me2BestOppDist = CVector(pVision->TheirPlayer(opponentID).Pos() - me.Pos()).mod();
	//自已有球的判断
	bool isMeHasBall;
	bool isMechHasBall = infraredOn > 1;
	bool visionHasBall = (isVisionHasBall(pVision,_executor))|| (infraredOn > 1 && !ball.Valid());
	//	cout << infraredOn<<endl;

	isMeHasBall = isMechHasBall || visionHasBall;
	if (isMeHasBall) {
		meHasBall = meHasBall >=maxMeHasBall ? maxMeHasBall : meHasBall +1;
		meLoseBall = 0;
	}
	else {
		meHasBall = 0;
		meLoseBall = meLoseBall >=maxMeHasBall? maxMeHasBall:meLoseBall +1;
	}
	
	double opp2GoalLeftDist = (opp.Pos() - theirLeft).mod();
	double opp2GoalRightDist = (opp.Pos() - theirRight).mod();
	bool oppNearLeft = opp2GoalRightDist > opp2GoalLeftDist;
	

		switch(_state)
		{
		case BEGINNING: 
				_state = GOTO;
				if (DEBUG_ENGINE)
				{
						cout << "BEGINNING-->GOTO"<<endl;
				}
			break;
		case GOTO:

			if (pullBackFlag == true && isOppFaceTheirDoor(pVision,Param::Math::PI/2) || (Utils::InTheirPenaltyArea(ball.Pos(),0) && oppBlockMe(pVision,_executor)))
			{
				if (pullCnt > 30 && meHasBall>meHasBallMaxCnt)
				{
					pullCnt = 0;
					pullBackFlag = false;
					pullBallTarget = me.Pos() + Utils::Polar2Vector(25,Utils::Normalize(me.Dir()+Param::Math::PI));
					_state = DRIBBLE_PULL;
					if (DEBUG_ENGINE)
					{
						cout << "goto-->dribblePushPull"<<endl;
					}
				}
			}
			else if ((Utils::InTheirPenaltyArea(ball.Pos(),0) && oppBlockMe(pVision,_executor)))
			{
				pullCnt++;
			}
			else if (!isBallInPushArea(pVision)&&GENTLE_MODE && ballBetweentMeAndOpp(pVision,_executor)&& checkOppHasBall(pVision)&& ballOnTheirHalf(pVision))
			{
				if (DEBUG_ENGINE)
				{
				//	cout <<"here"<<endl;
				}
				if (meHasBall > 30)
				{
					if (DEBUG_ENGINE)
					{
						cout << "goto-->turn!!!!!!!!!!!!!!!!!!"<<endl;
					}
					_state = TURN;
					turnPos = me.Pos();
					finalTurnDir =Utils::Normalize(me.Dir() + Param::Math::PI)  ;
				}
			}
			else
			{
				if (DEBUG_ENGINE)
				{
				//	cout <<"there"<<endl;
				}
				if (meHasBall > meHasBallMaxCnt && isBallInPushArea(pVision)&&oppBlockMe(pVision,_executor) )
				{ 
					if (DEBUG_ENGINE)
						cout << "goto-->crazyPush" << endl;
					_state = CRAZYPUSH;
				}/*else if ((meHasBall > 5 &&dribbleFlag== true&& pullBackFlag == true&&isOppFaceTheirDoor(pVision,Param::Math::PI/2)) || (meHasBall > 5 && Utils::InTheirPenaltyArea(ball.Pos(),0) && oppBlockMe(pVision,_executor)))
				 {
				 if (DEBUG_ENGINE)

				 _state = DRIBBLE_PULL;
				 pullBackFlag = false;
				 pullBallTarget = me.Pos() + Utils::Polar2Vector(25,Utils::Normalize(me.Dir()+Param::Math::PI));
				 if (DEBUG_ENGINE)
				 {
				 cout << "goto-->dribblePushPull"<<endl;
				 }
				 }*/else if (meHasBall > meHasBallMaxCnt && oppBlockMe(pVision,_executor))
				{
					if (DEBUG_ENGINE)
						cout << "goto-->normalPush" << endl;
					_state = NORMAL_PUSH;
				}else if (meHasBall > meHasBallMaxCnt)
				{
					if (DEBUG_ENGINE)
						cout << "goto-->kick" << endl;
					_state = KICK;
				}
			}
			break;
		case TURN:
			/*if (meLoseBall > 100)
			{
				if (DEBUG_ENGINE)
				{
					cout << "turn-->gpto:LoseBall"<<endl;
				}
				_state = GOTO;
			}else*/
			{ 
				if (turnCnt > 1)
				{
					if (DEBUG_ENGINE)
					{
						cout << "turn-->gpto"<<endl;
					}
				_state = GOTO;
					turnCnt = 0;
				}
				else turnCnt ++;
			}
			break;
			case  NORMAL_PUSH:
				if (meHasBall>meHasBallMaxCnt && isMePassedOpp(pVision,_executor))
				{
					_state = KICK;
				}else if (meLoseBall >5)
				{
					_state = GOTO;
				}
				break;
		case CRAZYPUSH:
			if (self2Ball.mod() > 30 || abs(Utils::Normalize(me.Dir() - me2goal.dir())) > Param::Math::PI / 4)
			{
				if (DEBUG_ENGINE)
				{
					cout << "crazzPush-->goto/n";
				}
				_state = GOTO;
			}else if (isMePassedOpp(pVision,_executor))
			{
				if (DEBUG_ENGINE)
				{
					cout << "crazzPush-->kick/n";
				}
				_state = KICK;
			}
			break;
		case DRIBBLE_PULL:
			if ( meLoseBall > 30 || (self2Ball.mod() > 30 && ball.Valid()))
			{
				if (DEBUG_ENGINE)
					cout << "dribble-->goto" << endl;
				_state = GOTO;
			}if (me.Pos().dist(pullBallTarget)< 10&& meHasBall > meHasBallMaxCnt )
			{  
				if (DEBUG_ENGINE)
					cout << "dribble-->kick" << endl;
				_state = KICK;
			}
			break;
		case KICK:
			if (meLoseBall > 10)
			{
				if (DEBUG_ENGINE)
					cout << "kick-->goto" << endl;
				_state =GOTO;
			}
	break;
		case  STOP:
			setSubTask(PlayerRole::makeItStop(_executor));
			break;
		default:
			_state = BEGINNING;
			break;
		}

	bool isToTheirGoal = faceTheirGoal(pVision, _executor);      // 2014-05-25 方向朝对方球门时强制开射门 yys
	if (isToTheirGoal && _state != NORMAL_PUSH && _state != CRAZYPUSH && _state != TURN && _state != DRIBBLE_PULL && _state != STOP) {
		KickStatus::Instance()->setKick(_executor, kickPower); 
	}

	flag = flag | PlayerStatus::DRIBBLING;

	//DribbleStatus::Instance()->setDribbleCommand(_executor, 3);
	if (ball.X() < -Param::Field::PITCH_LENGTH/4 && abs(ball.Y()) > Param::Field::PENALTY_AREA_R )
		_pushBallTarget = ball.Pos() + Utils::Polar2Vector(100, 0);
	else
		calcPushTarget(pVision);

	switch(_state)
	{	
	case GOTO:
		//cout << "ballmoving2opp: " << ballMoving2Opp(pVision,_executor) << " ballmod: "<<ball.Vel().mod() <<" opp2ballmod: "<< opp2ball.mod()<<endl;
		if (Utils::InOurPenaltyArea(ball.Pos(),0))
		{
			blockingOuterOppTime(pVision,_executor);
		}else if (checkUsingVoilence(pVision,_executor))
		{
			voilenceActionTime(pVision,_executor);
		}else if ( (ballMoving2Opp(pVision,_executor)&& ball.Vel().mod() > 160 ))
		{ 
			//cout << "ball move to opp && ballvel > 300"<<endl;
			blockingBestOppTime(pVision,_executor);
			//setSubTask(PlayerRole::makeItJamBall(_executor,_pushBallTarget,flag));
		}
		else
		{
			KickDirection::Instance()->GenerateShootDir(_executor, pVision->OurPlayer(_executor).Pos());
			kickDir = KickDirection::Instance()->getRealKickDir();

			if (!ballOnTheirHalf(pVision))//后半场
			{						
				if (checkBallFront(pVision)&& ball2oppDist < Param::Vehicle::V2::PLAYER_SIZE*4 && ballBetweentMeAndOpp(pVision,_executor))
				{
					//cout <<"face opp get ball" << endl;
					faceDir = opp.Dir() + Param::Math::PI;
					if (WEAK_OPP)
					{
						setSubTask(PlayerRole::makeItNoneTrajGetBall(_executor,faceDir,CVector(0,0),flag_not_dribble,-2));
					}else{
						setSubTask(PlayerRole::makeItChaseKickV1(_executor,faceDir,flag_not_dribble));
					}	
				}else
				{
					if (WEAK_OPP)
					{
						setSubTask(PlayerRole::makeItNoneTrajGetBall(_executor,kickDir,CVector(0,0),flag_not_dribble,-2));
					}else
					{
						setSubTask(PlayerRole::makeItChaseKickV1(_executor,kickDir,flag_not_dribble));
					}					
				}
			}else//前半场
			{
				if (Utils::InTheirPenaltyArea(ball.Pos(),0))
				{
					if (!Utils::InTheirPenaltyArea(me.Pos(),0))
					{
						setSubTask(PlayerRole::makeItGoto(_executor,ball.Pos(),kickDir,CVector(0,0),0,flag_not_dribble));
					}else
					{
						KickDirection::Instance()->GenerateShootDir(_executor, pVision->OurPlayer(_executor).Pos());
						kickDir = KickDirection::Instance()->getRealKickDir();
						//setSubTask(PlayerRole::makeItNoneTrajGetBall(_executor,kickDir,CVector(0,0),flag,-2));
						setSubTask(PlayerRole::makeItChaseKickV1(_executor,kickDir,flag_not_dribble));
					}
				}else //不在禁区
				{
				//	cout << "ballbetweeOpp:"<<ballBetweentMeAndOpp(pVision,_executor) << " "<<"lala:"<<(abs(Utils::Normalize(me2goal.dir() - self2Ball.dir())) < Param::Math::PI / 2.0) << endl;
					pullBackFlag = false;
					dribbleFlag = false;
					if ( isOppFaceTheirDoor(pVision,Param::Math::PI/2)&&ballBetweentMeAndOpp(pVision,_executor) )
					{
					//	cout << "SDFSDF"<<endl;
						pullBackFlag = true;
						if (meHasBall > 1)
						{
							pullCnt++;
						}
						/*		TaskT task;
						task.executor = _executor;
						task.player.angle = (opp.Pos() - ball.Pos()).dir();

						task.player.rotvel = -2;
						task.player.flag =flag;
						if (!WEAK_OPP)
						{
						task.player.max_acceleration = 900;
						}*/
					//	setSubTask(TaskFactoryV2::Instance()->NoneTrajGetBall(task));
						if (WEAK_OPP)
						{
							setSubTask(PlayerRole::makeItNoneTrajGetBall(_executor,(opp.Pos() - ball.Pos()).dir(),CVector(0,0),flag_not_dribble,-2));
						}else
						{
							faceDir =  (opp.Pos() - ball.Pos()).dir();
							setSubTask(PlayerRole::makeItChaseKickV1(_executor,faceDir,flag_not_dribble));
						}			
					}
					//else if (is_runner_dir_ok)
					//{
					//	if (selectChipKick(pVision,_executor))
					//	{
					//		KickStatus::Instance()->setKick(_executor,me2goal.mod());
					//	}else
					//	{
					//		KickStatus::Instance()->setKick(_executor,9999);
					//	}
					//	setSubTask(PlayerRole::makeItChaseKickV1(_executor,kickDir,flag));
					////	setSubTask(PlayerRole::makeItNoneTrajGetBall(_executor,kickDir,CVector(0,0),flag,-2));
					//}
					else
					{
						KickDirection::Instance()->GenerateShootDir(_executor, pVision->OurPlayer(_executor).Pos());
						kickDir = KickDirection::Instance()->getRealKickDir();
						/*		TaskT task;
						task.executor = _executor;
						task.player.angle = kickDir;
						task.player.rotvel = -2;
						task.player.flag =flag;
						if (!WEAK_OPP)
						{
						task.player.max_acceleration = 900;
						}*/
						//setSubTask(TaskFactoryV2::Instance()->NoneTrajGetBall(task));
						if (WEAK_OPP)
						{
							setSubTask(PlayerRole::makeItNoneTrajGetBall(_executor,kickDir,CVector(0,0),flag_not_dribble,-2));
						}else
						{
							//std::cout << "here here here here here!!!!!!!!!!!!!" << std::endl;
							if (ball2oppDist > 75.0)
							{
								setSubTask(PlayerRole::makeItNoneTrajGetBall(_executor,kickDir,CVector(0,0),flag_not_dribble,-2));
							}
							else{
								setSubTask(PlayerRole::makeItChaseKickV1(_executor,kickDir,flag_not_dribble));
							}
						}
					}
				}
			}
		}
		break;
		case TURN:
		//setSubTask(PlayerRole::makeItSimpleGoto(_executor,turnPos,finalTurnDir,CVector(0,0),0,flag));
		//	setSubTask(PlayerRole::makeItNoneTrajGetBall(_executor,finalTurnDir,CVector(0,0),flag,-2));
			setSubTask(PlayerRole::makeItRun(_executor,0,0,20));
			break;
	case DRIBBLE_PULL:
		setSubTask(PlayerRole::makeItSimpleGoto(_executor,pullBallTarget,me.Dir(),CVector(0,0),0,flag));
		break;
	case CRAZYPUSH:
		setSubTask(PlayerRole::makeItCrazyPush(_executor, faceDir));
		break;
	case NORMAL_PUSH:
		KickDirection::Instance()->GenerateShootDir(_executor, pVision->OurPlayer(_executor).Pos());
		kickDir = KickDirection::Instance()->getRealKickDir();
		//setSubTask(PlayerRole::makeItNoneTrajGetBall(_executor,kickDir,CVector(0,0),flag,-2));
		if (WEAK_OPP)
		{
			setSubTask(PlayerRole::makeItNoneTrajGetBall(_executor,kickDir,CVector(0,0),flag_not_dribble,-2));
		}else
		{
			setSubTask(PlayerRole::makeItChaseKickV1(_executor,kickDir,flag_not_dribble));
		}
		break;
	case KICK:
		KickDirection::Instance()->GenerateShootDir(_executor, pVision->OurPlayer(_executor).Pos());
		kickDir = KickDirection::Instance()->getRealKickDir();
		if (ballInTheirPenaltyArea(pVision,_executor))
		{
			if (is_runner_dir_ok_penaltyArea)
			{
				KickStatus::Instance()->setKick(_executor,kickPower);
				setSubTask(PlayerRole::makeItChaseKickV1(_executor,kickDir));
			}else
			{
				//setSubTask(PlayerRole::makeItNoneTrajGetBall(_executor,kickDir,CVector(0,0),flag,-2));
				setSubTask(PlayerRole::makeItChaseKickV1(_executor,kickDir,flag_not_dribble));
			}
		}else{
			if (is_runner_dir_ok)
			{
				if (selectChipKick(pVision,_executor) && ball.Pos().x() > 10 && ball.Pos().x() < 200)
				{
					//cout << "Use chip kick in advance" << endl;
					KickStatus::Instance()->clearAll();
					KickStatus::Instance()->setChipKick(_executor,kickPower); // 力度需要调整
				}else
				{
					KickStatus::Instance()->setKick(_executor,kickPower);
				}
				
			}else
			{
				if (WEAK_OPP)
				{
					setSubTask(PlayerRole::makeItNoneTrajGetBall(_executor,kickDir,CVector(0,0),flag_not_dribble,-2));
				}else{
					setSubTask(PlayerRole::makeItChaseKickV1(_executor,kickDir,flag_not_dribble));
				}
					//setSubTask(PlayerRole::makeItNoneTrajGetBall(_executor,kickDir,CVector(0,0),flag,-2));
			}
	    }
		break;
	}
	
	//cout << _cycle<<endl;
	_cycle = pVision->Cycle();
	CStatedTask::plan(pVision);
}
//位置判断函数
bool CAdvanceBallV1::ballInTheirPenaltyArea(const CVisionModule* pVision,const int vecNumber)
{
	const MobileVisionT& ball = pVision->Ball();
	return Utils::InTheirPenaltyArea(ball.Pos(),0);
}
bool CAdvanceBallV1::isBallInPushArea(const CVisionModule* pVision)
{
	const MobileVisionT& ball = pVision->Ball();
	if(ball.Pos().x() >170 )
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool CAdvanceBallV1::checkBallBehind(const CVisionModule* pVision,double angle)
{
	const PlayerVisionT& opp = pVision->TheirPlayer(opponentID);
	const MobileVisionT& ball = pVision->Ball();
	CVector ball2opp = opp.Pos() - ball.Pos();
	bool ballDirBehindOpp = abs(Utils::Normalize(opp.Dir() - ball2opp.dir())) < angle;
	bool ballDistBehindOpp = ball2opp.mod() < OPP_HAS_BALL_DIST;
	//GDebugEngine::Instance()->gui_debug_line(opp.Pos(),opp.Pos() + Utils::Polar2Vector(200 , 0),COLOR_BLACK);
	bool isBallBehindOpp = ballDirBehindOpp && ballDistBehindOpp;
	return isBallBehindOpp;
}
bool CAdvanceBallV1::checkBallFront(const CVisionModule* pVision,double angle)
{
	const PlayerVisionT& opp = pVision->TheirPlayer(opponentID);
	const MobileVisionT& ball = pVision->Ball();
	CVector opp2ball = ball.Pos() - opp.Pos();
	bool ballDirFrontOpp = abs(Utils::Normalize(opp.Dir() - opp2ball.dir())) < angle;
	//cout << "BallDirFrontOPP:"<<ballDirFrontOpp<<endl;
	bool ballDistFrontOpp = opp2ball.mod() < OPP_HAS_BALL_DIST;
	//GDebugEngine::Instance()->gui_debug_line(opp.Pos(),opp.Pos() + Utils::Polar2Vector(200 , 0),COLOR_BLACK);
	bool isBallFrontOpp = ballDirFrontOpp && ballDistFrontOpp;
	return isBallFrontOpp;
}
bool CAdvanceBallV1::ballBetweentMeAndOpp(const CVisionModule* pVision,const int vecNumber)
{
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const PlayerVisionT& opp = pVision->TheirPlayer(opponentID);
	const MobileVisionT& ball = pVision->Ball();
	
	CVector me2Ball = ball.Pos() - me.Pos();
	CVector ball2Opp = opp.Pos() - ball.Pos();

	if (abs(Utils::Normalize(me2Ball.dir() - ball2Opp.dir())) < Param::Math::PI /3.0)
	{
		return true;
	}else
	{
		return false;
	}
}
//朝向判断函数

bool CAdvanceBallV1::checkBallFrontOrBehindBigPre(const CVisionModule* pVision)
{
	const PlayerVisionT& opp = pVision->TheirPlayer(opponentID);
	const MobileVisionT& ball = pVision->Ball();
	double opp2BallDir = CVector(ball.Pos() - opp.Pos()).dir();
	double oppDir = opp.Dir();
	double opp2BallDir_oppDir_Diff = abs(Utils::Normalize(opp2BallDir - oppDir));
	bool isBallFrontOpp;
	isBallFrontOpp = opp2BallDir_oppDir_Diff< Param::Math::PI * 2.0 / 3.0 /*/ 1.5*/ ;
	return isBallFrontOpp;
}
bool CAdvanceBallV1::isOppFaceTheirDoor(const CVisionModule* pVision,const double angle)
{
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& opp = pVision->TheirPlayer(opponentID);
	double opp2BallDist = (opp.Pos() - ball.Pos()).mod();
	double judgeAngle = abs (Utils::Normalize((opp.Dir() - CVector(CGeoPoint(Param::Field::PITCH_LENGTH / 2.0,0) - opp.Pos() ).dir())) );
	bool isFaceTheirDoor = judgeAngle < angle || judgeAngle == angle;
	bool isBallNearOpp = opp2BallDist < OPP_HAS_BALL_DIST;
	//GDebugEngine::Instance()->gui_debug_line(opp.Pos(), opp.Pos() + Utils::Polar2Vector(200,Utils::Normalize(opp.Dir() + Param::Math::PI / 3)),COLOR_WHITE);
	//GDebugEngine::Instance()->gui_debug_line(opp.Pos(), opp.Pos() + Utils::Polar2Vector(200,Utils::Normalize(opp.Dir() - Param::Math::PI / 3)),COLOR_WHITE);
	return isFaceTheirDoor&&isBallNearOpp;

}
bool CAdvanceBallV1::isOppFaceOurDoor(const CVisionModule* pVision,double angle)
{
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& opp = pVision->TheirPlayer(opponentID);
	double opp2BallDist = (opp.Pos() - ball.Pos()).mod();
	bool isBallNearOpp = opp2BallDist < OPP_HAS_BALL_DIST;
	double judgeAngle = abs (Utils::Normalize((opp.Dir() - CVector(CGeoPoint(-Param::Field::PITCH_LENGTH / 2.0,0) - opp.Pos() ).dir())) );
	bool isFaceOurDoor = judgeAngle < angle || judgeAngle == angle;
	return isFaceOurDoor && isBallNearOpp;
}
bool CAdvanceBallV1::ballOnTheirHalf(const CVisionModule* pVision)
{
	if (pVision->Ball().X() > -50)
	{
		return true;
	}
	else
		return false;
}
bool CAdvanceBallV1::meFaceTheirDoor(const CVisionModule* pVision,const int vecNumber)
{
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	double me2GoalAngle;
	me2GoalAngle =abs(Utils::Normalize((CVector(CGeoPoint(Param::Field::PITCH_LENGTH / 2.0,0) - me.Pos()).dir() - me.Dir()))) ;
	if (me2GoalAngle < Param::Math::PI / 3)
	{
		return true;
	}
	else
		return false;
}
//动作函数
void CAdvanceBallV1::voilenceActionTime(const CVisionModule* pVision,int vecNumber)
{
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const PlayerVisionT& opp = pVision->TheirPlayer(opponentID);

	//cout<<"AdvanceBall::VOILENCE MODE"<<endl;
	CVector opp2me = CVector(me.Pos() - opp.Pos());
	CGeoPoint faceTarget = ball.Pos() ;
	double extr_dist = 0.0;
	CVector self2ball = ball.Pos() - me.Pos();
	if (self2ball.mod() > 60 ) 
	{    
		extr_dist = self2ball.mod() * self2ball.mod() / 500; 
		if (extr_dist > 150) 
		{
			extr_dist = 150;
		}
		CGeoPoint fast_point = faceTarget + Utils::Polar2Vector(extr_dist,(faceTarget - me.Pos()).dir());
		if (! Utils::OutOfField(fast_point,0))
		{
			faceTarget = fast_point;
		}		
	}
	CVector finalSpeed = Utils::Polar2Vector(500,(faceTarget - me.Pos()).dir());
	TaskT task;
	task.executor = vecNumber;
	task.player.angle =Utils::Normalize(self2ball.dir() + Param::Math::PI) ;
	task.player.max_acceleration = 650;
	task.player.pos = opp.Pos();
	task.player.vel = finalSpeed;
	task.player.max_deceleration = 650;
	setSubTask(TaskFactoryV2::Instance()->GotoPosition(task));
//	setSubTask(PlayerRole::makeItSimpleGoto(vecNumber, opp.Pos(),opp2me.dir(),finalSpeed,0,0));
}
void CAdvanceBallV1::blockingOuterOppTime(const CVisionModule* pVision,int vecNumber)
{
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const PlayerVisionT& opp = pVision->TheirPlayer(opponentID);
	int flag =task().player.flag;

	CGeoPoint blockingIntoPenaltyPoint =CGeoPoint(0,0);
	int theirBestPlayerOutOurPenalty = 0;
	double theirBestPlayerOutOurPenalty2Goal = 0;

	const CBestPlayer::PlayerList& oppList = BestPlayer::Instance()->theirFastestPlayerToBallList();
	int OppNum = oppList.size();
	for (int i = 0; i< OppNum;i ++)
	{
		CGeoPoint oppPos = pVision->TheirPlayer(oppList[i].num).Pos();
		if (!Utils::InOurPenaltyArea(oppPos,0))
		{
			theirBestPlayerOutOurPenalty = oppList[i].num;
			posSide side  = POS_SIDE_MIDDLE;
			theirBestPlayerOutOurPenalty2Goal = CVector(oppPos - CGeoPoint(-Param::Field::PITCH_LENGTH/2.0,0)).dir();
			blockingIntoPenaltyPoint = DefendUtils::calcPenaltyLinePoint(theirBestPlayerOutOurPenalty2Goal,side,1);
			break;
		}
	}
	double me2TheOppDir = (pVision->TheirPlayer(theirBestPlayerOutOurPenalty).Pos() - me.Pos()).dir();
	setSubTask(PlayerRole::makeItSimpleGoto(vecNumber,blockingIntoPenaltyPoint,me2TheOppDir,CVector(0,0),0,flag));
}
void CAdvanceBallV1::blockingBestOppTime(const CVisionModule* pVision,int vecNumber)
{
	const PlayerVisionT& opp = pVision->TheirPlayer(opponentID);
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const MobileVisionT& ball = pVision->Ball();
	CVector opp2ourGoal = CGeoPoint(-Param::Field::PITCH_LENGTH / 2.0 , 0) - opp.Pos();
	CGeoSegment oppShootLine = CGeoSegment(opp.Pos(), CGeoPoint(-Param::Field::PITCH_LENGTH / 2.0 , 0));
	CGeoCirlce oppCircle = CGeoCirlce(opp.Pos(),OPP_HAS_BALL_DIST* 0.25);
	CGeoSegmentCircleIntersection intersectionPoint = CGeoSegmentCircleIntersection(oppShootLine,oppCircle);
	CGeoPoint moviongPoint = intersectionPoint.point1();
	CVector me2Ball = ball.Pos() - me.Pos();
	setSubTask(PlayerRole::makeItSimpleGoto(vecNumber,moviongPoint,me2Ball.dir(),CVector(0,0),0,0));
}
//条件判断函数
bool CAdvanceBallV1::ballMoving2Opp(const CVisionModule* pVision,const int vecNumber)
{
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const MobileVisionT& ball = pVision->Ball();
	CVector self2ball = ball.Pos() - me.Pos();
	double diff_ballMoving2me = Utils::Normalize(ball.Vel().dir() - self2ball.dir() - Param::Math::PI);
	if ( opponentID > 0 ){
		const PlayerVisionT& opp = pVision->TheirPlayer(opponentID);
		CVector ball2enemy = opp.Pos() - ball.Pos();
		double diff_ballMoving2Opp = Utils::Normalize(ball2enemy.dir() - ball.Vel().dir());
		if ( abs(diff_ballMoving2Opp) < Param::Math::PI/6 
			&& abs(diff_ballMoving2me) - abs(diff_ballMoving2Opp) > Param::Math::PI/20)
		{
			return  true;
		}
	}
	return false;
}
bool CAdvanceBallV1::isMePassedOpp(const CVisionModule* pVision,const int vecNumber)
{
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const PlayerVisionT& opp = pVision->TheirPlayer(opponentID);
	const MobileVisionT& ball = pVision->Ball();

	CVector me2opp = opp.Pos() - me.Pos();
	CVector opp2Ball = ball.Pos() - opp.Pos();
	CVector ball2Opp = opp.Pos() - ball.Pos();
	CVector me2Ball = ball.Pos() - me.Pos();
	bool meDirControlBall = abs(Utils::Normalize(me2Ball.dir() - me.Dir())) < Param::Math::PI / 4;
	bool meDistControlBall = me2Ball.mod() < Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + 11;
	bool mePassOpp_1 =abs(Utils::Normalize(me2Ball.dir() - me2opp.dir())  )>Param::Math::PI / 2.5;
	bool mePassOpp_2 = abs(Utils::Normalize(opp.Dir() - ball2Opp.dir()))<Param::Math::PI / 3.0;
	if (meDistControlBall && meDistControlBall && (mePassOpp_1||mePassOpp_2)||me2opp.mod()>50)
	{
		//GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0,0),"PASS!!!!!!!!!!!",COLOR_WHITE);
		return true;
	}else
	{
		return false;
	}
}
bool CAdvanceBallV1::isVisionHasBall(const CVisionModule* pVision,const int vecNumber)
{

	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const MobileVisionT& ball =pVision->Ball();
	//double visionJudgDist= Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + Param::Field::BALL_SIZE +2;
	double visionJudgDist= 11;
	bool distVisionHasBall = CVector(me.Pos() - ball.Pos()).mod() <= visionJudgDist;
	bool dirVisionHasBall ;
	double meDir = me.Dir();
	double me2Ball = (ball.Pos() - me.Pos()).dir();
	double meDir_me2Ball_Diff = abs(Utils::Normalize((meDir - me2Ball)));
	if (meDir_me2Ball_Diff < Param::Math::PI / 4.0)
	{
		dirVisionHasBall = true;
	}
	else
		dirVisionHasBall = false;
	bool isVisionPossession = dirVisionHasBall && distVisionHasBall;
	return isVisionPossession;
}
bool CAdvanceBallV1::isShootBlockedInPenaltyArea(const CVisionModule* pVision,const int vecNumber)
{
	//判断一系列射门角度有没有被挡住

	//图像信息
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& opp = pVision->TheirPlayer(opponentID);
	//是否挡住的判断
	bool isBlocked;

	KickDirection::Instance()->GenerateShootDir(vecNumber, pVision->OurPlayer(vecNumber).Pos());
	double kickDir = KickDirection::Instance()->getRealKickDir();

	CGeoLine ball2ourGoal = /*CGeoLine(me.Pos(),ourGoal); */CGeoLine(me.Pos(),kickDir);
	CGeoPoint projectionPoint = ball2ourGoal.projection(opp.Pos());
	double opp2LineDist = (projectionPoint - opp.Pos()).mod();
	//	bool isPassOpp;//false表示没Pass
	//isPassOpp =/*Utils::Normalize( me.Dir() - (opp.Pos() - me.Pos()).dir()) > Param::Math::PI/3; */isMePassedOpp(pVision,vecNumber);

	isBlocked = opp2LineDist < (Param::Vehicle::V2::PLAYER_SIZE / 2.0  +  5 )/*&& !isPassOpp*/;
	//cout <<"opp2LineDist"<<opp2LineDist<<"   dist:"<<opp2LineDist<<" "<<endl;
	if (isBlocked){
		return true;
	}
	else{
		return false;
	}
}
bool CAdvanceBallV1::isShootBlocked(const CVisionModule* pVision,const int vecNumber)
{
	//判断一系列射门角度有没有被挡住

	//图像信息
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& opp = pVision->TheirPlayer(opponentID);
	//是否挡住的判断
	bool isBlocked;

	KickDirection::Instance()->GenerateShootDir(vecNumber, pVision->OurPlayer(vecNumber).Pos());
	double kickDir = KickDirection::Instance()->getRealKickDir();

	CGeoLine ball2ourGoal = /*CGeoLine(me.Pos(),ourGoal); */CGeoLine(me.Pos(),kickDir);
	CGeoPoint projectionPoint = ball2ourGoal.projection(opp.Pos());
	double opp2LineDist = (projectionPoint - opp.Pos()).mod();
	bool isPassOpp;//false表示没Pass
	isPassOpp =/*Utils::Normalize( me.Dir() - (opp.Pos() - me.Pos()).dir()) > Param::Math::PI/3; */isMePassedOpp(pVision,vecNumber);

	isBlocked = opp2LineDist < Param::Vehicle::V2::PLAYER_SIZE / 2.0 && !isPassOpp;
	//cout << "opp2LineDist:"<<opp2LineDist<<"  "<<"isPassOpp"<<isPassOpp<<endl;
	if (isBlocked){
		return true;
	}
	else{
		return false;
	}

}
bool CAdvanceBallV1::checkOppHasBall(const CVisionModule* pVision) 
{
	int _executor = task().executor;
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& me = pVision->OurPlayer(_executor);
	const CVector self2ball = ball.Pos() - me.Pos();
	//此处进行最有威胁的车的计算
	opponentID = 0;

	const CBestPlayer::PlayerList& oppList = BestPlayer::Instance()->theirFastestPlayerToBallList();
	if ( oppList.size() < 1)
		return false;
	else
		opponentID = oppList[0].num;

	if (!pVision->TheirPlayer(opponentID).Valid()) {
		opponentID = getTheirMostClosetoPosPlayerNum(pVision, pVision->Ball().Pos());
	}
	const PlayerVisionT& opponent = pVision->TheirPlayer(opponentID);

	//标识出敌方最有威胁的车
	if (DEBUG_ENGINE)
	{
		drawBestOpp(opponent.Pos());
	}

	CVector opponent2ball = ball.Pos() - opponent.Pos();
	CVector opp2ourGoal = ourGoal - opponent.Pos();
	double opponent2ball_diff = fabs(Utils::Normalize(opponent2ball.dir() - opponent.Dir()));
	double judgeDist = OPP_HAS_BALL_DIST;
	//if ( (opponent2ball.mod() < 40 && self2ball.mod()+15 > opponent2ball.mod()/* && opponent2ball_diff < Param::Math::PI * 90 / 180*/)
	//	|| (opponent2ball.mod() < Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE + 5 /*&& opponent2ball_diff < Param::Math::PI * 100 / 180*/))	
	//GDebugEngine::Instance()->gui_debug_arc(opponent.Pos(),judgeDist,0,360,COLOR_WHITE);
	if(opponent2ball.mod() < judgeDist  && opponent2ball_diff < Param::Math::PI * 70 / 180)
		return true; // 考虑对手的朝向,没有对着球则忽略
	else 
		return false;
}
bool CAdvanceBallV1::checkUsingVoilence(const CVisionModule*pVision,int vecNumber)
{
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const PlayerVisionT& opp = pVision->TheirPlayer(opponentID);
	CVector opp2ball = ball.Pos() - opp.Pos();

	if (ball.X() < 0 && !WEAK_OPP && isOppFaceOurDoor(pVision)&&checkBallFront(pVision) && !ballBetweentMeAndOpp(pVision,vecNumber))
		return true;
	else return false;
}
bool CAdvanceBallV1::oppBlockMe(const CVisionModule* pVision,const int vecNumber)
{
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const PlayerVisionT& opp = pVision->TheirPlayer(opponentID);
	CGeoLine shootLine = CGeoLine(me.Pos(),CVector(ball.Pos() - me.Pos()).dir());
	CGeoPoint projecPoint = shootLine.projection(opp.Pos());
	CVector me2Goal = theirCenter - me.Pos();
	CVector me2Ball  = ball.Pos() - me.Pos();
	CVector ball2opp = opp.Pos() - ball.Pos();
	CVector me2Opp = opp.Pos() - me.Pos();
	bool isBallNearOpp = ball2opp.mod() < 15;
	bool isMeFaceTheirGoal = abs(Utils::Normalize(me.Dir() - me2Goal.dir())) < Param::Math::PI / 6.0;
	bool oppFrontMe = abs(Utils::Normalize(me2Ball.dir() - me2Opp.dir())) < Param::Math::PI / 9.0;
	if (projecPoint.dist(opp.Pos()) < Param::Vehicle::V2::PLAYER_SIZE&&(ball.Pos().x()> me.Pos().x() && ball.Pos().x() < opp.Pos().x()))
	{
		return true && isMeFaceTheirGoal && isBallNearOpp&&oppFrontMe ;
	}
	else
	{
		return false;
	}
}
bool CAdvanceBallV1::selectChipKick(const CVisionModule* pVision,int vecNumber)
{
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& opp = pVision->TheirPlayer(opponentID);
	if (!opp.Valid())
	{
		return false;
	}
	KickDirection::Instance()->GenerateShootDir(vecNumber, pVision->OurPlayer(vecNumber).Pos());
	double kickDir = KickDirection::Instance()->getRealKickDir();
	CVector ball2opp  =  opp.Pos() - ball.Pos();
	CVector me2ball = ball.Pos() - me.Pos();
	CGeoLine ball2Goal = CGeoLine(ball.Pos(), kickDir);
	CGeoPoint projPoint = ball2Goal.projection(opp.Pos());
	double blockDist = opp.Pos().dist(projPoint);
	if(ball2opp.mod() > 26 && ball2opp.mod() < 50 && abs(Utils::Normalize(me2ball.dir() -ball2opp.dir() )) < Param::Math::PI / 4.0 && blockDist < Param::Field::MAX_PLAYER_SIZE * 0.5 )
	{
		return true;
	}
	return false;
}
//计算函数
void CAdvanceBallV1::calcPushTarget(const CVisionModule* pVision)
{
	int _executor = task().executor;
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& me = pVision->OurPlayer(_executor);
	const CVector self2ball = ball.Pos() - me.Pos();

	// 优先考虑到球门方向是否拥堵
	CVector ball2theirGoal = theirCenter - ball.Pos();
	bool is_shootline_blocked = false;
	int opp_best_index = 0;
	double oppDist2Ball = 1000;
	int left_opp_num = 0;
	int right_opp_num = 0;
	for (int i=1; i<=Param::Field::MAX_PLAYER; i++){
		if ( !pVision->TheirPlayer(i).Valid() )
			continue;

		const PlayerVisionT& opp = pVision->TheirPlayer(i);
		if (opp.Pos().dist(theirCenter) < 80 )
			continue;

		CVector ball2opp = opp.Pos() - ball.Pos();
		double diff_block = Utils::Normalize(ball2opp.dir() - ball2theirGoal.dir());

		if ( abs(diff_block) < Param::Math::PI/4 && ball2opp.mod() < 50 )
			is_shootline_blocked = true;

		if ( opp.Y() > 0 )
			right_opp_num++;
		else
			left_opp_num++;

		if ( ball2opp.mod() < oppDist2Ball ){
			opp_best_index = i;
			oppDist2Ball = ball2opp.mod();
		}
	}

	// 我已经可以安全控住球时才考虑变换pushTarget
	bool safe_change_dir = false;
	if ( opp_best_index > 0 ){
		const PlayerVisionT& bestOpp = pVision->TheirPlayer(opp_best_index);
		CVector bestOpp2ball = ball.Pos() - bestOpp.Pos();
		double usBlockAngle = Utils::Normalize(bestOpp2ball.dir() - self2ball.dir());
		if (RobotSensor::Instance()->IsInfraredOn(_executor) &&
			abs(usBlockAngle) < Param::Math::PI/2.0 )
			safe_change_dir = true;
	}

	if ( !safe_change_dir ){
		_pushBallTarget = theirCenter;
		return;
	}

	if ( !is_shootline_blocked )
		attack_side = ATTACK_TO_CENTER;
	else if ( attack_side == ATTACK_TO_CENTER ){
		attack_side = left_opp_num > right_opp_num ? ATTACK_TO_RIGHT : ATTACK_TO_LEFT;
	}
	else if ( attack_side == ATTACK_TO_LEFT ){
		attack_side = left_opp_num > right_opp_num + 1 ? ATTACK_TO_RIGHT : ATTACK_TO_LEFT;
	}
	else{
		attack_side = right_opp_num > left_opp_num + 1 ? ATTACK_TO_LEFT : ATTACK_TO_RIGHT; 
	}

	// 查看自己的最近的队友在哪边
	double minDist = 100;
	int friendID = 0;
	int friendSide = 0; // 1表示右边,-1表示左边
	for (int i=1; i<=Param::Field::MAX_PLAYER; i++){
		if ( i == _executor )
			continue;
		if ( !pVision->OurPlayer(i).Valid() )
			continue;
		const PlayerVisionT& myfriend = pVision->OurPlayer(i);
		if ( myfriend.Pos().dist(ourGoal) < 80 )
			continue;

		double friendDist = myfriend.Pos().dist(me.Pos());
		if ( friendDist < minDist ){
			friendID = i;
			minDist = friendDist;
			CVector ball2friend = myfriend.Pos() - ball.Pos();
			double diff2friend = Utils::Normalize(ball2friend.dir() - self2ball.dir());
			//cout<<"No."<<i<<" diff friend:"<<Utils::Rad2Deg(diff2friend)<<endl;
			if ( abs(diff2friend) > 5*Param::Math::PI/12 )
				continue;

			// 这里假设我拿球的朝向是向着对方半场的
			friendSide = diff2friend > 0 ? 1 : -1;
		}
	}

	double push_dir;
	if ( friendSide != 0 ){
		const PlayerVisionT& selected_friend = pVision->OurPlayer(friendID);
		CGeoPoint friend_front = selected_friend.Pos() + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER, selected_friend.Dir());
		CVector ball2friend = friend_front - ball.Pos();
		double temp_diff = Utils::Normalize(ball2theirGoal.dir() - ball2friend.dir());
		push_dir = Utils::Normalize(ball2friend.dir() + 0.2*temp_diff);
		if ( friendSide > 0 )
			push_dir = min(Param::Math::PI/2, push_dir);
		else if ( friendSide < 0 )
			push_dir = max(-Param::Math::PI/2, push_dir);
	}

	switch (attack_side) 
	{
	case ATTACK_TO_CENTER: // 没人档，直接往中间攻
		_pushBallTarget = theirCenter;
		break;
	case ATTACK_TO_LEFT:
		if ( friendSide < 0 || friendSide > 0 && ball.Y() < Param::Field::PITCH_WIDTH/2 - 80 )
			_pushBallTarget = ball.Pos() + Utils::Polar2Vector(60, push_dir);
		else{
			double pd = Utils::Normalize(ball2theirGoal.dir()-Param::Math::PI/3);
			pd = max(-Param::Math::PI/2, pd);
			_pushBallTarget = ball.Pos() + Utils::Polar2Vector(60, pd);
			//cout<<"attack left\n";
		}
		break;
	case ATTACK_TO_RIGHT:
		if ( friendSide > 0 || friendSide < 0 && ball.Y() > -Param::Field::PITCH_WIDTH/2 + 80)
			_pushBallTarget = ball.Pos() + Utils::Polar2Vector(60, push_dir);
		else{
			double pd = Utils::Normalize(ball2theirGoal.dir()+Param::Math::PI/3);
			pd = min(Param::Math::PI/2, pd);
			_pushBallTarget = ball.Pos() + Utils::Polar2Vector(60, pd);
			//cout<<"attack right\n";
		}
		break;
	}
}



bool CAdvanceBallV1::checkEnoughSpaceToTurnKick(const CVisionModule* pVision)
{
	const MobileVisionT& ball = pVision->Ball();
	CGeoPoint goalCenter = CGeoPoint(Param::Field::PITCH_LENGTH / 2.0 ,0);
	double ball2GoalCenterDist = (ball.Pos() - goalCenter).mod();
	if (ball2GoalCenterDist >120 && ball2GoalCenterDist < 230)
	{
		return true;
	}
	else
		return false;
}
void CAdvanceBallV1::decideChipOrShoot(const CVisionModule* pVision,double& kickDir,const int vecNumber)
{
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& opp = pVision->TheirPlayer(opponentID);

	bool shootBlocked; //true被挡，挑射；false，没被挡，平射

	shootBlocked = isShootBlocked(pVision,vecNumber);
	if (shootBlocked)
	{
		if (ballOnTheirHalf(pVision))
		{
			if( (me.Pos() - opp.Pos()).mod() > 25 && !checkEnoughSpaceToTurnKick(pVision))
			{
				KickStatus::Instance()->setChipKick(vecNumber,200);
			}else
			{
				KickStatus::Instance()->setKick(vecNumber,kickPower);
			}
		}else{
			//球在我们半场，直到顶到有空档为止
		}
	}else
	{
		KickStatus::Instance()->setKick(vecNumber,kickPower);	
	}
}
bool CAdvanceBallV1::decidePushOrPull(const CVisionModule* pVision,int& flag)
{
	flag = flag | PlayerStatus::DRIBBLING;
	if (ballOnTheirHalf(pVision))
	{
		//后拉
		return true;
	} 
	else
	{
		//前顶
		return false;
	}
}


void CAdvanceBallV1::drawBestOpp(const CGeoPoint& oppPos)
{
	GDebugEngine::Instance()->gui_debug_msg(oppPos, "Best Opp!!!!!!!!!!", COLOR_WHITE);
}
void CAdvanceBallV1::toStream(std::ostream& os)const
{

}

CPlayerCommand* CAdvanceBallV1::execute(const CVisionModule* pVision)
{
	if( subTask() ){
		return subTask()->execute(pVision);
	}
	if( _directCommand ){
		return _directCommand;
	}
	return 0;
}

bool CAdvanceBallV1::faceTheirGoal(const CVisionModule* pVision, const int executor) {
	double dist = Param::Field::PITCH_LENGTH / 2 - pVision->OurPlayer(executor).Pos().x();
	double buffer = 0.05 * dist + 20;
	const CGeoPoint theirLeft = CGeoPoint(Param::Field::PITCH_LENGTH / 2, -Param::Field::GOAL_WIDTH / 2 - buffer);
	const CGeoPoint theirRight = CGeoPoint(Param::Field::PITCH_LENGTH / 2, Param::Field::GOAL_WIDTH / 2 + buffer);
	double leftAngle = Utils::Normalize((theirLeft - pVision->OurPlayer(executor).Pos()).dir());
	double rightAngle = Utils::Normalize((theirRight - pVision->OurPlayer(executor).Pos()).dir());
	double myDir = pVision->OurPlayer(executor).Dir();

	double rawLeftAngle = Utils::Normalize((theirLeft - pVision->OurPlayer(executor).RawPos()).dir());
	double rawRifhtAngle = Utils::Normalize((theirLeft - pVision->OurPlayer(executor).RawPos()).dir());
	double myRawDir = pVision->OurRawPlayer(executor).dir;

	bool isToTheirGoal = ((myDir > leftAngle) && (myDir < rightAngle));
	bool isRawToTheirGoal = ((myRawDir > rawLeftAngle) && (myRawDir < rawRifhtAngle));

	if (false || DEBUG_ENGINE) {
	GDebugEngine::Instance()->gui_debug_line(theirLeft, pVision->OurPlayer(executor).Pos());
	GDebugEngine::Instance()->gui_debug_line(theirRight, pVision->OurPlayer(executor).Pos());
	GDebugEngine::Instance()->gui_debug_line(pVision->OurPlayer(executor).Pos() + Utils::Polar2Vector(100, myDir), pVision->OurPlayer(executor).Pos());

	GDebugEngine::Instance()->gui_debug_line(theirLeft, pVision->OurPlayer(executor).RawPos());
	GDebugEngine::Instance()->gui_debug_line(theirRight, pVision->OurPlayer(executor).RawPos());
	GDebugEngine::Instance()->gui_debug_line(pVision->OurPlayer(executor).RawPos() + Utils::Polar2Vector(100, myRawDir), pVision->OurPlayer(executor).RawPos());
	}
	return (isToTheirGoal || isRawToTheirGoal);
}

int CAdvanceBallV1::getTheirMostClosetoPosPlayerNum(const CVisionModule* pVision, CGeoPoint pos) {
	double dist = 1000;
	int num = 0;
	for (int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
		if (pVision->TheirPlayer(i).Valid()) {
			if (pVision->TheirPlayer(i).Pos().dist(pos)<dist) {
				dist = pVision->TheirPlayer(i).Pos().dist(pos);
				num = i;
			}
		}
	}
	return num;
}