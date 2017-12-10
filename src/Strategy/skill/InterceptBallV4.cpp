#include "InterceptBallV4.h"
#include <VisionModule.h>
#include <skill/Factory.h>
#include <BestPlayer.h>
#include <RobotSensor.h>
#include <TaskMediator.h>
#include <PlayInterface.h>
#include <iostream>
#include "GDebugEngine.h"

namespace{
	//重要参数!!!
	bool VOILENCE_MODE = true;
	bool WEAK_OPP = true;//针对静态队的抢球办法

	bool USE_OPP_DIR = true;

	CGeoPoint defenceGoal0;
	CGeoPoint defenceGoal1;
	CGeoPoint defenceGoal2;

	CGeoPoint ourleftPost;
	CGeoPoint ourrightPost;

	bool STATE_PRINT = false;	
	bool debug = false;
	const int maxFrared = 100;
	const int maxMeHasBall = 50;
	double subDist=0;
}

using namespace std;

CInterceptBallV4::CInterceptBallV4()
{
	CGeoPoint defenceGoal0=CGeoPoint(-Param::Field::PITCH_LENGTH/2, 0);
	CGeoPoint defenceGoal1=CGeoPoint(-Param::Field::PITCH_LENGTH/2, 15*Param::Field::RATIO);
	CGeoPoint defenceGoal2=CGeoPoint(-Param::Field::PITCH_LENGTH/2, -15*Param::Field::RATIO);

	CGeoPoint ourleftPost=CGeoPoint(-Param::Field::PITCH_LENGTH/2, -Param::Field::GOAL_WIDTH/2);
	const CGeoPoint ourrightPost=CGeoPoint(-Param::Field::PITCH_LENGTH/2, Param::Field::GOAL_WIDTH/2);
}
CInterceptBallV4::~CInterceptBallV4()
{
	_lastCycle = 0;
	//jamming = false;
	//pushing_opp = false;
}
void CInterceptBallV4::toStream(std::ostream& os)const
{
	os <<" Jam and Push";
}
void CInterceptBallV4::plan(const CVisionModule* pVision)
{
	const int vecNumber = task().executor;
	const int flag = task().player.flag;
	const double finalDir = task().player.angle;
	
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	CGeoPoint myHeadPos = me.Pos() + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER,me.Dir());
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint rawBallPos = pVision->RawBall().Pos();
	CVector self2ball = rawBallPos - me.Pos();
	double diff_with_ball = Utils::Normalize(self2ball.dir() - me.Dir());
	const CGeoPoint& ballTarget = rawBallPos + Utils::Polar2Vector(20,finalDir);
	//GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(200, -150), "^_^"/*"JAMPUSHHHHHHHHH!!!!!!!!!!!!!"*/);
	const bool needDribble = task().player.ischipkick;
	const bool needDribbleWithoutKickWay = task().player.needdribble;

	int opp_num = checkOpp(pVision);
	if ( !opp_num ){ //如果没有对应的敌方车
		//cout << "NO OPP Car" <<endl;
		if (needDribble){;
			DribbleStatus::Instance()->setDribbleCommand(vecNumber, 3);
		}
		setSubTask(PlayerRole::makeItNoneTrajGetBall(vecNumber, 0, CVector(0, 0), flag));
		CPlayerTask::plan(pVision);
		return;
	}

	if ( pVision->Cycle() - _lastCycle > 6 ){
		setState(BEGINNING);
		/*jamming = false;
		pushing_opp = false;*/
	}
	bool frared = RobotSensor::Instance()->IsInfraredOn(vecNumber);
	if ( frared ){
		infraredOn = infraredOn >= maxFrared ? maxFrared : infraredOn+1;
		infraredOff = 0;
	}
	else{
		infraredOn = 0;
		infraredOff = infraredOff >= maxFrared ? maxFrared : infraredOff+1;
	}

	bool isMeHasBall ;
	bool isMechHasBall = infraredOn > 2;
	bool visionHasBall = (isVisionHasBall(pVision,vecNumber))|| (infraredOn > 1 && !ball.Valid());
	//	cout << infraredOn<<endl;
	isMeHasBall = isMechHasBall || visionHasBall;
	if (isMeHasBall)
	{
		meHasBall = meHasBall >=maxMeHasBall ? maxMeHasBall : meHasBall +1;
		meLoseBall = 0;
	}else{
		meHasBall = 0;
		meLoseBall = meLoseBall >=maxMeHasBall? maxMeHasBall:meLoseBall +1;
	}
	const PlayerVisionT& threatonOpp = pVision->TheirPlayer(opp_num);
	CVector opp2ball = rawBallPos - threatonOpp.Pos(); //威胁队员指向球的向量

	bool ballDetected = RobotSensor::Instance()->IsInfraredOn(vecNumber); //红外
	bool isOppProccession = checkOppProccession(pVision, opp_num); // 对方是否拿球，false为没拿球
	bool isKickDirArrived = WorldModel::Instance()->KickDirArrived(pVision->Cycle(), finalDir, Param::Math::PI / 60, vecNumber);
	bool isKickDistArrived = self2ball.mod() < 3 * Param::Vehicle::V2::PLAYER_SIZE;
	bool needKick = isKickDirArrived && isKickDistArrived && task().player.needkick;
	const CGeoLine& ballVelLine = CGeoLine(rawBallPos,ball.Vel().dir());
	const CGeoPoint ProjPoint = ballVelLine.projection(me.Pos());
	double projDist = ProjPoint.dist(me.Pos());										//小车到投影点的距离
	const double ball2projDist = ProjPoint.dist(rawBallPos);							//投影点到球的距离
	
	switch ( state()) 
	{
	case BEGINNING:
		setState(APPROACH_BALL);
		if ( STATE_PRINT )
			cout<<"BEGIN --> APPROACH OPP\n";
		subDist=(ball2projDist*ball2projDist-projDist*projDist)/(2*ball2projDist)/(ball.Vel().mod()+20)*50;
		subDist=subDist/fabs(subDist)*min(fabs(subDist),120);
		break;
	case APPROACH_BALL:
		if ( abs(diff_with_ball) < Param::Math::PI/30 && self2ball.mod() < 13 || ballDetected ){ //diffwithBall：我到球的方向和我的朝向的夹角
			setState(STABLE_BALL);
			if ( STATE_PRINT )
				cout<<"APPROACH OPP --> JAM BALL\n";
		}
		break;
	case STABLE_BALL:
		if (meHasBall > 30)
		{
			setState(TURN);
		}
		if (self2ball.mod()>40)
		{
			setState(APPROACH_BALL);
		}
		if (needKick){
			setState(KICK);
		}
		break;
	case TURN:
		if (abs(Utils::Normalize(finalDir - (self2ball.dir() ))) < Param::Math::PI / 6)
		{
			setState(FINISHED);
		}
		if (needKick){
			setState(KICK);
		}
		if (self2ball.mod()>40)
		{
			setState(APPROACH_BALL);
		}
		break;
	case KICK:
		if (!isKickDistArrived){
			setState(APPROACH_BALL);
		}
		else if(!isKickDirArrived){
			setState(TURN);
		}
	case FINISHED:
		if ( self2ball.mod() > 3*Param::Vehicle::V2::PLAYER_SIZE ){
			setState(APPROACH_BALL);
			if ( STATE_PRINT )
				cout<<"FINISHED --> APPROACH OPP\n";
		}
		break;
	}

	switch ( state() ) 
	{
	case APPROACH_BALL:
		if(debug)cout<<"1"<<endl;
		if(self2ball.mod() < 80){
			DribbleStatus::Instance()->setDribbleCommand(vecNumber, 3);
		}
		planApproachOpp(pVision, vecNumber, opp_num,finalDir,subDist);
		break;
	case STABLE_BALL:
		if(debug)cout<<"2"<<endl;
		DribbleStatus::Instance()->setDribbleCommand(vecNumber, 3);
		if (ball.Vel().mod() < 15 && rawBallPos.dist(me.Pos())<15)
		{
			setSubTask(PlayerRole::makeItDribbleTurn(vecNumber,finalDir,Param::Math::PI/90));
			//setSubTask(PlayerRole::makeItNoneTrajGetBall(vecNumber,finalDir,CVector(0,0),flag));
		}else{
			//setSubTask(PlayerRole::makeItDribbleTurn(vecNumber,me.Dir(),Param::Math::PI/30));
			if (needDribble){
			//cout << "Dribble" << endl;
			DribbleStatus::Instance()->setDribbleCommand(vecNumber, 3);
		}
			setSubTask(PlayerRole::makeItNoneTrajGetBall(vecNumber,me.Dir(),CVector(0,0),flag));
		}
		break;
	case  TURN:
		if (debug)cout << "3"<<endl;
		setSubTask(PlayerRole::makeItDribbleTurn(vecNumber,finalDir,Param::Math::PI/90));
		//DribbleStatus::Instance()->setDribbleCommand(vecNumber, 2);
		//setSubTask(PlayerRole::makeItNoneTrajGetBall(vecNumber,finalDir,CVector(0,0),flag));
		break;
	case  KICK:
		if (!needDribble){
			KickStatus::Instance()->setKick(vecNumber, task().player.kickpower);
			setSubTask(PlayerRole::makeItChaseKickV1(vecNumber, finalDir, 0));
		}
		else{
			DribbleStatus::Instance()->setDribbleCommand(vecNumber, 3);
			KickStatus::Instance()->setChipKick(vecNumber, task().player.chipkickpower);
			setSubTask(PlayerRole::makeItChaseKickV1(vecNumber, finalDir, 0));
		}

	case FINISHED:
		DribbleStatus::Instance()->setDribbleCommand(vecNumber, 3);
		if(debug)cout<<"4"<<endl;
		setSubTask(PlayerRole::makeItDribbleTurn(vecNumber,finalDir,Param::Math::PI/90));
		//setSubTask(PlayerRole::makeItNoneTrajGetBall(vecNumber, finalDir, CVector(0,0),flag));
		break;
	}

	_lastCycle = pVision->Cycle();
	CPlayerTask::plan(pVision);
}
void CInterceptBallV4::planApproachOpp(const CVisionModule* pVision, const int myID, const int oppID,const double finaldir,double subDist)
{
	//cout<<"planApproachOpp"<<endl;
	// 永远堵在对手到我方球门的位置
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint rawBallPos = pVision->RawBall().Pos();
	const PlayerVisionT& me = pVision->OurPlayer(myID);
	const CGeoLine& ballVelLine = CGeoLine(rawBallPos,ball.Vel().dir());
	const CGeoSegment ballVelSeg = CGeoSegment(rawBallPos,rawBallPos+Utils::Polar2Vector(800,ball.Vel().dir()));
	const CGeoPoint ProjPoint = ballVelLine.projection(me.Pos());
	int flag = task().player.flag;
	double projDist = ProjPoint.dist(me.Pos());										//小车到投影点的距离
	const double ball2projDist = ProjPoint.dist(rawBallPos);							//投影点到球的距离
	CGeoPoint faceTarget = CGeoPoint(0,0);
	double faceDir = 0;
	faceDir = Utils::Normalize(ball.Vel().dir() + Param::Math::PI);
	bool meOnBallVelSeg=ballVelSeg.IsPointOnLineOnSegment(ProjPoint);
	
	const double antiBallvel=Utils::Normalize(ball.Vel().dir()+Param::Math::PI);
	bool isPrepare=projDist<5&&Utils::Normalize(me.Dir()-antiBallvel)<Param::Math::PI*6/180&&meOnBallVelSeg;

	bool needDribble = task().player.ischipkick;

	CGeoPoint predictPoint=ProjPoint;
	double ballVelFactor=0.5;
	if (ball.Vel().mod()>220){
		ballVelFactor=0.8;
	}else if (ball.Vel().mod()>150){
		ballVelFactor=0.5;
	}else {
		ballVelFactor=0.2;
	}
	if (ball.Vel().mod() <100){
		if (needDribble){
			//cout << "Dribble" << endl;
			DribbleStatus::Instance()->setDribbleCommand(myID, 3);
		}
		setSubTask(PlayerRole::makeItNoneTrajGetBall(myID, finaldir, CVector(0,0),flag));
	}else{
		if (!isPrepare&&ball2projDist/(ball.Vel().mod()-20)<projDist/150+fabs(Utils::Normalize(faceDir-me.Dir()))/(Param::Math::PI*1.8)+0.09){
			predictPoint=ProjPoint+Utils::Polar2Vector(ball.Vel().mod()*ballVelFactor,ball.Vel().dir());
		}else if(ball.Vel().mod()/ball2projDist<1.2 && ball2projDist>80){
			CVector ballVelVector=-ball.Vel()/ball.Vel().mod();
			predictPoint=ProjPoint+ballVelVector*subDist;
		}else{
			predictPoint=ProjPoint;
		}
		if (Utils::OutOfField(predictPoint,10)){
			CGeoRectangle Field=CGeoRectangle(	CGeoPoint(Param::Field::PITCH_LENGTH / 2.0, -Param::Field::PITCH_WIDTH / 2.0),
																		CGeoPoint(-Param::Field::PITCH_LENGTH / 2.0, Param::Field::PITCH_WIDTH / 2.0));
			CGeoLineRectangleIntersection inter=CGeoLineRectangleIntersection(ballVelLine,Field);
			if (inter.intersectant()){
				if (ball.Vel().dir()>0){
					predictPoint.setY(max(inter.point1().y(),inter.point2().y()));
				}else{
					predictPoint.setY(min(inter.point1().y(),inter.point2().y()));
				}
				if (fabs(ball.Vel().dir())<Param::Math::PI/2)
				{
					predictPoint.setX(max(inter.point1().x(),inter.point2().x()));
				}else{
					predictPoint.setX(min(inter.point1().x(),inter.point2().x()));
				}
			}
		}
		if (!meOnBallVelSeg){
			if (needDribble){
				//cout << "Dribble" << endl;
				DribbleStatus::Instance()->setDribbleCommand(myID, 3);
			}
			setSubTask(PlayerRole::makeItNoneTrajGetBall(myID,faceDir,ball.Vel()*(ballVelFactor*2),flag,10));
		}else{
			setSubTask(PlayerRole::makeItGoto(myID,predictPoint,faceDir,CVector(0,0),0,flag));
		}

	}
}

int CInterceptBallV4::checkOpp(const CVisionModule* pVision)
{
	const CBestPlayer::PlayerList& oppList = BestPlayer::Instance()->theirFastestPlayerToBallList();
	if ( oppList.size() > 0 )
		return oppList[0].num;
	else
		return 0;
}
bool CInterceptBallV4::checkOppProccession(const CVisionModule* pVision, const int oppID)
{
	const PlayerVisionT& opp = pVision->TheirPlayer(oppID);
	if ( !opp.Valid() )
		return false;

	const MobileVisionT& ball = pVision->Ball();
	const ObjectPoseT& raw_ball = pVision->RawBall();

	CGeoPoint ballPos;
	if ( raw_ball.Valid() )
		ballPos = raw_ball.Pos();
	else
		ballPos = ball.Pos();

	const CVector opp2ball = ballPos - opp.Pos();
	double opp_diff_with_ball = USE_OPP_DIR ? Utils::Normalize(opp2ball.dir() - opp.Dir()) : 1.0;

	bool loose = false;
	static int loose_count = 0;
	if ( abs(opp_diff_with_ball) > 1.5*Param::Vehicle::V2::DRIBBLE_ANGLE && opp2ball.mod() > 15 ){
		if ( loose_count++ > 5 ){
			loose_count = 5;
			loose = true;
		}
	}
	else
		loose_count /= 2;

	return !loose;
}

bool CInterceptBallV4::isVisionHasBall(const CVisionModule* pVision,const int vecNumber)
{

	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const MobileVisionT& ball =pVision->Ball();
	double visionJudgDist= Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + Param::Field::BALL_SIZE / 2 +6;
	bool distVisionHasBall = CVector(me.Pos() - ball.Pos()).mod() < visionJudgDist;
	bool dirVisionHasBall ;
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