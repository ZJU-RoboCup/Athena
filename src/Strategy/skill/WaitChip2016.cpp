#include "WaitChip2016.h"
#include "skill/Factory.h"
#include "GDebugEngine.h"
#include "BallSpeedModel.h"
#include "BallStatus.h"
#include "WaitKickPos.h"
#include "DribbleStatus.h"
#include "KickStatus.h"
#include "KickDirection.h"
#include "WorldModel.h"

using namespace Utils;

namespace{

	enum STATE {
		WAIT = 2,
		AIM = 3,
		SAVE = 4,
		
	};

	const double deg = Param::Math::PI / 180;

	const double BIG_ANG_THRESH = 85 * deg;
	const double AIM_ENTER_ANG_THRESH = 30 * deg;

	const double REACT_RANGE = 100.0;
	const double MUST_REACT_RANGE = 50.0;

	const double AIM_DEG_PRECISION = 1.0;
	const double AIM_DIST_PRECISION = 1.0;


	const double CENTER_TO_FRONT = 7.2;//7.2;
	const int BALL_LINE_HITS_MIN = 4;
	const int BALL_LINE_HITS_MAX = 15;
	const double BLOCK_LENGTH = 5;//1.7;//1.5;

	const double BALL_SPEED_DYNA = 70.0;
	const double BALL_SPEED_STAT = 60.0;

	const int BALL_LAND_JUDGE_RANGE = 5;
	const double BALL_LAND_ANG_THRESH = 2 * deg;
}


CWaitChip2016::CWaitChip2016()
{
	last_cycle = 0;
	cur_cnt = 0;
	need_shoot = false;
}

bool CWaitChip2016::calcBallLine(CGeoPoint& pt, CGeoLine& line, int mode)
{
	static int hits_count = 0;
	static bool is_full = false;
	static CGeoPoint hits[50];
	static CGeoLine reg_line;

	if (mode == 1)
	{
		is_full = false;
		hits_count = 0;
		return false;
	}
	else if (mode == 2)
	{
		line = reg_line;
	}

	hits[hits_count] = pt;
	hits_count++;
	if (hits_count >= BALL_LINE_HITS_MAX)
	{
		is_full = true;
		hits_count = 0;
	}

	if (hits_count >= BALL_LINE_HITS_MIN || is_full)
	{
		double Ex, Ex2, Ey, Exy;
		int N;
		if (is_full) N = BALL_LINE_HITS_MAX;
		else N = hits_count;

		Ex = 0, Ex2 = 0, Ey = 0, Exy = 0;
		for (int i = 0; i < N; i++)
		{
			Ex = Ex + hits[i].x();
			Ey = Ey + hits[i].y();
			Ex2 = Ex2 + hits[i].x()*hits[i].x();
			Exy = Exy + hits[i].x()*hits[i].y();
			GDebugEngine::Instance()->gui_debug_x(hits[i], COLOR_YELLOW);
		}

		double a, b;

		b = (N*Exy - Ex*Ey) / (N*Ex2 - Ex*Ex);
		a = (Ey - b*Ex) / N;
		//cout << "#" << a << "!" << b << endl;
		double x1, x2, y1, y2;
		x1 = -300;
		x2 = 300;
		y1 = b*x1 + a;
		y2 = b*x2 + a;
		reg_line = CGeoLine(CGeoPoint(x1, y1), CGeoPoint(x2, y2));
		line = reg_line;
		GDebugEngine::Instance()->gui_debug_line(CGeoPoint(x1, y1), CGeoPoint(x2, y2), COLOR_ORANGE);
		return true;
	}
	return false;
}

bool CWaitChip2016::isBallLanded(CVector ball_vel,int mode)
{
	static bool isFull=false;
	static int index=0;
	double ballDir[30];

	if (mode == 1)
	{
		index = 0;
		isFull = false;
		return false;
	}

	ballDir[index] = ball_vel.dir();
	index++;
	if (index == 30) index = 0;


	int indexPast;
	if ((index-1) >= BALL_LAND_JUDGE_RANGE)
		indexPast = (index - 1) - BALL_LAND_JUDGE_RANGE;
	else if (isFull)
		indexPast = 30 + (index - 1) - BALL_LAND_JUDGE_RANGE;
	else
		return false;

	if (abs(Normalize(ballDir[index - 1] - ballDir[indexPast])) < BALL_LAND_ANG_THRESH)
		return true;
	else
		return false;
}

void CWaitChip2016::plan(const CVisionModule* pVision)
{
	//Cycle的处理
	static int curCycle;
	static int lastCycle;

	int player = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(player);
	const MobileVisionT& ball = pVision->Ball();

	curCycle = pVision->Cycle();;
	static STATE curState = WAIT;
	if (curCycle - lastCycle>5) {
		curState = WAIT;
		static CGeoPoint pBallInit = ball.Pos();
		isBallLanded(CVector(0, 0), 1);
	}


	//Task的处理
	TaskT myTask(task());
	myTask.player.flag = myTask.player.flag | PlayerStatus::DODGE_OUR_DEFENSE_BOX;

	//几何信息获取
	CGeoPoint pPlayer = me.Pos();
	CVector vPlayer = me.Vel();
	double dPlayer = me.Dir();

	CGeoPoint pBall = ball.Pos();
	static CGeoPoint pBallLast;
	CVector vBall = ball.Vel();
	static CVector vBallLast = CVector(0, 0);

	static CGeoLine lBall;
	if (!ball.Valid())
	{
		calcBallLine(pBall, lBall, 2);
	}
	else if (vBall.mod() > 75)
	{
		if (!calcBallLine(pBall, lBall))
			lBall = CGeoLine(pBall, vBall.dir());
	}
	else
	{
		calcBallLine(pBall, lBall, 1);
		lBall = CGeoLine(pBall, vBall.dir());
	}
	pBallLast = pBall;

	double dTask = myTask.ball.angle;
	CGeoPoint pTask = myTask.ball.pos;

	CVector vPlayer2Ball = pBall - pPlayer;
	double dPlayer2Ball = vPlayer2Ball.dir();
	CVector vBall2Player = vPlayer2Ball * -1;

	CGeoPoint pProject = lBall.projection(pTask);
	double distFromT2P = pProject.dist(pTask);

	CGeoPoint pMouth = pPlayer + Polar2Vector(CENTER_TO_FRONT, dPlayer);
	CGeoPoint pPlrProject = lBall.projection(pMouth);

	CGeoLine lShoot, lVerti;
	CGeoPoint pInter;
	CVector vControl, vOutput;

	CGeoCirlce cirRange = CGeoCirlce(pTask, REACT_RANGE*1.25);
	bool isBall2Player = vBall.mod() > BALL_SPEED_DYNA && CGeoLineCircleIntersection(lBall, cirRange).intersectant() && vPlayer2Ball*vBall<0;
	bool isBallLowSpeed = ball.Valid() && vBall.mod() < BALL_SPEED_STAT;
	double dDelta = abs(Normalize(vPlayer2Ball.dir() - dTask));

	CVector vBall2Gole = CGeoPoint(450, 0) - pBall;

	//对阻挡进行补偿
	CGeoPoint pControl, pTouch;
	CVector vVer1, vVer2, vFix;
	double dFix;
	vVer1 = Polar2Vector(1.0, dPlayer + 90 * deg);
	vVer2 = Polar2Vector(1.0, dPlayer - 90 * deg);
	if (vVer1*vBall2Player > 0)
	{
		dFix = abs(Normalize(vVer1.dir() - vBall2Player.dir()));
		vFix = vVer1*(BLOCK_LENGTH / tan(dFix));
		GDebugEngine::Instance()->gui_debug_line(pMouth, pMouth + vFix, COLOR_RED);
	}
	else
	{
		dFix = abs(Normalize(vVer2.dir() - vBall2Player.dir()));
		vFix = vVer2*(BLOCK_LENGTH / tan(dFix));
		GDebugEngine::Instance()->gui_debug_line(pMouth, pMouth + vFix, COLOR_RED);
	}
	pTouch = lBall.projection(pMouth + vFix);
	pControl = pTouch + vFix*(-1) + Polar2Vector(CENTER_TO_FRONT, dTask + 180 * deg);

	GDebugEngine::Instance()->gui_debug_x(pTouch, COLOR_RED);

	static double sum, delta, delta_last, p, i, d;

	//状态切换
	switch (curState)
	{

	case WAIT:
		if ((isBallLanded(vBall) || pPlayer.dist(pBall) < MUST_REACT_RANGE) && isBall2Player)
		{
			curState = AIM;
			calcBallLine(pBall, lBall, 1);
		}

		else if (pPlayer.dist(pBall) < MUST_REACT_RANGE && isBallLowSpeed)
			curState = SAVE;

	case AIM:
		if (isBallLowSpeed)
			curState = SAVE;
		else if (WorldModel::Instance()->IsBallKicked(player))
			curState = WAIT;
		break;

	case SAVE:
		if (pBall.dist(pPlayer) > REACT_RANGE || vBall.mod() > BALL_SPEED_DYNA)
		{
		curState = WAIT;
		isBallLanded(vBall, 1);
		}
		break;
	default:
		break;
	}

	//状态执行
	switch (curState)
	{
	case WAIT:
		//cout << "RUN:" << curCycle << endl;
		if (distFromT2P < MUST_REACT_RANGE && pPlayer.dist(pTask)<MUST_REACT_RANGE*1.25)
			myTask.player.pos = pControl;
		else
			myTask.player.pos = pTask;

		myTask.player.angle = dTask;
		myTask.player.flag = PlayerStatus::QUICKLY;
		setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(myTask));
		break;

	case AIM:
		//cout << "AIM:" << curCycle << endl;
		KickStatus::Instance()->setKick(player, 700);
		myTask.player.pos = pControl;
		myTask.player.angle = dTask;
		myTask.player.flag = PlayerStatus::QUICKLY;
		setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(myTask));
		break;

	case SAVE:
		setSubTask(PlayerRole::makeItGoAndTurnKick(player, vBall2Gole.dir(), 0, 700));
		break;

	default:
		break;
	}

	lastCycle = curCycle;
	vBallLast = vBall;
	CStatedTask::plan(pVision);
}

CPlayerCommand* CWaitChip2016::execute(const CVisionModule* pVision)
{
	if (subTask()) {
		return subTask()->execute(pVision);
	}
	return NULL;
}