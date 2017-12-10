#include "InterceptBall.h"
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

#define INTER_TEST_MODE true
#define INTER_DEBUG if(INTER_TEST_MODE==true)

namespace
{
	const double pi = 3.141592653;
	const double degree = pi / 180.0;
	const double INTER_ROBOT_FRONT2CENTER = 7.5;
	const double INTER_ESTIMATED_CAR_SPEED = 150;
	const double INTER_AIMING_THRESH = 20;
	const double INTER_CHTCH_THRESH = 30;
	const double INTER_CHASE_SPEED_COEFF = 2;
	const double INTER_CATCH_FIX_ANG = 45 * degree;
	const double INTER_CHASE_MAXDIST = 150;
	enum INTER_STATE
	{
		AVOID = 0,
		PREPARE = 1,
		CHASE = 2,
		AIM = 3,
		CATCH = 4
	};
}


void CInterceptBall::plan(const CVisionModule* pVision)
{
	int iRobotNum = task().executor;
	const PlayerVisionT& objPlayer = pVision->OurPlayer(iRobotNum);
	const MobileVisionT& objBall = pVision->Ball();

	//Geom
	//Ball Infomation	
	const CGeoPoint pBall = objBall.Pos();
	const CVector vBallVel = objBall.Vel();
	const double dBallVel = vBallVel.mod();
	const double angBallVel = dBallVel > 5 ?  vBallVel.dir() : pi;
	const bool bBallDynamic = dBallVel > 5 ? true : false;

	//Player Infomation
	const CGeoPoint pPlayer = objPlayer.Pos();
	const double angPlayerFacing = objPlayer.Dir();

	const CVector vPlayerVel = objPlayer.Vel(); //pVision->OurRawPlayerSpeed(robotNum);
	const double dPlayerVel = vPlayerVel.mod();
	const double angPlayerVel = vPlayerVel.dir();

	const CVector vPlayer2Ball = pBall - pPlayer;
	const double dPlayer2Ball = vPlayer2Ball.mod();
	const double angPlayer2Ball = vPlayer2Ball.dir();


	const CGeoLine lBallTraj = CGeoLine(pBall, angBallVel);
	const CGeoPoint pProj = lBallTraj.projection(pPlayer);

	const CVector Vb = pProj - pBall;
	const CVector Vp = pProj - pPlayer;
	
	double dx;
	double dy;
	if (bBallDynamic)
	{
		dx = Vb.mod()*Sign(vBallVel*Vb);
		dy = Vp.mod()*Sign(Vp*CVector(1, 0));
		//cout << Vp << "*" << CVector(1, 0) << "=" << Vp*CVector(0, 1) << endl;
	}
	else
	{
		dx = pBall.y() - pPlayer.y();
		dy = pBall.x() - pPlayer.x();
	}


	//Decision
	static INTER_STATE inter_state;
	double dArriveTime;
	dArriveTime = abs(dy) / INTER_ESTIMATED_CAR_SPEED;
	bool bIsTimeEnough;
	CGeoPoint pBallPredict = pBall + vBallVel*dArriveTime;
	CVector Vbp = pProj - pBallPredict;
	if (Vbp*vBallVel > 0) bIsTimeEnough = true;
	else bIsTimeEnough = false;

	INTER_DEBUG GDebugEngine::Instance()->gui_debug_x(pProj, COLOR_RED);
	INTER_DEBUG GDebugEngine::Instance()->gui_debug_line(pPlayer, pProj, COLOR_RED);
	INTER_DEBUG GDebugEngine::Instance()->gui_debug_x(pBallPredict, COLOR_BLUE);
	//cout << "## Dx:" << dx << "- Dy:" << dy << endl;
	
	if (abs(dy) < INTER_AIMING_THRESH)
	{
		if (pPlayer.dist(pBall) < INTER_CHTCH_THRESH)
		{
			INTER_DEBUG cout << "CATCH" << endl;
			inter_state = CATCH;
		}
		else
		{
			INTER_DEBUG cout << "AIM" << endl;
			inter_state = AIM;
		}
	}
	else if (dy<0)
	{
		INTER_DEBUG cout << "AVOID" << endl;
		inter_state = AVOID;
	}
	else if (bIsTimeEnough)
	{
		INTER_DEBUG cout << "PREPARE" << endl;
		inter_state = PREPARE;
	}
	else if (!bIsTimeEnough)
	{
		INTER_DEBUG cout << "CHASE" << endl;
		inter_state = CHASE;
	}
	else
	{
		INTER_DEBUG cout << "AVOID" << endl;
		inter_state = AVOID;
	}
	cout << inter_state << endl;

	//Motion
	switch (inter_state)
	{
	case AVOID:
				{
				  setSubTask(PlayerRole::makeItStop(iRobotNum));
				  break;
				}
	case PREPARE:
				{
					CGeoPoint pTask;
					pTask = pProj.midPoint(pBallPredict);
					setSubTask(PlayerRole::makeItGoto(iRobotNum, pTask, angPlayer2Ball, PlayerStatus::QUICKLY));
					break;
				}
	case CHASE:
				{
				  static double chase_dist = 0;
				  static CGeoPoint pHist = pPlayer;
				  static int lastCycle = pVision->Cycle();

				  
				  if (abs(pVision->Cycle() - lastCycle) < 3)
				  {
					  chase_dist += pPlayer.dist(pHist);
				  }
				  else
				  {
					  chase_dist = 0;
					  pHist = pPlayer;
				  }

				  CGeoPoint pTask = pBall + vBallVel*dArriveTime*INTER_CHASE_SPEED_COEFF;
				  if (chase_dist>INTER_CHASE_MAXDIST)
				  {
					  setSubTask(PlayerRole::makeItStop(iRobotNum));
				  }
				  else
				  {
					  setSubTask(PlayerRole::makeItGoto(iRobotNum, pTask, angPlayer2Ball, PlayerStatus::QUICKLY));
				  }
				 
				  pHist = pPlayer;
				  lastCycle = pVision->Cycle();
				  break;
				}
	case AIM:
				{
				setSubTask(PlayerRole::makeItGoto(iRobotNum, pProj, angPlayer2Ball, PlayerStatus::SLOWLY));
				break;
				}
	case CATCH:
				{
				  DribbleStatus::Instance()->setDribbleCommand(iRobotNum, 2);
				 
				  double angCatch;
				  CGeoPoint pCatch;
				  if (angPlayer2Ball > INTER_CATCH_FIX_ANG) angCatch = angPlayer2Ball - INTER_CATCH_FIX_ANG;
				  else if (angPlayer2Ball < -1 * INTER_CATCH_FIX_ANG) angCatch = angPlayer2Ball + INTER_CATCH_FIX_ANG;
				  CVector fix = Polar2Vector(INTER_ROBOT_FRONT2CENTER, Normalize(angCatch + pi));
				  pCatch = pProj + fix;
				  

				  if (RobotSensor::Instance()->IsInfraredOn(iRobotNum) || dBallVel<20)
				  {
					  setSubTask(PlayerRole::makeItStop(iRobotNum));
				  }
				  else
				  {
					  setSubTask(PlayerRole::makeItGoto(iRobotNum, pCatch, angCatch, PlayerStatus::SLOWLY));
				  }
				  break;
				}
	default:
		break;
	}
	

	CStatedTask::plan(pVision);
}

CPlayerCommand* CInterceptBall::execute(const CVisionModule* pVision)
{
	if (subTask()){
		return subTask()->execute(pVision);
	}
	return NULL;
}
