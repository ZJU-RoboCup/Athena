#include "Goalie2015V2.h"
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

using namespace Utils;

#define GOALIE_TEST_MODE false



namespace
{
	const double dangerFactor = 2.5;
	const double sec = 60.0;
	const double PENALTY_BUFFER = 16.0;
	const double DEFEND_PENALTY_BUFFER = 25.0;

	//避门柱参数
	const int FIELD_WIDTH = 600;
	const int FIELD_LEN = 900;
	const int GATE_WIDTH = 100;
	const int BALL_SPEED_THRESH = 100;

	const double DANGER_SPEED_THRESH = 400;
	const double KICKBACK_REF_DIST = 300;
	const double KICKBACK_AVOID = 15;

	const CGeoPoint pGateCenter = CGeoPoint(-FIELD_LEN / 2, 0);
	const CGeoPoint pGateLetf = CGeoPoint(-FIELD_LEN / 2, -GATE_WIDTH / 2);
	const CGeoPoint pGateRight = CGeoPoint(-FIELD_LEN / 2, GATE_WIDTH / 2);
	const CGeoPoint pFieldCenter = CGeoPoint(0, 0);
	const CGeoPoint pFrontLeft = CGeoPoint(-FIELD_LEN / 2 + 50, -30);
	const CGeoPoint pFrontRight = CGeoPoint(-FIELD_LEN / 2 + 50, 30);

	const double pi = Param::Math::PI;
	const double degree = Param::Math::PI / 180;

	enum GOALIE_STATE
	{
		NORMAL = 1,
		CLEAR_BALL = 2,
		ATTACK_ENEMY = 3
	};

}

CGoalie2015V2::CGoalie2015V2()
{
	state = NORMAL;

}

void CGoalie2015V2::plan(const CVisionModule* pVision)
{
	int iRobotNum = task().executor;
	const PlayerVisionT& objPlayer = pVision->OurPlayer(iRobotNum);
	const MobileVisionT& objBall = pVision->Ball();

	//图像视觉
	//Ball Infomation	
	const CGeoPoint pBall = objBall.Pos();
	const CVector vBallVel = objBall.Vel();
	const double dBallVel = vBallVel.mod();
	const double angBallVel = vBallVel.dir();
	bool bFastBall = (dBallVel > BALL_SPEED_THRESH) ? true : false;
	int iPreTime = (int)dBallVel;
	CGeoPoint pBallPredict = BallSpeedModel::Instance()->posForTime(dBallVel, pVision);

	//Enemy Infomation
	const PlayerVisionT objEnemy = pVision->TheirPlayer(BestPlayer::Instance()->getTheirBestPlayer());
	CGeoPoint pEnemy = objEnemy.Pos();

	//Player Infomation
	const CGeoPoint pPlayer = objPlayer.Pos();
	const double angPlayerFacing = objPlayer.Dir();

	const CVector vPlayerVel = objPlayer.Vel(); //pVision->OurRawPlayerSpeed(robotNum);
	const double dPlayerVel = vPlayerVel.mod();
	const double angPlayerVel = vPlayerVel.dir();

	const CVector vPlayer2BallPredict = pBallPredict - pPlayer;
	const double dPlayer2BallPredict = vPlayer2BallPredict.mod();

	const CVector vPlayer2Ball = pBall - pPlayer;
	const double dPlayer2Ball = vPlayer2Ball.mod();
	const double angPlayer2Ball = vPlayer2Ball.dir();

	const double angPlayerBall2BallVel = fabs(Normalize(vPlayer2Ball.dir() - angBallVel));	//球车向量与球速线夹角
	const double angPlayerBall2MeDir = fabs(Normalize(vPlayer2Ball.dir() - angPlayerFacing));		//球车向量与小车朝向夹角

	const CVector vPlayer2FCenter = pFieldCenter - pPlayer;
	const double andPlayer2FCenter = vPlayer2FCenter.dir();

	//定义pTask与angTask
	CGeoPoint pTask = DefPos2015::Instance()->getDefPos2015(pVision).getGoaliePos();
	double angTask;
	static double angLastValid;
	static CGeoPoint pLastValid;

	if (objBall.Valid())
	{
		angTask = angPlayer2Ball;
		angLastValid = angTask;
		pLastValid = pBall;
	}
	else
	{
		angTask = angLastValid;
	}

	TaskT taskGoalie(task());
	taskGoalie.player.flag |= PlayerStatus::QUICKLY;
	taskGoalie.player.pos = pTask;
	taskGoalie.player.angle = angTask;


	////////////////////////////////////////////////////////////////
	//状态切换与taskGoalie设置
	////////////////////////////////////////////////////////////////
	//cycle recoder
	static int cycle = pVision->Cycle();
	static int cycleNormal = -100;
	static int cycleInDanger = -100;
	static int cycleStopBall = -100;
	static int cycleClearBall = -100;
	static int cycleBallInvalid = -100;
	static int cycleAttackEnemy = -100;
	static int cycleKicked = -100;
	cycle = pVision->Cycle();
	if (world->IsBallKicked(iRobotNum)) cycleKicked = cycle;

	//bools
	static bool isAttackSuccess = false;
	static bool isClearSuccess = false;


	if (WorldModel::Instance()->CurrentRefereeMsg() == "gameStop")
	{ //Over All #1: "gameStop"
		state = NORMAL;
	}
	else if (!InOurPenaltyArea(pPlayer, PENALTY_BUFFER * 2))
	{ //Over ALL #2 : "Goalie leaves the penalty"
		state = NORMAL;
	}

	else if (cycle - cycleKicked < 0.35 * sec)
	{//Over All #3: "JustKicked"
		state = NORMAL;
	}

	else
		switch (state)
	{
		case NORMAL:
			if (needtoClearBall(pVision) && WorldModel::Instance()->CurrentRefereeMsg() != "gameStop") //清球 优先于 进攻
			{
				state = CLEAR_BALL;
			}
			else if (needtoAttackEnemy(pVision) && (cycle - cycleAttackEnemy)>2 * sec)
			{
				state = ATTACK_ENEMY;
			}
			break;

		case CLEAR_BALL:
			if (!needtoClearBall(pVision))
			{
				state = NORMAL;
			}
			break;

		case ATTACK_ENEMY:
			//to clear ball
			if ((dBallVel < 80 && InOurPenaltyArea(pBall, -30))
				|| (dBallVel < 30 && InOurPenaltyArea(pBall, PENALTY_BUFFER))
				&& pBall.dist(pEnemy)>30)
			{
				state = CLEAR_BALL;
			}

			else if ((!InOurPenaltyArea(pBall, PENALTY_BUFFER) && !DefendUtils::BallIsToPenaltyArea()) ||
				(!(Utils::InOurPenaltyArea(pEnemy, PENALTY_BUFFER) && (pBall.dist(pEnemy)>30) ||
				  fabs((pEnemy - pGateCenter).dir() - angBallVel) > Param::Math::PI * 2 / 3)))
			{
				state = NORMAL;
			}

			else if (isAttackSuccess == true)
			{
				state = NORMAL;
				isAttackSuccess = false;
			}
			break;

		default:
			state = NORMAL;
			break;
	}

	//状态执行
	switch (state)
	{
	case NORMAL:{//--------------- STATE NORMAL ---------------
					if (GOALIE_TEST_MODE) cout << "in normal goalie" << endl;
					setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(taskGoalie));
					break;
	}

	case CLEAR_BALL:{//--------------- STATE CLEAR BALL ---------------
						if (GOALIE_TEST_MODE) cout << "in clear ball" << endl;
						double angClearBallDir = CVector(pBall - pGateCenter).dir();
						double angBall2Gate = CVector(pGateCenter - pBall).dir();
						//双后卫
						if (TaskMediator::Instance()->singleBack() == 0 && TaskMediator::Instance()->leftBack() != 0)
						{
							CGeoPoint leftpos = DefPos2015::Instance()->getDefPos2015(pVision).getLeftPos();
							CGeoPoint rightpos = DefPos2015::Instance()->getDefPos2015(pVision).getRightPos();
							double angBall2LeftPos = (DefPos2015::Instance()->getDefPos2015(pVision).getLeftPos() - pBall).dir() - 0.3;
							double angBall2RightPos = (DefPos2015::Instance()->getDefPos2015(pVision).getRightPos() - pBall).dir() + 0.3;

							if (AngleBetween(angBall2Gate, angBall2LeftPos, angBall2RightPos))
							{
								if (angBall2RightPos >= 0 && angBall2LeftPos >= 0){
									angClearBallDir = Normalize(angBall2LeftPos - Param::Math::PI / 6);
								}
								else if (angBall2LeftPos <= 0 && angBall2RightPos <= 0){
									angClearBallDir = Normalize(angBall2RightPos + Param::Math::PI / 6);
								}
								else{
									double toLeft = Normalize(angClearBallDir - angBall2LeftPos);
									double toRight = Normalize(angBall2RightPos - angClearBallDir);
									if (fabs(toLeft) > fabs(toRight))
									{
										angClearBallDir = Utils::Normalize(angBall2RightPos + Param::Math::PI / 6);
									}
									else
									{
										angClearBallDir = Utils::Normalize(angBall2LeftPos - Param::Math::PI / 6);
									}
								}
							}
						}
						//单后卫
						else if (TaskMediator::Instance()->singleBack() != 0)
						{
							double angBall2Single = (DefPos2015::Instance()->getDefPos2015(pVision).getSinglePos() - pBall).dir();
							if (std::abs(angBall2Single - angClearBallDir)<Param::Math::PI / 18)
							{
								if (std::abs(angClearBallDir)>Param::Math::PI / 10)
								{
									angClearBallDir = -angClearBallDir;
									if (std::abs(angClearBallDir) > Param::Math::PI * 70 / 180)
									{
										if (angClearBallDir < 0)
											angClearBallDir = angClearBallDir + Param::Math::PI / 9;
										else
											angClearBallDir = angClearBallDir - Param::Math::PI / 9;
									}
								}
								else
								{
									angClearBallDir = angClearBallDir + Param::Math::PI * 7 / 18;
								}
							}
						}
						//T_T没有后卫
						else
						{
							angClearBallDir = CVector(pBall - pGateCenter).dir();
						}

						if (pEnemy.dist(pBall) < 100)
						{
							angClearBallDir = CVector(pBall - pGateCenter).dir();
						}

						//限制极限位置
						if (angClearBallDir >= Param::Math::PI * 80 / 180.0)
						{
							angClearBallDir = Param::Math::PI * 80 / 180.0;
						}
						else if (angClearBallDir <= -Param::Math::PI * 80 / 180.0)
						{
							angClearBallDir = -Param::Math::PI * 80 / 180.0;
						}

						//执行方式选择
						if (pBall.dist(pGateLetf) < Param::Vehicle::V2::PLAYER_SIZE * dangerFactor && pBall.y() >= pGateLetf.y()
							&& CVector(pBall - pPlayer).dir() > 0)
						{
							setSubTask(PlayerRole::makeItGoto(iRobotNum, pFrontLeft, CVector(pBall - pPlayer).dir(), CVector(0, 0), 0, 0));
						}
						else if (pBall.dist(pGateRight) < Param::Vehicle::V2::PLAYER_SIZE * dangerFactor && pBall.y() <= pGateRight.y()
							&& CVector(pBall - pPlayer).dir() < 0)
						{
							setSubTask(PlayerRole::makeItGoto(iRobotNum, pFrontRight, CVector(pBall - pPlayer).dir(), CVector(0, 0), 0, 0));
						}
						else
						{
							DribbleStatus::Instance()->setDribbleCommand(iRobotNum, 3);
							setSubTask(PlayerRole::makeItNoneTrajGetBall(iRobotNum, angClearBallDir, CVector(0, 0), PlayerStatus::QUICKLY | PlayerStatus::DRIBBLING));
						}
						break;
	}

	case ATTACK_ENEMY:{//------------------------- ATTACK_ENEMY ----------------------------
						  //if (GOALIE_TEST_MODE) cout << "# state : ATTACK!!! " << endl;
						  static int iTimer = 0;
						  if ((cycle - cycleAttackEnemy)>2)
						  {
							  iTimer = 0;
						  }

						  taskGoalie.player.pos = pEnemy;
						  taskGoalie.player.is_specify_ctrl_method = true;
						  taskGoalie.player.specified_ctrl_method = NONE_TRAP;
						  taskGoalie.player.max_acceleration = 1200;
						  setSubTask(TaskFactoryV2::Instance()->GotoPosition(taskGoalie));
						  
						  if ((pEnemy - pPlayer).mod() < Param::Field::MAX_PLAYER_SIZE*1.5)
							  iTimer++;
						  if (iTimer >= 20)
							  isAttackSuccess = true;

						  // cout << iTimer << endl;
						  cycleAttackEnemy = cycle;
						  break;
	}

	default:{//--------------- STATE DEFAULT ---------------
				cout << "no state ? probably something wrong :（" << endl;
				break;
	}
	}
	CStatedTask::plan(pVision);
}


CPlayerCommand* CGoalie2015V2::execute(const CVisionModule* pVision)
{
	if (subTask()){
		return subTask()->execute(pVision);
	}
	return NULL;
}

//additional

bool CGoalie2015V2::needtoClearBall(const CVisionModule* pVision)
{
	bool result = false;
	const MobileVisionT& ball = pVision->Ball();
	int advancer = BestPlayer::Instance()->getOurBestPlayer();
	const PlayerVisionT enemy = pVision->TheirPlayer(BestPlayer::Instance()->getTheirBestPlayer());
	if (((ball.Vel().mod() < 150 && Utils::InOurPenaltyArea(ball.Pos(), -30)) ||
		(ball.Vel().mod() < 50 && Utils::InOurPenaltyArea(ball.Pos(), PENALTY_BUFFER)) && ball.Valid()
		&& !checkWeHaveHelper(pVision) && !Utils::InOurPenaltyArea(enemy.Pos(), DEFEND_PENALTY_BUFFER)) || // '1
		((ball.Vel().mod() < 120 && Utils::InOurPenaltyArea(ball.Pos(), -30)) ||
		(ball.Vel().mod() < 40 && Utils::InOurPenaltyArea(ball.Pos(), PENALTY_BUFFER)) && ball.Valid()
		&& !checkWeHaveHelper(pVision) && Utils::InOurPenaltyArea(enemy.Pos(), DEFEND_PENALTY_BUFFER))) // '2
	{
		result = true;
		GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(50, 0), "need to clear ball");
	}
	return result;
}

bool CGoalie2015V2::needtoAttackEnemy(const CVisionModule* pVision)
{
	bool result = false;
	const PlayerVisionT enemy = pVision->TheirPlayer(BestPlayer::Instance()->getTheirBestPlayer());
	const PlayerVisionT& me = pVision->OurPlayer(task().executor);
	const MobileVisionT& ball = pVision->Ball();
	if ((Utils::InOurPenaltyArea(enemy.Pos(), DEFEND_PENALTY_BUFFER) && ball.Vel().mod()>120 && DefendUtils::BallIsToPenaltyArea()
		&& !DefendUtils::isBallShotToTheGoal())
		|| (!ball.Valid() && Utils::InOurPenaltyArea(enemy.Pos(), PENALTY_BUFFER)))
	{
		result = true;
		GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0, 0), "need to attack enemy");
	}
	return result;
}

bool CGoalie2015V2::checkWeHaveHelper(const CVisionModule* pVision)
{
	bool result = false;
	int helper[Param::Field::MAX_PLAYER + 1] = { 0 };
	int helpernum = 0;
	int bestnum = BestPlayer::Instance()->getOurBestPlayer();
	const MobileVisionT& ball = pVision->Ball();
	for (int i = 1; i <= Param::Field::MAX_PLAYER; i++){
		if (pVision->OurPlayer(i).Pos().dist(ball.Pos())<30){
			helpernum++;
			helper[helpernum] = i;
		}
	}
	if (Utils::InOurPenaltyArea(ball.Pos(), PENALTY_BUFFER) && !Utils::InOurPenaltyArea(ball.Pos(), 0)){
		if (pVision->TheirPlayer(bestnum).X()<ball.X() &&
			abs((pGateCenter - ball.Pos()).dir() - (pGateCenter - pVision->TheirPlayer(bestnum).Pos()).dir())<Param::Math::PI / 4
			&& helpernum >= 3){
			result = true;
		}
	}
	return result;
}