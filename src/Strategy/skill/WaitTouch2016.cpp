#include "WaitTouch2016.h"
#include "skill/Factory.h"
#include "GDebugEngine.h"
#include "BallSpeedModel.h"
#include "BallStatus.h"
#include "WaitKickPos.h"
#include "DribbleStatus.h"
#include "KickStatus.h"
#include "KickDirection.h"
#include "WorldModel.h"
#include <fstream>
#include <tinyxml/ParamReader.h>

using namespace Utils;
// shoot dir is RAW ,for compensation measure.
namespace{

	enum STATE {
		RUN = 1,
		AIM = 2,
		SAVE = 3,
		BIGANG = 4
	};

	const bool verBos = false;
	const double deg = Param::Math::PI / 180;

	const double BIG_ANG_THRESH = 85 * deg;
	const double AIM_ENTER_ANG_THRESH = 30 * deg;

	const double REACT_RANGE = 60.0;
	const double AIM_DEG_PRECISION = 1.0;
	const double AIM_DIST_PRECISION = 1.0;


	const double CENTER_TO_FRONT = 7.2;//7.2;
	const int BALL_LINE_HITS_MIN = 3;
	const int BALL_LINE_HITS_MAX = 10;
	const double BLOCK_LENGTH = 1.7;//1.7;//1.5;

	const double BALL_SPEED_DYNA = 100.0;
	const double BALL_SPEED_STAT = 50.0;
	const double FRICTION = 1;
	const int SAFE_DISTANCE = 50;
}


namespace{
	int MAX_CNT_FOR_SHOOT = 2;
	CGeoPoint last_pTouch = CGeoPoint(0,0);
}
using namespace std;
CWaitTouch2016::CWaitTouch2016()
{
	last_cycle = 0;
	cur_cnt = 0;
	need_shoot = false;
	ofstream of("yes.txt");
	int num=90;
	of<<num;
}
bool CWaitTouch2016::calcBallLine(CGeoPoint& pt, CGeoLine& line, int mode)
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
			if(verBos) GDebugEngine::Instance()->gui_debug_x(hits[i], COLOR_YELLOW);
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
		if(verBos) GDebugEngine::Instance()->gui_debug_line(CGeoPoint(x1, y1), CGeoPoint(x2, y2), COLOR_ORANGE);
		return true;
	}
	return false;
}

void CWaitTouch2016::plan(const CVisionModule* pVision)
{
	//Cycle的处理
	//cout << "waittouch is running " <<endl;
	static int curCycle;
	static int lastCycle;

	static int FRICTION;

	// --------------------------------ftq--------------------------------------
	static double shoot_vel = 0;
	static double shoot_dir = 0;
	// --------------------------------ftq--------------------------------------


	DECLARE_PARAM_READER_BEGIN(FieldParam)
		READ_PARAM(FRICTION)
	DECLARE_PARAM_READER_END

	curCycle = pVision->Cycle();
	static STATE curState = RUN;
	if (curCycle - lastCycle>5) {
		curState = RUN;
	}
	//Task的处理
	int player = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(player);
	const MobileVisionT& ball = pVision->Ball();

	TaskT myTask(task());
	myTask.player.flag = myTask.player.flag | PlayerStatus::DODGE_OUR_DEFENSE_BOX;

	//几何信息获取
	CGeoPoint pPlayer = me.Pos();
	CVector vPlayer = me.Vel();
	double dPlayer = me.Dir();

	CGeoPoint pBall = ball.RawPos();
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

	CGeoSegment pointSegment = CGeoSegment(pTask+Utils::Polar2Vector(REACT_RANGE,dTask), pTask+Utils::Polar2Vector(-REACT_RANGE,dTask));

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

	double instancePrediction = vBall.mod()*vBall.mod()/FRICTION;
	CGeoCirlce cirRange = CGeoCirlce(pTask, REACT_RANGE);
	bool isBall2Player = instancePrediction-vPlayer2Ball.mod()>SAFE_DISTANCE && CGeoLineCircleIntersection(lBall, cirRange).intersectant();//&& vPlayer2Ball*vBall<0;
	bool isBallLowSpeed = ball.Valid() && vBall.mod() < BALL_SPEED_STAT;
	double dDelta = abs(Normalize(vPlayer2Ball.dir() - dTask));

	CVector vBall2Gole = CGeoPoint(450, 0) - pBall;

	//对阻挡进行补偿

	CGeoPoint pControl, pTouch;
	CVector vVer1, vVer2, vFix;
	double dFix;
	bool is_from_aim = false;

	vVer1 = Polar2Vector(1.0, dPlayer + 90 * deg);
	vVer2 = Polar2Vector(1.0, dPlayer - 90 * deg);
	if (vVer1*vBall2Player > 0)
	{
		dFix = abs(Normalize(vVer1.dir() - vBall2Player.dir()));
		vFix = vVer1*(BLOCK_LENGTH / tan(dFix));
		if(verBos) GDebugEngine::Instance()->gui_debug_line(pMouth, pMouth + vFix, COLOR_RED);
	}
	else
	{
		dFix = abs(Normalize(vVer2.dir() - vBall2Player.dir()));
		vFix = vVer2*(BLOCK_LENGTH / tan(dFix));
		if(verBos) GDebugEngine::Instance()->gui_debug_line(pMouth, pMouth + vFix, COLOR_RED);
	}
	pTouch = lBall.projection(pMouth + vFix);

	if(Utils::OutOfField(pTouch, 0) || !pointSegment.IsPointOnLineOnSegment(pTouch) || Utils::InTheirPenaltyArea(pTouch,5)){
		if(verBos) cout << "WaitTouchNew : Pos: OutOfField" << endl;
		pTouch = last_pTouch;
	}
	last_pTouch = pTouch;
	pControl = pTouch + vFix*(-1) + Polar2Vector(CENTER_TO_FRONT, dTask + 180 * deg);

	if(verBos) GDebugEngine::Instance()->gui_debug_x(pTouch, COLOR_RED);

	static double sum, delta, delta_last, p, i, d;

	if(!isBall2Player) curState = SAVE;

	//状态切换
	switch (curState)
	{
	case RUN:
		if (pPlayer.dist(pTask) < REACT_RANGE && isBall2Player && dDelta >= BIG_ANG_THRESH){
			curState = BIGANG;
		}
		else if (pPlayer.dist(pTask) < REACT_RANGE && isBall2Player)
		{
			calcBallLine(pBall, lBall, 1);
			curState = AIM;
		}
		else if (pPlayer.dist(pTask) < REACT_RANGE && pBall.dist(pPlayer) <= REACT_RANGE && isBallLowSpeed){
			curState = SAVE;
		}
		break;
	case BIGANG:
		if (pPlayer.dist(pBall) < REACT_RANGE && isBallLowSpeed)
			curState = SAVE;
		else if (pPlayer.dist(pBall) > REACT_RANGE || WorldModel::Instance()->IsBallKicked(player))
			curState = RUN;
		break;

	case AIM:
		if (isBallLowSpeed){
			curState = RUN;
		}
		else if (WorldModel::Instance()->IsBallKicked(player)){
			curState = RUN;
		}
		if(curState == RUN)
			is_from_aim = true;

		break;

	case SAVE:
		if (pBall.dist(pPlayer) > REACT_RANGE && isBall2Player)
			curState = RUN;
		break;
	default:
		break;
	}
	if(verBos) cout << "state : " << curState << endl;

	bool kicked = true;
	//状态执行
	switch (curState)
	{

	case RUN:
		if (distFromT2P < REACT_RANGE && pPlayer.dist(pTask)<REACT_RANGE*1.25)
			myTask.player.pos = pControl;
		else
			myTask.player.pos = pTask;

		KickDirection::Instance()->GenerateShootDir(player, pPlayer, true);
		// -----------------------------------ftq-----------------------------
		shoot_vel = KickDirection::Instance()->getInSpeed();
		shoot_dir = KickDirection::Instance()->getInDir();
		// -----------------------------------ftq-----------------------------

		//cout << "GenerateShootDir" <<endl;
		dTask = KickDirection::Instance()->getRealKickDir();
		
		myTask.player.angle = dTask;
		myTask.player.flag = PlayerStatus::QUICKLY;
		setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(myTask));
		break;

	case BIGANG:
		setSubTask(PlayerRole::makeItInterKickV4(player, dTask / 3 * 2));
		break;

	case AIM:

		KickDirection::Instance()->GenerateShootDir(player, pPlayer);
		// -----------------------------------ftq-----------------------------
		shoot_vel = KickDirection::Instance()->getInSpeed();
		shoot_dir = KickDirection::Instance()->getInDir();
		// -----------------------------------ftq-----------------------------
		//cout << "aim" <<endl;
		dTask = KickDirection::Instance()->getRealKickDir();

		KickStatus::Instance()->setKick(player, 790);
		myTask.player.pos = pControl;
		myTask.player.angle = dTask;
		myTask.player.flag = PlayerStatus::QUICKLY;
		setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(myTask));

		kicked = false;
		break;

	case SAVE:
		KickDirection::Instance()->GenerateShootDir(player, pPlayer);
		// -----------------------------------ftq-----------------------------
		shoot_vel = KickDirection::Instance()->getInSpeed();
		shoot_dir = KickDirection::Instance()->getInDir();
		// -----------------------------------ftq-----------------------------

		dTask = KickDirection::Instance()->getRealKickDir();
		//cout << "save" <<endl;
		
		if (vBall.mod() > BALL_SPEED_STAT){
			setSubTask(PlayerRole::makeItChaseKickV2(player, dTask, myTask.player.flag));
		}
		else{
			setSubTask(PlayerRole::makeItGoAndTurnKickV4(player, vBall2Gole.dir(), CVector(0,0),0,-2, CMU_TRAJ));
			setSubTask(PlayerRole::makeItShootBall(player, dTask, false, 5 * deg, 1000, 1000));
		}
		break;

	default:
		break;
	}

	double outdir = 0;
	/*if (kicked || WorldModel::Instance()->IsBallKicked(player))
	{
		// 已经射出，求的位置即为射出位置，与kicker的夹角即为
		if (1) {
			CGeoPoint ballPos = pVision->Ball().Pos();
			CGeoPoint robotPos = pVision->OurPlayer(player).Pos();
			double direction = pVision->OurPlayer(player).Dir();

			double self2ball = pVision->OurPlayer(player).Dir() - (pVision->Ball().Pos() - pVision->OurPlayer(player).Pos()).dir();
			outdir = self2ball; // * 180 / Param::Math::PI;
			KickDirection::Instance()->getData(shoot_vel, shoot_dir, outdir);
		}
	}*/
	
	lastCycle = curCycle;
	vBallLast = vBall;
	CStatedTask::plan(pVision);
}

CPlayerCommand* CWaitTouch2016::execute(const CVisionModule* pVision)
{
	if (subTask()) {
		return subTask()->execute(pVision);
	}
	return NULL;
}