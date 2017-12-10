/////////////////////////////////////////////////////////////////////////
// Editor : YuN
// Date   : 2015/05/25
// Descri : Same as GoAndTurnKick but quicker and has better performance
/////////////////////////////////////////////////////////////////////////
#define KICKTYPE_CHIP 1
#define KICKTYPE_FLAT 0

#include "CircleAndPass.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include "ControlModel.h"
#include "KickStatus.h"
#include <utils.h>
#include <ControlModel.h>
#include <DribbleStatus.h>

using namespace Utils;

namespace
{
	double MAX_TRANSLATION_ACC = 600;		// [unit : cm/s2]
	double MAX_TRANSLATION_DEC = 650;

	//Set State
	enum goAndTurnKickState
	{
		GOTO = 1,
		CIRCLE=2,
		AIM = 3,
		KICK = 4
	};
	bool IS_SIMULATION = false;
	int AimCnt = 0;
	int AimMaxCnt = 3;

	//Set Math Constant
	static double pi = 3.141592653;
	static double degree = pi / 180.0;

	//Set Configs
	static double CHASE_RANGE = 120;			//×·Çò·¶Î§
	static double CHASE_EXTRA_SPEED = 45.0;		//×·Çò¶îÍâËÙ¶È

	static double GOTO_RANGE = 30.0;			//GOTO·¶Î§
	static double GOTO_EXTRA_SPEED = 5.0;		//GOTO¶îÍâËÙ¶È
	static double AVOID_RANGE = 20.0;
	static double ROUND_RANGE_TOLERANCE = 3;	//GOTO·¶Î§ÈÝÏÞ

	static double CIRCLE_R = 10.0+Param::Vehicle::V2::PLAYER_SIZE+Param::Field::BALL_SIZE;			//Ô²»¡°ë¾¶
	static double CIRCLE_PRECISION = 5 * degree;  

	static double AIM_RANGE = 2.0 + Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE;				//AIM·¶Î§
	static double AIM_PRECISION = 3 * degree;	//AIM¾«¶È

	static double KICK_ABANDON_RANGE = 6.0 + Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE;		//KICK·ÅÆú·¶Î§_¾àÀë
	static double KICK_ABANDON_PRECISION = 6 * degree; //KICK·ÅÆú·¶Î§_½Ç¶È
}

CCircleAndPass::CCircleAndPass()
{
	DECLARE_PARAM_READER_BEGIN(General)
	READ_PARAM(IS_SIMULATION)
		DECLARE_PARAM_READER_END
	_lastCycle = 0;
}

void CCircleAndPass::plan(const CVisionModule* pVision)
{
	// ÅÐ¶ÏÊÇ·ñÐèÒª½øÐÐ×´Ì¬ÖØÖÃ
	if (pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1)
	{
		setState(GOTO);
		_lastRotVel = 0;
	}
	_lastCycle = pVision->Cycle();

	////////////////////////////////////////////////////////////////////////////////
	//                        Vision & Geography
	////////////////////////////////////////////////////////////////////////////////
	const int vecNumber = task().executor;
	const int kickType = task().player.flag;
	const int kickPower = task().player.chipkickpower;

	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const double angFinal = task().player.angle;
	const double angFinalR = Utils::Normalize(angFinal + pi);

	const CGeoPoint pBall = ball.Pos();
	const CGeoPoint pPlayer = me.Pos();
	const double angPlayerFacing = me.Dir();

	CVector vPlayer2Ball = pBall - pPlayer;
	CVector vBall2Player = pPlayer - pBall;
	const double dPlayer2Ball = me.Pos().dist(pBall);
	const double angPlayer2Ball = vPlayer2Ball.dir();

	//Further Compute
	const CGeoPoint pFinal = pBall + Polar2Vector(CIRCLE_R, angFinalR);
	const CGeoPoint pChase = pBall + Polar2Vector(CHASE_EXTRA_SPEED, angPlayer2Ball);
	CVector vFinal2Ball = pBall - pFinal;

	bool isSameDir = (vPlayer2Ball*vFinal2Ball >= 0) ? true : false;
	double ang1, ang2;
	double angBall2Final = Normalize(vFinal2Ball.dir() + pi);

	//Ô¶¾àÀëÅÜ 90¡ãÇÐÏß
	ang1 = Normalize(angPlayer2Ball + pi / 2);
	ang2 = Normalize(angPlayer2Ball - pi / 2);
	


	//SetTask
	TaskT myTask(task());
	myTask.player.flag = myTask.player.flag | PlayerStatus::DODGE_OUR_DEFENSE_BOX;


	////////////////////////////////////////////////////////////////////////////////
	//                             Alter States
	////////////////////////////////////////////////////////////////////////////////
	static int lastState = GOTO;

	if (dPlayer2Ball > GOTO_RANGE * 1.15)
	{//#1: Force Alter
		setState(GOTO);
		cout << "#GOTO" << endl;
	}
	else
		switch (lastState)
	{
		case GOTO:
			cout << "#GOTO" << endl;
			if (dPlayer2Ball <= GOTO_RANGE)
			{
				setState(CIRCLE);
			}
			break;

		case CIRCLE:
			cout << "#CIRCLE" << endl;
			if (dPlayer2Ball <= CIRCLE_R*1.25 && fabs(angPlayer2Ball - angFinal)<CIRCLE_PRECISION)
			{
				setState(AIM);
			}
			break;

		case AIM:
			cout << "#AIM" << endl;
			if (fabs(Normalize(angPlayerFacing - angFinal)) < AIM_PRECISION)
			{	
				setState(KICK);
			}
			break;

		case KICK:
			cout << "#KICK" << endl;
			if (pPlayer.dist(pBall) >KICK_ABANDON_RANGE)
			{
				setState(AIM);
			}
			else if (fabs(Normalize(angPlayerFacing - angFinal)) > KICK_ABANDON_PRECISION)
			{
				setState(AIM);
			}
			break;


		default:
			setState(GOTO);
			break;
	}
	lastState = state();

	////////////////////////////////////////////////////////////////////////////////
	//                            Plan States
	////////////////////////////////////////////////////////////////////////////////
	switch (state())
	{
	case GOTO:
	{
				if (dPlayer2Ball > CHASE_RANGE)
				 {
					 GDebugEngine::Instance()->gui_debug_line(pPlayer, pChase, COLOR_RED);//gui_debug_x(pChase, COLOR_RED);
					 myTask.player.pos = pChase;
					 myTask.player.angle = angPlayer2Ball;//finalDir;
					 myTask.player.flag = PlayerStatus::QUICKLY;
					 setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(myTask));
				 }
				 else
				 {
					 CGeoPoint pAvoid;
					 if (fabs(Normalize(ang1 - angBall2Final))<fabs(Normalize(ang2 - angBall2Final)))
						 pAvoid = pBall + Polar2Vector(CIRCLE_R*1.20, ang1);
					 else
						 pAvoid = pBall + Polar2Vector(CIRCLE_R*1.20, ang2);

					 double delta = fabs(dPlayer2Ball - CIRCLE_R);
					 double delta_tri = pow(delta, 4) * 10; //¶ã±Ü&¿¿½ü²ÎÊý
					 if (dPlayer2Ball > CIRCLE_R + ROUND_RANGE_TOLERANCE)
					 {
						 pAvoid = pAvoid + Polar2Vector(vPlayer2Ball.dir(), delta_tri);
					 }
					 else if (dPlayer2Ball < CIRCLE_R)
					 {
						 pAvoid = pAvoid + Polar2Vector(Normalize(vPlayer2Ball.dir() + pi), delta_tri * 2);
					 }


					 if (isSameDir)
					 {
						 GDebugEngine::Instance()->gui_debug_line(pPlayer, pFinal, COLOR_RED);
						 myTask.player.pos = pFinal;
						 myTask.player.angle = angFinal;
						 myTask.player.flag = PlayerStatus::QUICKLY;
						 setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(myTask));
					 }
					 else
					 {
						 GDebugEngine::Instance()->gui_debug_line(pPlayer, pAvoid, COLOR_RED);
						 myTask.player.pos = pAvoid;
						 myTask.player.angle = angFinal;
						 myTask.player.flag = PlayerStatus::QUICKLY;
						 setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(myTask));
					 }

				 }
				 break;
	}
	case CIRCLE:
	{
				   double dt = 1 / 20.0;
				   double vmin;
				   CVector V, V0;
				   CGeoPoint B, P;
				   CGeoCirlce cirBall = CGeoCirlce(pBall, CIRCLE_R);
				   B = pBall;
				   P = pPlayer;
				   CVector Vpb,Vpbv,a1,a2;
				   Vpb = B - P;
				   a1 = Polar2Vector(1.0, Vpb.dir() + pi / 2);
				   a2 = Polar2Vector(1.0, Vpb.dir() - pi / 2);
				   if (a1*V >= 0) Vpbv = a1;
				   else Vpbv = a2;
				   
				   V = me.Vel();
				   vmin = V.mod() - dt*MAX_TRANSLATION_DEC;
				   V0 = Vpbv*(V*Vpbv);
				   
				   if (V0.mod() < vmin) //make sure that V0 isn't too small
				   {
					   V0 = V0 / V0.mod()*vmin;
				   }

				   CGeoPoint Pp = P + V0*dt;
				   GDebugEngine::Instance()->gui_debug_x(Pp, COLOR_YELLOW);

				   CGeoLine lPlayerPre2Ball = CGeoLine(B,Pp);
				   CGeoLineCircleIntersection intersect1 = CGeoLineCircleIntersection(lPlayerPre2Ball, cirBall);
				   CGeoPoint Gp = (intersect1.point1().dist2(P) < intersect1.point2().dist2(P)) ?
					   intersect1.point1() : intersect1.point2();

				   CVector Vpg, Vdst, a3;
				   Vpg = Gp - P;
				   a3 = Polar2Vector(1.0, Vpg.dir());
				   Vdst = a3*(a3*V);

				   if (Vdst.mod() < vmin) //make sure that Vdst isn't too small
				   {
					   Vdst = Vdst / Vdst.mod()*vmin;
				   }

				   double d = Vpg.mod();
				   double a = 2 * (d / pow(dt, 2) - Vdst.mod() / dt);

				   double w = 0;
				   w = -10 * Normalize(angPlayerFacing - angPlayer2Ball);
					   
				   Vdst = a3*(Vdst.mod() + a*dt);
				   setSubTask(PlayerRole::makeItRun(vecNumber, Vdst.x(), Vdst.y(), w, 0));
				   
				   /*
				   double dt = 1 / 60.0;
				   CGeoPoint pPlayerPredict = pPlayer + V*dt;
				   CGeoLine lPlayerPre2Ball = CGeoLine(pBall, pPlayerPredict);
				   CGeoLineCircleIntersection intersect1 = CGeoLineCircleIntersection(lPlayerPre2Ball, cirBall);
				   CGeoPoint pPreSec = (intersect1.point1().dist2(pPlayer) < intersect1.point2().dist2(pPlayer)) ?
					   intersect1.point1() : intersect1.point2();

				   
				   GDebugEngine::Instance()->gui_debug_line(pBall, pPlayerPredict,COLOR_RED);
				   GDebugEngine::Instance()->gui_debug_x(pPreSec, COLOR_YELLOW);

				   CGeoLine lRPP = CGeoLine(pPlayer, pPreSec);
				   CVector Vrpp = pPreSec - pPlayer;
				   CVector Vp = (Vrpp / Vrpp.mod())*((V*Vrpp) / Vrpp.mod());
				   double Dp = Vp.mod();
				   double a = 2 * (Dp / pow(dt, 2) - Vp.mod() / dt);
				   if (a>MAX_TRANSLATION_ACC) a = MAX_TRANSLATION_ACC;
				   else if (a < -1 * MAX_TRANSLATION_DEC) a = -1 * MAX_TRANSLATION_DEC;

				   CVector Vcnt = Vp / Vp.mod()*(Vp.mod() + a*dt);
				   GDebugEngine::Instance()->gui_debug_line(pPlayer,pPlayer+Vcnt, COLOR_YELLOW);
				   */

				   break;
	}
	case AIM:
	{
				if (kickType == KICKTYPE_CHIP)
				{
					DribbleStatus::Instance()->setDribbleCommand(vecNumber, 3);
				}

				myTask.player.pos = pBall + Polar2Vector(AIM_RANGE, angFinalR);//Polar2Vector(vFinal2Ball.dir(),AIM_RANGE);
				myTask.player.angle = angFinal;
				GDebugEngine::Instance()->gui_debug_x(pBall + Polar2Vector(angFinalR, 40), COLOR_RED);
				myTask.player.flag = PlayerStatus::QUICKLY;
				setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(myTask));
				break;
	}
	case KICK:  
	{
					myTask.executor = vecNumber;
					myTask.player.angle = angFinal;
					setSubTask(TaskFactoryV2::Instance()->NoneTrajGetBall(myTask));

					if (kickType == KICKTYPE_CHIP)
					{
						DribbleStatus::Instance()->setDribbleCommand(vecNumber, 3);
						KickStatus::Instance()->setChipKick(vecNumber, kickPower);
					}

					else if (kickType == KICKTYPE_FLAT)
					{
						KickStatus::Instance()->setKick(vecNumber, kickPower);
					}
					break;
	}
	default:
	{
			   cout << "WARNING : GoAndTurnKick with no STATE !" << endl;
			   break;
	}
	}
	//That's an end .
	return CStatedTask::plan(pVision);
}

CPlayerCommand* CCircleAndPass::execute(const CVisionModule* pVision)
{
	if (subTask()) {
		return subTask()->execute(pVision);
	}

	return NULL;
}
