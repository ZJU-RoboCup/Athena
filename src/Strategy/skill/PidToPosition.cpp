#include "PidToPosition.h"
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

#define PID_TEST_MODE false
#define PID_DEBUG if(PID_TEST_MODE==true)

namespace
{
	const double p=1.2;
	const double i=0.8;
	const double d=0.3;

	const double degree = 0.017453;
	const double pi = 3.141592653;

	const double PID_QUANTUM_SPEED =1;
	const double PID_MAX_SPEED = 200;
	const double PID_BASIC_SPEED = 150;
	const int PID_SMART_THRESH = 20;
	const double PID_ROTATE_COEFF = 1.5;
	
}


void CPidToPosition::plan(const CVisionModule* pVision)
{
	int iRobotNum = task().executor;
	const PlayerVisionT& objPlayer = pVision->OurPlayer(iRobotNum);
	const MobileVisionT& objBall = pVision->Ball();

	//ÕºœÒ ”æı
	//Ball Infomation	
	const CGeoPoint pBall = objBall.Pos();
	const CVector vBallVel = objBall.Vel();
	const double dBallVel = vBallVel.mod();
	const double angBallVel = vBallVel.dir();

	//Player Infomation
	const CGeoPoint pPlayer = objPlayer.Pos();
	const double angPlayerFacing = Normalize(objPlayer.Dir());

	const CVector vPlayerVel = objPlayer.Vel(); //pVision->OurRawPlayerSpeed(robotNum);
	const double dPlayerVel = vPlayerVel.mod();
	const double angPlayerVel = vPlayerVel.dir();

	const CGeoPoint pTask = task().player.pos;
	const double angTask = task().player.angle;
	
	const CVector vPlayer2Task = pTask - pPlayer;
	const double  dPlayer2Task = vPlayer2Task.mod();
	const double angPlayer2Task = vPlayer2Task.dir();

	////////////////////////////////////////////////////////////////
	//◊¥Ã¨«–ªª”ÎtaskGoalie…Ë÷√
	////////////////////////////////////////////////////////////////

	CGeoPoint pDst;
	double angDst = task().player.angle;
	double dControlP = 0;
	double dControlIx = 0;
	double dControlIy = 0;
	double dControlD = 0;
	double dControl = 0;

	PID_DEBUG cout << "-----------Data-----------" << endl;
	//p
	if (dPlayer2Task < PID_SMART_THRESH * 2)
		dControlP = dPlayer2Task*0.5*p;
	else
		dControlP = dPlayer2Task*p;
	PID_DEBUG cout << "P:" << dControlP << endl;

	//i
	static double integrateX = 0;
	static double integrateY = 0;
	integrateX += vPlayer2Task.x()/60.0;
	integrateY += vPlayer2Task.y()/60.0;
	integrateX *= 0.9;
	integrateY *= 0.9;
	dControlIx = integrateX*i;
	dControlIy = integrateY*i;
	PID_DEBUG cout << "Ix:" << dControlIx<<endl;
	PID_DEBUG cout << "Iy:" << dControlIy<<endl;
	
	//d
	static double hist = 0;
	static double diff = 0;
	diff = -abs(hist-dPlayer2Task);
	hist = dPlayer2Task;
	dControlD = diff*d;
	PID_DEBUG cout << "d:" << dControlD << endl;


	if (pPlayer.dist(pTask) < PID_SMART_THRESH)
	{
		setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(task()));
		integrateX = 0;
		integrateY = 0;
	}
	else
	{
		double x, y, w=0;
		double angDst;
		double angDelta1,angDelta2;
		angDst=Normalize(vPlayer2Task.dir());
		angDelta1 = Normalize(angPlayerFacing - angDst);
		angDelta2 = Normalize(angPlayerFacing - (angDst + pi));
		//if (abs(angDelta) < 3.5 * degree) angDelta = 0;

		x = Polar2Vector(PID_QUANTUM_SPEED, angDst).x()*(dControlP-  dControlD);
		y = Polar2Vector(PID_QUANTUM_SPEED, angDst).y()*(dControlP - dControlD); //dControl;
		x += PID_QUANTUM_SPEED*dControlIx;
		y += PID_QUANTUM_SPEED*dControlIy;
		if (abs(x)>PID_MAX_SPEED || abs(y) > PID_MAX_SPEED)
		{
			if (abs(x) >= abs(y))
			{
				y = Sign(y)*(abs(y) / abs(x))*PID_MAX_SPEED;
				x = Sign(x)*PID_MAX_SPEED;
			}
			else
			{
				x = Sign(x)*(abs(x) / abs(y))*PID_MAX_SPEED;
				y = Sign(y)*PID_MAX_SPEED;
			}
		}

		x += Polar2Vector(PID_BASIC_SPEED, angDst).x();// *(dControlP - dControlD);
		y += Polar2Vector(PID_BASIC_SPEED, angDst).y(); //*(dControlP - dControlD); //dControl;
		
		cout << abs(angDelta1) << abs(angDelta2) << endl;
		if (abs(angDelta1) <abs(angDelta2))
		w = -angDelta1*PID_ROTATE_COEFF;
		else
		w = -angDelta2 * PID_ROTATE_COEFF;
	

		GDebugEngine::Instance()->gui_debug_line(pPlayer, pPlayer + Polar2Vector(100, angPlayerFacing), COLOR_RED);
		setSubTask(PlayerRole::makeItRun(iRobotNum, x, y, w, PlayerStatus::QUICKLY));
	}
	CStatedTask::plan(pVision);
}


CPlayerCommand* CPidToPosition::execute(const CVisionModule* pVision)
{
	if (subTask()){
		return subTask()->execute(pVision);
	}
	return NULL;
}
