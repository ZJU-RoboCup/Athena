/////////////////////////////////////////////////////////////////////////
// Editor : YuN
// Date   : 2015/04/25
// Descri : New GoAndTurnKick with good performance .
/////////////////////////////////////////////////////////////////////////
#define KICKTYPE_CHIP 1
#define KICKTYPE_FLAT 0
#define GO_AND_TURN_KICK_DEBUG false
#define DEBUG if( GO_AND_TURN_KICK_DEBUG)
#include "GoAndTurnKick.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include "ControlModel.h"
#include "KickStatus.h"
#include <utils.h>
#include <ControlModel.h>
#include <DribbleStatus.h>
#include <RobotSensor.h>

using namespace Utils;
namespace
{
	//Set State
	enum goAndTurnKickState
	{
		GOTO = 1,
		AIM = 2,
		KICK = 3
	};
	bool IS_SIMULATION = false;
	int AimCnt = 0;
	int AimMaxCnt = 3;
	
	//Set Math Constant
	static double pi = 3.141592653;
	static double degree = pi / 180.0;

	//Set Configs
	static double CHASE_RANGE = 130.0;			//追球范围
	static double CHASE_EXTRA_SPEED = 35.0;		//追球额外速度

	static double GOTO_RANGE = 45.0;			//GOTO范围
	static double GOTO_PRECISION = 3 * degree;  //GOTO精度
	static double GOTO_EXTRA_SPEED = 5.0;

	static double ROUND_RANGE = 8.0 + Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE;			//GOTO圆弧半径
	static double AVOID_RANGE = 13.0 + Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE;
	static double ROUND_RANGE_TOLERANCE = 4.0;	//GOTO范围容限

	static double AIM_RANGE =3.0 + Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE;//AIM范围
	static double AIM_PRECISION = 1.5 * degree;	//AIM精度

	static double KICK_ABANDON_RANGE = 5.0 + Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE;		//KICK放弃范围_距离
	static double KICK_ABANDON_PRECISION = 3.0 * degree; //KICK放弃范围_角度

	static double DribbleTimeMax = 0.15;

	enum KICK_METHOD
	{
		SMART = 1,
		NONE_TRAG=2
	};
	const KICK_METHOD KICK_METHOD_OPT = SMART;
}

CGoAndTurnKick::CGoAndTurnKick()
{
	DECLARE_PARAM_READER_BEGIN(General)
	READ_PARAM(IS_SIMULATION)
		DECLARE_PARAM_READER_END
	_lastCycle = 0;
}

void CGoAndTurnKick::plan(const CVisionModule* pVision)
{
	// 判断是否需要进行状态重置
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
	const CGeoPoint pFinal = pBall + Polar2Vector(ROUND_RANGE, angFinalR);
	const CGeoPoint pChase = pBall + Polar2Vector(CHASE_EXTRA_SPEED, angPlayer2Ball);
	CVector vFinal2Ball = pBall - pFinal;

	bool isSameDir = (vPlayer2Ball*vFinal2Ball >= 0) ? true : false;
	double ang1, ang2;
	double angBall2Final = Normalize(vFinal2Ball.dir() + pi);

	if (dPlayer2Ball > AVOID_RANGE)
	{//远距离跑 90°切线
		
		ang1 = Normalize(angPlayer2Ball + pi / 2);
		ang2 = Normalize(angPlayer2Ball - pi / 2);
	}
	else
	{//近距离跑小角切线
		ang1 = Normalize(angPlayer2Ball + pi * 0.77);
		ang2 = Normalize(angPlayer2Ball - pi * 0.77);
	}
	
	CGeoPoint pAvoid;
	if (fabs(Normalize(ang1 - angBall2Final))<fabs(Normalize(ang2 - angBall2Final)))
		pAvoid = pBall + Polar2Vector(ROUND_RANGE, ang1);
	else
		pAvoid = pBall + Polar2Vector(ROUND_RANGE, ang2);

	double delta = fabs(dPlayer2Ball - ROUND_RANGE);
	double delta_tri = pow(delta, 3) * 2; //躲避&靠近参数

	if (dPlayer2Ball > ROUND_RANGE + ROUND_RANGE_TOLERANCE)
	{
		pAvoid = pAvoid + Polar2Vector(vPlayer2Ball.dir(), delta_tri/3);
	}
	else if (dPlayer2Ball < ROUND_RANGE)
	{
		pAvoid = pAvoid + Polar2Vector(delta_tri, Normalize(vPlayer2Ball.dir() + pi));
	}
	//SetTask
	TaskT myTask(task());
	myTask.player.flag = myTask.player.flag | PlayerStatus::DODGE_OUR_DEFENSE_BOX;

	////////////////////////////////////////////////////////////////////////////////
	//                             Alter States
	////////////////////////////////////////////////////////////////////////////////
	static int lastState = GOTO;
	static int Cycle;
	static int lastCycle;
	static int kickCycle = 0;
	static int dribbleTime = 0;
	Cycle = pVision->Cycle();

	if (Cycle - lastCycle > 10)
	{//#1: Force Alter
		DEBUG cout << "*REFRESH" << endl << "#GOTO" << endl;
		setState(GOTO);
	}
	else if (dPlayer2Ball > GOTO_RANGE)
	{//#2: Force Alter
		DEBUG cout << "#GOTO" << endl;
		setState(GOTO);
	}
	else
		switch (lastState)
	{
		case GOTO:
			DEBUG cout << "#GOTO" << endl;
			DEBUG cout << fabs(angPlayer2Ball - angFinal) << "^" << endl;
			if (dPlayer2Ball < ROUND_RANGE*1.2 && fabs(angPlayer2Ball - angFinal)<GOTO_PRECISION)
			{
				setState(AIM);
			}
			break;

		case AIM:
			DEBUG cout << "#AIM" << endl;
			if (fabs(Normalize(angPlayerFacing - angFinal)) < AIM_PRECISION)
			{
				setState(KICK);
				kickCycle = pVision->Cycle();
				dribbleTime = 0;
			}
			break;

		case KICK:
			DEBUG cout << "#KICK" << endl;
			if (pPlayer.dist(pBall) > GOTO_RANGE)
			{
				setState(GOTO);
			}
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
	{	 //冲向球
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
	case AIM:
	{
				if (kickType == KICKTYPE_CHIP)
				{
					DribbleStatus::Instance()->setDribbleCommand(vecNumber, 3);
				}

				GDebugEngine::Instance()->gui_debug_line(pPlayer, pAvoid, COLOR_RED);
				myTask.player.pos = pBall + Polar2Vector(AIM_RANGE, angFinalR);
				GDebugEngine::Instance()->gui_debug_x(myTask.player.pos, COLOR_BLUE);
				
				myTask.player.angle = angFinal;
				myTask.player.flag = PlayerStatus::QUICKLY;
				setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(myTask));
				break;
	}
	case KICK:  //*踢球由lua设定,此处仅负责走位
	{		
				bool canKick = false;
				myTask.executor = vecNumber;
				myTask.player.pos = pBall;
				myTask.player.angle = angFinal;
				myTask.player.flag = PlayerStatus::QUICKLY;
				
				 if (kickType == KICKTYPE_CHIP)
				 {
					 DribbleStatus::Instance()->setDribbleCommand(vecNumber, 3);
					 if (RobotSensor::Instance()->IsInfraredOn(vecNumber))
					 {
						 dribbleTime++;
						 //setSubTask(TaskFactoryV2::Instance()->StopRobot(myTask));
						 //KickStatus::Instance()->setChipKick(vecNumber, kickPower);
					 }
					 else
					 {
						 if (KICK_METHOD_OPT == NONE_TRAG)
							 setSubTask(TaskFactoryV2::Instance()->NoneTrajGetBall(myTask));
						 else if (KICK_METHOD_OPT == SMART)
							 setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(myTask));
					 }

					 if (dribbleTime > DribbleTimeMax*60)
					 {
						 //cout << "C H I P !!" << endl;
						 if (KICK_METHOD_OPT == NONE_TRAG)
							 setSubTask(TaskFactoryV2::Instance()->NoneTrajGetBall(myTask));
						 else if (KICK_METHOD_OPT == SMART)
							 setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(myTask));
						 KickStatus::Instance()->setChipKick(vecNumber, kickPower);
					 }
				  }

				 else if (kickType == KICKTYPE_FLAT)
				 {
					 if (KICK_METHOD_OPT == NONE_TRAG)
						 setSubTask(TaskFactoryV2::Instance()->NoneTrajGetBall(myTask));
					 else if (KICK_METHOD_OPT == SMART)
						 setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(myTask));

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
	lastCycle=Cycle;
	return CStatedTask::plan(pVision);
}

CPlayerCommand* CGoAndTurnKick::execute(const CVisionModule* pVision)
{
	if (subTask()) {
		return subTask()->execute(pVision);
	}

	return NULL;
}
