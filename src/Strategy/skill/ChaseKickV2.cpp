#include "ChaseKickV2.h"
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

using namespace Param;
namespace
{
	//2.�켣�����㷨ʹ�ñ���
	int TRAJECTORY_METHORD = 1;
	double MAX_TRANSLATION_ACC = 650;
	int SPEED_DOWN_MODE = 0;
	enum{
		RUSH_TO_BALL = 1,	//���ٽӽ���
		FOLLOW_BALL = 2,	//����������
		GO_KICK_BALL = 3,	//��ǰ������
		SPEED_UP=4,
		WAIT_BALL=5,
		GET_BALL=6,  
		SPEED_DOWN=7
	};
	bool testOn = false;
	bool verBos= false;
	//״̬�л���ر���
	const double RUSH_TO_BALL_CRITICAL_DIST = 100;	//100cm
	const double FOLLOWBALL_CRITICAL_DIST = 50;		//50cm
	const double GO_KICK_BALL_CRITICAL_DIST = 2*Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE;

	//Ԥ�����
	double CM_PREDICT_FACTOR = 1.5;
	const double Ball_Moving_Fast_Speed = 50;	//1m/s
	//��������
	const double speed_factor = 0.7;
	const double Left_Reach_Allowance=3;
	const double Right_Reach_Allowance=3;
	const double AllowanceFactor = 2.5;
	const int State_Counter_Num=5;      //����״̬��ת
	const double fieldFactor = 0.01;      //��������Ŵ����
	double crossWiseFactor[12] = { 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5 };       //��Ҫ������λ��SPEED_UP״̬�У����ڵ������ܵĲ��죬��Ҫ��������
	double verticalWiseFactor[12]= { 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5 };
	const CGeoPoint LEFT_GOAL_POST = CGeoPoint(-Field::PITCH_LENGTH / 2, -Field::GOAL_WIDTH / 2);
	const CGeoPoint RIGHT_GOAL_POST = CGeoPoint(-Field::PITCH_LENGTH / 2, Field::GOAL_WIDTH / 2);
	const double MaxSpeed=350;
	CR_DECLARE(ROBOT_ID);
	CR_DECLARE(CHASE_KICK_CROSS_FACTOR);
	CR_DECLARE(CHASE_KICK_VERTICAL_FACTOR);
}

CChaseKickV2::CChaseKickV2()
{
	{
		DECLARE_PARAM_READER_BEGIN(CGotoPositionV2)
		READ_PARAM(TRAJECTORY_METHORD)
		READ_PARAM(MAX_TRANSLATION_ACC)
		DECLARE_PARAM_READER_END
	}
	{
		DECLARE_PARAM_READER_BEGIN(CChaseKickV2)
		READ_PARAM(SPEED_DOWN_MODE)
		DECLARE_PARAM_READER_END
	}
	
	CR_DECLARE(ROBOT_ID);
	CR_DECLARE(CHASE_KICK_CROSS_FACTOR);
	CR_DECLARE(CHASE_KICK_VERTICAL_FACTOR);
	CR_SETUP(KickParam, ROBOT_ID, CR_INT);
	CR_SETUP(KickParam, CHASE_KICK_CROSS_FACTOR, CR_DOUBLE);
	CR_SETUP(KickParam, CHASE_KICK_VERTICAL_FACTOR, CR_DOUBLE);
	int robots = VARSIZE(ROBOT_ID);
	for (int i = 0; i < robots; ++i) {
		int robot_num = VIVAR(ROBOT_ID)[i];
		crossWiseFactor[i] = VDVAR(CHASE_KICK_CROSS_FACTOR)[robot_num - 1];
		verticalWiseFactor[i] = VDVAR(CHASE_KICK_VERTICAL_FACTOR)[robot_num - 1];
	}
	_directCommand = NULL;
	_lastCycle = 0;
	_stateCounter=0;
	_goKickCouter=0;
	_compensateDir=0;
	_relinquishCounter = 0;
	_openKickCounter = 0;
}

void CChaseKickV2::plan(const CVisionModule* pVision)
{

	//�ս��뱾skill��Ϊ��ʼ״̬����BEGINNING����Ҫ��һЩ������
	if (pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1){
		setState(BEGINNING);
		_goKickCouter = 0;
		_compensateDir = 0;
		_relinquishCounter = 0;
		_openKickCounter = 0;
	}

	_directCommand = NULL;
	const MobileVisionT& ball = pVision->Ball();
	const int robotNum = task().executor;
	const int realNum = PlayInterface::Instance()->getRealIndexByNum(robotNum);
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const int playerFlag = task().player.flag;
	const int enemyLeaderNum = BestPlayer::Instance()->getTheirBestPlayer();          //�з�������в��
	const PlayerVisionT& enemyLeader = pVision->TheirPlayer(enemyLeaderNum);


	const CGeoPoint predict_posBall = BallSpeedModel::Instance()->posForTime(24, pVision);	//Ԥ���
	const CGeoPoint predict_posBallInAvoid = BallSpeedModel::Instance()->posForTime(60, pVision);	//Ԥ���
	const double AccelerateRatio = MAX_TRANSLATION_ACC / 650;                               //���ٶ���650��Ϊ��׼����߼��ٶȽ��ı����
	const double speedupGeneralFactor = 1/AccelerateRatio;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//2.��ǰ������Ϣ����Ҫ��ͼ����Ϣ
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const CVector meVel = pVision->OurRawPlayerSpeed(robotNum);
	const double meSpeed = meVel.mod();
	double finalKickDir = task().player.angle;									//���õ�������

	const CVector self2ball = predict_posBall - me.Pos();								//С����Ԥ��������
	const CVector self2rawball = ball.Pos() - me.Pos();									//С������ǰ������
	const double dist2predictball = self2ball.mod();									//С����Ԥ�������
	const double dist2ball = self2rawball.mod();										//С������ǰ�����
	const double reverse_finalDir = Utils::Normalize(finalKickDir + Param::Math::PI);		//����������ķ���
	const double dAngDiff = Utils::Normalize(self2ball.dir() - finalKickDir);
	const double dAngDiffRaw = Utils::Normalize(self2rawball.dir() - finalKickDir);		//С������ǰ�� - ������ �н�	TSB
	const CVector ballVel = ball.Vel();
	const double ballSpeed = ballVel.mod();
	const double ballVelDir = Utils::Normalize(ball.Vel().dir());
	bool isBallSpeedFast = (ballSpeed >= Ball_Moving_Fast_Speed) ? true : false;		//�����趨����ֵ�ж������Ƿ��㹻��
	
	double allowInfrontAngleBuffer = (dist2ball / (Param::Vehicle::V2::PLAYER_SIZE))*Param::Vehicle::V2::KICK_ANGLE < Param::Math::PI / 5.0 ?
		(dist2ball / (Param::Vehicle::V2::PLAYER_SIZE))*Param::Vehicle::V2::KICK_ANGLE : Param::Math::PI / 5.0;
	bool isBallInFront = fabs(Utils::Normalize(self2rawball.dir() - me.Dir())) < allowInfrontAngleBuffer
		&& dist2ball < (2.5*Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE);

	//�����ź�
	double isSensored = RobotSensor::Instance()->IsInfoValid(robotNum) && RobotSensor::Instance()->IsInfraredOn(robotNum);	//�Ƿ��м�⵽����
	//ͼ���Ӿ�
	const double dAngleMeDir2FinalKick = fabs(Utils::Normalize(me.Dir() - finalKickDir));				//��ǰ���������������н�
	const double dAngleMeBall2BallVelDir = fabs(Utils::Normalize(self2rawball.dir() - ballVelDir));	//�������������߼н�
	const double dAngleMeBall2MeDir = fabs(Utils::Normalize(self2rawball.dir() - me.Dir()));		//��������С������н�
	const double dAngleBall2FinalKick = fabs(Utils::Normalize(self2rawball.dir() - finalKickDir));
	const double dAngleFinalKick2BallVelDir = fabs(Utils::Normalize(finalKickDir - ballVelDir));
	const double antiKickDir = Utils::Normalize(finalKickDir + Param::Math::PI);
	const CGeoPoint myPos = me.Pos();
	const CGeoLine self2targetLine = CGeoLine(myPos, myPos + Utils::Polar2Vector(800, Utils::Normalize(finalKickDir)));			//С��������Ŀ����ֱ��
	const CGeoSegment self2targetSeg = CGeoSegment(myPos, myPos + Utils::Polar2Vector(800, Utils::Normalize(finalKickDir)));
	const CGeoLine ballMovingLine = CGeoLine(ball.Pos(), ball.Pos() + Utils::Polar2Vector(800, Utils::Normalize(ballVelDir)));		//����ֱ��
	const CGeoSegment ballMovingSeg = CGeoSegment(ball.Pos(), ball.Pos() + Utils::Polar2Vector(800, Utils::Normalize(ballVelDir)));
	const CGeoLineLineIntersection self2targetLine_ballMovingLine = CGeoLineLineIntersection(self2targetLine, ballMovingLine);
	CGeoPoint self2targetLine_ballMovingLine_secPos = predict_posBall;
	if (self2targetLine_ballMovingLine.Intersectant()){
		self2targetLine_ballMovingLine_secPos = self2targetLine_ballMovingLine.IntersectPoint();	//С����Ŀ������ߺ������ߵĽ���
	}
	
	const bool isIntersectionPosOnself2targetSeg = self2targetSeg.IsPointOnLineOnSegment(self2targetLine_ballMovingLine_secPos);	//������С��Ŀ����߶�
	const bool isIntersectionPosOnBallMovingSeg = ballMovingSeg.IsPointOnLineOnSegment(self2targetLine_ballMovingLine_secPos);		//�����������߶�
	const CGeoPoint ballProj = self2targetLine.projection(ball.Pos());
	const bool ballOnTargetSeg = self2targetSeg.IsPointOnLineOnSegment(ballProj);

	CGeoPoint kickPos = ball.Pos() + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER + 0.2, antiKickDir);
	CGeoPoint goalPos1 = CGeoPoint(Param::Field::PITCH_LENGTH / 2, -40);
	CGeoPoint goalPos2 = CGeoPoint(Param::Field::PITCH_LENGTH / 2, 40);
	const CGeoSegment goalSeg = CGeoSegment(goalPos1, goalPos2);
	const CGeoLine	goalLine = CGeoLine(goalPos1, goalPos2);
	CGeoLineLineIntersection goalLine_ballVel = CGeoLineLineIntersection(goalLine, ballMovingLine);
	bool isBallVelOnGoalLine = false;
	if (goalLine_ballVel.Intersectant()){
		CGeoPoint goalLine_ballVel_secPos = goalLine_ballVel.IntersectPoint();
		if (goalSeg.IsPointOnLineOnSegment(goalLine_ballVel_secPos)){
			isBallVelOnGoalLine = true;
		}
	}
	const CGeoPoint projMe = ballMovingSeg.projection(me.Pos());					//С�������ƶ����ϵ�ͶӰ��
	double projDist = projMe.dist(me.Pos());										//С����ͶӰ��ľ���
	const double ball2projDist = projMe.dist(ball.Pos());							//ͶӰ�㵽��ľ���
	const bool meOnBallMovingSeg = ballMovingSeg.IsPointOnLineOnSegment(projMe);	//ͶӰ���Ƿ��������߶�����

	const double dAngeMeVel2BallVel = Utils::Normalize(me.Vel().dir() - ballVelDir);			//С���ٶȷ�������ٷ���ļн�
	const double dSpeedMe2Ball = fabs(ballSpeed - me.Vel().mod()*cos(dAngeMeVel2BallVel));	//���������߷��������ٶ�
	bool isToThierGoal = faceTheirGoal(pVision, robotNum);
	bool isInBackField = me.Pos().x() < -150;

	const CGeoLine& ballVelLine = CGeoLine(ball.RawPos(), ball.Vel().dir());
	CGeoPoint projPoint = ballVelLine.projection(me.Pos());
	double me2projDist = projPoint.dist(me.Pos());										//The distance between the car and the projection

	/*
	* ���ڼ��ٵ��жϣ���ֹ��ײ
	*/
		CGeoLine meToBallLine = CGeoLine(myPos, predict_posBallInAvoid);        //�˴������Ԥ���
		CGeoSegment meToBallSegment = CGeoSegment(myPos, predict_posBallInAvoid);
		CGeoSegment meToBallSegmentProlong = CGeoSegment(myPos, myPos+Utils::Polar2Vector(1000,(predict_posBall-myPos).dir()));
		CGeoPoint enemyProjPos = meToBallLine.projection(enemyLeader.Pos());
		double enemyProjDist = (enemyLeader.Pos() - enemyProjPos).mod();
		bool isEnemyOnSegment = meToBallSegment.IsPointOnLineOnSegment(enemyProjPos);
		bool isEnemyOnSegmentProlong = meToBallSegmentProlong.IsPointOnLineOnSegment(enemyProjPos);

		bool isNeedSpeedDown = ((enemyLeader.Pos() - myPos).mod()<Param::Vehicle::V2::PLAYER_SIZE * 13&&SPEED_DOWN_MODE)
			&&(isEnemyOnSegment&&me.Vel().mod() > 200 && enemyProjDist < Param::Vehicle::V2::PLAYER_SIZE * 3 //���˴��������߸������ҽϽ�
			|| !isEnemyOnSegment&&isEnemyOnSegmentProlong&&enemyProjDist < Param::Vehicle::V2::PLAYER_SIZE * 5 && me.Vel().mod()>230 //���˴����������ϣ��복��Զ�ľ��룬Ȼ���ҷ��ٶȽϴ�
			|| (enemyLeader.Pos() - myPos).mod() < Param::Vehicle::V2::PLAYER_SIZE * 2 && isEnemyOnSegmentProlong &&me.Vel().mod()>170);//�������������ϣ�ͬʱ�복�ǳ������ҷ��ٶ�һ���

	


	//��������ǶȲ���
	if (ballSpeed > 80 && state() == SPEED_UP&&fabs(ballVelDir) > Param::Math::PI / 6)
	{
		if (verBos)
		{
			cout << "compensate" << endl;
		}
		_compensateDir = -Utils::Sign(ballVelDir)*Param::Math::PI * 8 / 180;
	}

	//if (fabs(ballVelDir)>Param::Math::PI/6){
	//	finalKickDir=finalKickDir-ballVelDir/3;
	//	GDebugEngine::Instance()->gui_debug_line(me.Pos(),me.Pos()+Utils::Polar2Vector(800,Utils::Normalize(finalKickDir)),COLOR_BLACK);
	//}



	bool isCanDirectKick = false;
	//������Ϣ������û��
	if (isSensored){
		if (dAngleMeDir2FinalKick < Param::Math::PI / 10){
			isCanDirectKick = true;
		}
	}
	//ͼ����Ϣ������
	double go_kick_factor = self2rawball.mod() / GO_KICK_BALL_CRITICAL_DIST;
	go_kick_factor = go_kick_factor > 1.0 ? 1.0 : go_kick_factor;	//�Ƕȿ���,����
	go_kick_factor = go_kick_factor < 0.5 ? 0.5 : go_kick_factor;	//�Ƕȿ���,����
	double DirectKickAllowAngle = go_kick_factor*Param::Vehicle::V2::KICK_ANGLE;
	//����٣�����ԭʼ����Ϣ
	if (fabs(Utils::Normalize(self2rawball.dir() - me.Dir())) </* 1.25**/DirectKickAllowAngle
		&& dAngleMeDir2FinalKick < Param::Math::PI / 35 //HAVE DONE
		&& self2rawball.mod() <= GO_KICK_BALL_CRITICAL_DIST){	//��������ǰ�� ��  С���ѳ���Ŀ�귽��
		isCanDirectKick = true;
	}
	if (fabs(Utils::Normalize(self2rawball.dir() - me.Dir())) </* 1.25**/DirectKickAllowAngle
		&& dAngleMeDir2FinalKick < Param::Math::PI / 25
		&& dAngleFinalKick2BallVelDir < Param::Math::PI / 20 && isBallVelOnGoalLine
		&& (dAngleMeBall2BallVelDir < Param::Math::PI / 15
		|| dAngleMeBall2BallVelDir > 14 * Param::Math::PI / 15)){		//���ٷ����䷴���� �� Ŀ�������� ��һ��
		isCanDirectKick = true;
	}

	bool is_ball_just_front = fabs(Utils::Normalize(self2rawball.dir() - me.Dir())) < Param::Vehicle::V2::KICK_ANGLE
		&& self2rawball.mod() < 2.5*Param::Vehicle::V2::PLAYER_SIZE;

	isCanDirectKick = isCanDirectKick || is_ball_just_front;
	isBallInFront = isBallInFront || is_ball_just_front;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//3.Conditions definition
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool allow_follow = dist2ball < FOLLOWBALL_CRITICAL_DIST;
	bool allow_gokick = isCanDirectKick;
	bool need_rush = dist2ball > FOLLOWBALL_CRITICAL_DIST + 30;


	//�Ƕ������е����⣬����Ƕ�ȥ����
	bool need_follow = dist2ball > GO_KICK_BALL_CRITICAL_DIST + 15 || fabs(Utils::Normalize(self2rawball.dir() - me.Dir())) > Param::Math::PI / 2.0/*2*Param::Vehicle::V2::KICK_ANGLE*/;


	bool allow_touch_shoot = (fabs(dAngDiffRaw) <= Param::Math::PI / 2.0)
		&& (fabs(Utils::Normalize(ballVelDir - Utils::Normalize(self2rawball.dir() + Param::Math::PI))) < Param::Math::PI / 2.5)
		&& (ballSpeed > 30) && ball.Pos().x() > me.Pos().x() + 8 && fabs(ballVelDir) > Param::Math::PI / 1.8;

	//bool allow_touch_shoot=false;

	bool isReached;	//���Ƿ�ϵ������߻����ұ�
	bool notReached;
	int  isLeft = -1; //��Ҫ�ϵ��ķ���
	//cout<<ballVelDir<<endl;
	//cout<<isLeft<<endl;
	if (sin(ballVelDir)*Param::Field::PITCH_LENGTH / 2 < ball.Pos().x()*sin(ballVelDir) - ball.Pos().y()*cos(ballVelDir)){
		isLeft = 1;
	}
	if (isLeft == 1){
		isReached = kickPos.y() > me.Pos().y() + Left_Reach_Allowance;
	}
	else{
		isReached = kickPos.y()<me.Pos().y() - Right_Reach_Allowance;
	}
	if (isLeft == 1){
		notReached = kickPos.y()<me.Pos().y();
	}
	else{
		notReached = kickPos.y()>me.Pos().y();
	}
	bool badAngle = (fabs(ballVelDir)>Param::Math::PI * 25 / 180 && dAngleFinalKick2BallVelDir > Param::Math::PI * 15 / 180
		|| dAngleFinalKick2BallVelDir > Param::Math::PI * 25 / 180) && (!isBallVelOnGoalLine); 
	bool is_fast_ball = ballSpeed > Ball_Moving_Fast_Speed;

	bool is_long_dist = self2rawball.mod() > 250;
	bool need_speed_up = badAngle&&notReached&&is_fast_ball||is_long_dist;
	
	bool need_wait_ball = badAngle&&isReached&&dist2ball > 45 && (kickPos.x() - me.Pos().x()) / fabs(kickPos.y() - me.Pos().y())<1 && ball.Vel().mod()>220; //HAVE DONE
	bool wait_follow = isReached && (kickPos.x() - me.Pos().x()) / fabs(kickPos.y() - me.Pos().y()) > 1.2
		|| dAngleBall2FinalKick < Param::Math::PI / 6
		|| fabs(Utils::Normalize(me.Vel().dir() - ballVelDir))<Param::Math::PI * 45 / 180 && meSpeed>30
		|| ballSpeed<meSpeed*cos(Utils::Normalize(me.Vel().dir() - ballVelDir)) + 50
		|| notReached;

	bool bigAngle = fabs(ballVelDir)>Param::Math::PI * 120 / 180 || ball.Pos().x()<me.Pos().x() + 3;   //kickpos
	bool needGetBall = !is_fast_ball&&dist2ball<350||bigAngle;   //IMTODO

	bool isCrossBall = fabs(ballVelDir)>Param::Math::PI * 75 / 180 && fabs(ballVelDir) < Param::Math::PI * 115 / 180;         //�ж��Ƿ�Ϊ�������� TODO
	bool isVerticalBall = fabs(ballVelDir)<Param::Math::PI * 25 / 180 || fabs(ballVelDir) > Param::Math::PI * 155 / 180;

	double speedupball_factorX = (ball.Vel().x() - me.Vel().x()) / 10;
	double speedupball_factorY = (ball.Vel().y() - me.Vel().y()) / 10;
	bool isNeedroughstart = 0;//TODO
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//4.����״̬��ά��
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*if (ballSpeed>=Ball_Moving_Fast_Speed)
	{*/
	int new_state = state();
	int old_state;
	{
		old_state = new_state;
		switch (old_state) {
		case BEGINNING:			// ֱ������ GOTO
			new_state = RUSH_TO_BALL;
			if (needGetBall){
				new_state = GET_BALL;
			}
			else{
				if (need_speed_up){
					new_state = SPEED_UP;
				}
				else if (need_wait_ball){
					new_state = WAIT_BALL;
				}
			}
			break;
		case RUSH_TO_BALL:
			if (need_speed_up){
				new_state = SPEED_UP;
			}
			else if (need_wait_ball)
			{
				new_state = WAIT_BALL;
			}
			else if (allow_follow) {
				new_state = FOLLOW_BALL;
			}
			else if (needGetBall){
				new_state = GET_BALL;
			}
			else if (isNeedSpeedDown){
				new_state = SPEED_DOWN;
			}
			break;
		case FOLLOW_BALL:
			if (allow_gokick) {
				new_state = GO_KICK_BALL;
			}
			else if (need_speed_up)
			{
				new_state = SPEED_UP;
			}
			else if (need_rush){
				new_state = RUSH_TO_BALL;
			}
			else if (needGetBall){
				new_state = GET_BALL;
			}
			break;
		case GO_KICK_BALL:
			_goKickCouter++;
			if (need_follow){
				new_state = FOLLOW_BALL;
			}
			if (_goKickCouter == 40){               //NEWTODO
				new_state = SPEED_UP;
				setState(SPEED_UP);
				_stateCounter = State_Counter_Num;
				_goKickCouter = 0;
			}
			break;
		case SPEED_UP:
			if (isReached){
				new_state = FOLLOW_BALL;
			}
			else if (allow_gokick&&isReached){
				new_state = GO_KICK_BALL;
			}
			else if (!is_fast_ball){
				new_state = RUSH_TO_BALL;
			}
			else if (needGetBall){
				new_state = GET_BALL;
			}
			else if (isNeedSpeedDown){
				new_state = SPEED_DOWN;
			}
			break;
		case WAIT_BALL:
			if (allow_gokick)
			{
				new_state = GO_KICK_BALL;
			}
			else if (wait_follow)
			{
				new_state = FOLLOW_BALL;
			}
			else if (need_speed_up || wait_follow)
			{
				new_state = SPEED_UP;
			}
			else if (needGetBall){
				new_state = GET_BALL;
			}
			break;
		case GET_BALL:
			if (!needGetBall){
				new_state = FOLLOW_BALL;
			}
			// new add
			if (me2projDist < 20 && meOnBallMovingSeg){
				new_state = GO_KICK_BALL;
			}
			break;
		/*case RELINQUISH_BALL:
			if (ball.Pos().x() < 200){
				_relinquishCounter = 0;
				new_state = RUSH_TO_BALL;
			}
			break;*/
		case SPEED_DOWN:
			if (meSpeed < 50&&(enemyLeader.Pos()-myPos).mod()>Param::Vehicle::V2::PLAYER_SIZE*2){//TODO
				new_state = RUSH_TO_BALL;
			}
			break;
		default:
			new_state = RUSH_TO_BALL;
			break;
		}
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
			if (new_state == state()){
				_stateCounter = min(State_Counter_Num, _stateCounter + 1);
			}
			else{
				_stateCounter = max(0, _stateCounter - 1);
			}
		}
	}
	//��¼��ǰ����
	_lastCycle = pVision->Cycle();
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//5.���ݵ�ǰ��Ϣ�ۺϾ������Ԥ��λ��: ����Խ������������ԽС��Ԥ����Ӧ��֮ԽС//��Ԥ��λ�ü�������
	// TODO��TODO  TODO
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	double predict_factor = 1.0;	//��λ��Ԥ������
	double ballVel_factor = 1.0;	//����Ӱ������
	const CVector rawBall2predictBall = predict_posBall - ball.Pos();
	if (meOnBallMovingSeg){	//С��ͶӰ�������ƶ������棬�������Ѿ�������ǰ
		predict_factor = sqrt(self2rawball.mod() / 150)*sqrt(sqrt(ballSpeed / 250));
		predict_factor *= sqrt(dSpeedMe2Ball / 100);

	}
	else {
		predict_factor = sqrt(ballSpeed / 300)*sqrt(sqrt(self2rawball.mod() / 150));
		predict_factor *= sqrt(dSpeedMe2Ball / 150);
	}
	predict_factor -= 0.25;
	double maxFactor = 0.75, minFactor = 0.25;
	if (fabs(ballVelDir) > Param::Math::PI * 65 / 180)
	{
		predict_factor *= 1.25;
	}
	predict_factor = predict_factor > maxFactor ? maxFactor : predict_factor;
	predict_factor = predict_factor< minFactor ? minFactor : predict_factor;
	CVector extra_ball_vel = rawBall2predictBall * predict_factor;
	//??
	if (fabs(Utils::Normalize(extra_ball_vel.dir() - ball.Vel().dir())) > Param::Math::PI / 3.0) {
		extra_ball_vel = extra_ball_vel * (-1.0);
	}
	CGeoPoint real_predict_ballPos = ball.Pos() + extra_ball_vel*1.2;
	if (!is_fast_ball) {
		real_predict_ballPos = ball.Pos();
	}
	//some tempt variables
	CGeoPoint proj_temp;
	CVector tmp;

	double speedUpDistanceX = 0, speedUpDistanceY = 0;
	CVector speedUpVel;

	CGeoPoint wait_ballPos = ball.Pos() + ball.Vel() / 2;
	double wait_factor = 1;
	TaskT chase_kick_task(task());
	double gokickFactor = 1.0;
	double myVelSpeedRelative2Final = me.Vel().mod()*cos(Utils::Normalize(me.Vel().dir() - finalKickDir));
	//cout<<"meballdist"<<ball.Pos().x()-me.Pos().x()<<endl;
	if (verBos) cout << "chaseState:" << state() << endl;
	switch (state())
	{
		//rush��speed up��wait ball�����м�״̬���Գ�λ�ý��дֵ����ó���Ŵ���һ�������λ�ã�
		//rush�����ٷ���Ƚ�����ֱ�ӳ������ţ�speed up�����ٷ���ϴ��ҳ������������ϣ�wait up�����ٷ���ϴ󣬳�����������
		//follow ball ��go kick ball���Ǹ�������ٶȽ��е������ٵ�Ԥ�����ӣ��Ӷ��ó���ȷ������������ţ�
	case RUSH_TO_BALL:
		if (testOn){
			GDebugEngine::Instance()->gui_debug_msg(real_predict_ballPos, "RUSH_TO_BALL");
			GDebugEngine::Instance()->gui_debug_x(real_predict_ballPos, COLOR_CYAN);
		}
		if (fabs(dAngDiffRaw) <= Param::Math::PI / 2.0){
			//�趨�ܵĵ㣬�����Ǳ���
			chase_kick_task.player.pos = real_predict_ballPos + Utils::Polar2Vector(3 * Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE, reverse_finalDir);
			//add front vel
			if (myVelSpeedRelative2Final < 50) {
				chase_kick_task.player.vel = chase_kick_task.player.vel + Utils::Polar2Vector(50, Utils::Normalize(finalKickDir));
			}
			else if (myVelSpeedRelative2Final<75) {
				chase_kick_task.player.vel = chase_kick_task.player.vel + Utils::Polar2Vector(30 + myVelSpeedRelative2Final, Utils::Normalize(finalKickDir));
			}
			else {
				chase_kick_task.player.vel = chase_kick_task.player.vel + Utils::Polar2Vector(25 + myVelSpeedRelative2Final, Utils::Normalize(finalKickDir));
			}
			if (chase_kick_task.player.vel.mod() > speed_factor * MaxSpeed){	//����
				chase_kick_task.player.vel = chase_kick_task.player.vel * (speed_factor * MaxSpeed / chase_kick_task.player.vel.mod());
			}
		}
		else{	//���Ǳ��� ��ӳ��󷽹�������������
			double nowdir = Utils::Normalize(self2rawball.dir() + Param::Math::PI);
			int sign = Utils::Normalize((nowdir - finalKickDir)) > 0 ? 1 : -1;
			nowdir = Utils::Normalize(nowdir + sign*Param::Math::PI / 2.0);
			chase_kick_task.player.pos = ball.Pos() + Utils::Polar2Vector(1.5*Param::Field::MAX_PLAYER_SIZE, nowdir);
			chase_kick_task.player.vel = CVector(0, 0);
		}
		break;
		
	case SPEED_UP:
		//speed_up״̬�����������ŽǶȣ���ֹճ��Ϊ֮���follow��kick�γ����õ�����λ�úͽǶ�
		//speedUpDistance��speedUpVelӰ�������ǶȵĴ�С��
		//speedUpDistanceYӦ�������ģ��泵��yֵ�Ĳ�ֵԽ��ԽС��Ӱ�������ǶȾ��루�򳵺�����룩��������أ�
		//speedUpDistanceX��Ӱ����������룬�����ٷ�����أ���Ƕ�ʱӦΪ��ֵ��
		//speedUpVel����Ϊ���ٷ���������
		//cout<<"speed Up"<<endl;
		if (testOn){
			GDebugEngine::Instance()->gui_debug_msg(real_predict_ballPos, "SPEED_UP");
			GDebugEngine::Instance()->gui_debug_x(real_predict_ballPos, COLOR_CYAN);
		}
		if (isVerticalBall){
			chase_kick_task.player.max_acceleration = MAX_TRANSLATION_ACC;
			if (testOn){
				//cout << "Crazy Vertical speed up"<< endl;
			}
			/*speedUpDistanceY = fabs(kickPos.y() - me.Pos().y()) + crossWiseFactor[realNum] * Param::Vehicle::V2::PLAYER_SIZE + max((ballSpeed - 100)*sin(fabs(max(ballVelDir - Param::Math::PI * 35 / 180, 0)))*0.18, 0);*/
			speedUpDistanceY = fieldFactor*fabs(kickPos.y() - me.Pos().y()) * crossWiseFactor[realNum-1] * Param::Vehicle::V2::PLAYER_SIZE + max(ballSpeed - 80, 0)*max(sin(fabs(ballVelDir) - Param::Math::PI * 8 / 180), 0)*0.28;
			speedUpDistanceY += speedupball_factorY*0.15;
			speedUpDistanceY = min(speedUpDistanceY, 50);
			speedUpDistanceY = max(speedUpDistanceY, 0);
			//speedUpDistanceX = 3 - sin(fabs(ballVelDir)) * 10 + kickPos.x() - me.Pos().x()+verticalWiseFactor[realNum] *Param::Vehicle::V2::PLAYER_SIZE;
			speedUpDistanceX = fieldFactor*fabs(kickPos.x() - me.Pos().x()) * verticalWiseFactor[realNum-1] * Param::Vehicle::V2::PLAYER_SIZE + cos(fabs(ballVelDir))* max(ballSpeed - 100, 0) * 0.25;
			speedUpDistanceX += speedupball_factorX*0.25;
			speedUpDistanceX = min(speedUpDistanceX, 17); //20
			speedUpDistanceX = max(speedUpDistanceX, 0);
			//speedUpVel=ballVel/ballSpeed*((ballSpeed-80)*1.8+(fabs(ballVelDir)/3.14*180-20)*2);
			speedUpVel = ball.Vel() / ballSpeed*(ballSpeed + dist2ball + (50 - (meSpeed - ballSpeed)));
			if (fabs(ballVelDir) > Param::Math::PI * 65 / 180){
				double diffX = fabs(kickPos.x() - me.Pos().x());
				if (meSpeed<ballSpeed&&diffX>25)
				{
					speedUpDistanceX = (me.Pos().x() - kickPos.x()) / 2 + 5;
				}
				else{
					speedUpDistanceX = -4;
				}
				speedUpDistanceY += 8;
			}
			if (me.Pos().x() - kickPos.x() > 5){
				speedUpDistanceX = -5;
			}
			speedUpDistanceX *= pow(speedupGeneralFactor, 1.9);
			speedUpDistanceY *= pow(speedupGeneralFactor, 1.6);//TODO
		}
		else if(isCrossBall){
			if (testOn){
				//cout << "Crazy cross speed up" << endl;
			}
			/*speedUpDistanceY = 0.8*fabs(kickPos.y() - me.Pos().y()) + crossWiseFactor[realNum] * Param::Vehicle::V2::PLAYER_SIZE + max((ballSpeed - 100)*sin(fabs(max(ballVelDir - Param::Math::PI * 35 / 180, 0)))*0.18, 0);*/
			speedUpDistanceY = fieldFactor*fabs(kickPos.y() - me.Pos().y()) * crossWiseFactor[realNum-1] * Param::Vehicle::V2::PLAYER_SIZE + max(ballSpeed - 80, 0)*max(sin(fabs(ballVelDir) - Param::Math::PI * 2 / 180), 0)*0.40;
			speedUpDistanceY += speedupball_factorY*0.35;
			speedUpDistanceY = min(speedUpDistanceY, 58);
			speedUpDistanceY = max(speedUpDistanceY, 0);

			//speedUpDistanceX = 3 - sin(fabs(ballVelDir)) * 10 + kickPos.x() - me.Pos().x() + verticalWiseFactor[realNum] * Param::Vehicle::V2::PLAYER_SIZE;
			speedUpDistanceX = fieldFactor*fabs(kickPos.x() - me.Pos().x()) * verticalWiseFactor[realNum-1] * Param::Vehicle::V2::PLAYER_SIZE + cos(fabs(ballVelDir+Param::Math::PI * 10 / 180))* max(ballSpeed - 100, 0) * 0.1;
			speedUpDistanceX += speedupball_factorX*0.1;
			speedUpDistanceX = min(speedUpDistanceX, 10);
			speedUpDistanceX = max(speedUpDistanceX, 0);
			//speedUpVel=ballVel/ballSpeed*((ballSpeed-80)*1.8+(fabs(ballVelDir)/3.14*180-20)*2);
			speedUpVel = ball.Vel() / ballSpeed*(ballSpeed + dist2ball + (50 - (meSpeed - ballSpeed)));
			if (fabs(ballVelDir) > Param::Math::PI * 65 / 180){
				double diffX = fabs(kickPos.x() - me.Pos().x());
				if (meSpeed<ballSpeed&&diffX>25)
				{
					speedUpDistanceX = (me.Pos().x() - kickPos.x()) / 2 + 5;
				}
				else{
					speedUpDistanceX = -3;
				}
				speedUpDistanceY += 8;
			}
			if (me.Pos().x() - kickPos.x() > 5){
				speedUpDistanceX = -9;
			}
		}

		else{
			if (testOn){
				//cout << "Crazy abnormal speed up" << endl;
			}
			/*speedUpDistanceY = 0.8*fabs(kickPos.y() - me.Pos().y()) + crossWiseFactor[realNum] * Param::Vehicle::V2::PLAYER_SIZE + max((ballSpeed - 100)*sin(fabs(max(ballVelDir - Param::Math::PI * 35 / 180, 0)))*0.3, 0);*/
			speedUpDistanceY = fieldFactor*fabs(kickPos.y() - me.Pos().y()) * crossWiseFactor[realNum-1] * Param::Vehicle::V2::PLAYER_SIZE + max(ballSpeed - 80, 0)*max(sin(fabs(ballVelDir) - Param::Math::PI * 10 / 180), 0)*0.35;
			speedUpDistanceY += speedupball_factorY*0.3;
			speedUpDistanceY = min(speedUpDistanceY, 56);
			speedUpDistanceY = max(speedUpDistanceY, 0);
			//speedUpDistanceX = 3 - sin(fabs(ballVelDir)) * 10 + kickPos.x() - me.Pos().x() + verticalWiseFactor[realNum] * Param::Vehicle::V2::PLAYER_SIZE;
            speedUpDistanceX = fieldFactor*fabs(kickPos.x() - me.Pos().x()) * verticalWiseFactor[realNum-1] * Param::Vehicle::V2::PLAYER_SIZE + cos(fabs(ballVelDir+Param::Math::PI* 10/180))* max(ballSpeed - 100, 0) * 0.15;
			speedUpDistanceX += speedupball_factorX*0.12;
			speedUpDistanceX = min(speedUpDistanceX, 14); //20
			speedUpDistanceX = max(speedUpDistanceX, 0);
			//speedUpVel=ballVel/ballSpeed*((ballSpeed-80)*1.8+(fabs(ballVelDir)/3.14*180-20)*2);
			speedUpVel = ball.Vel() / ballSpeed*(ballSpeed + dist2ball + (50 - (meSpeed - ballSpeed)));
			if (fabs(ballVelDir) > Param::Math::PI * 65 / 180){
				double diffX = fabs(kickPos.x() - me.Pos().x());
				if (meSpeed<ballSpeed&&diffX>25)
				{
					speedUpDistanceX = (me.Pos().x() - kickPos.x()) / 2 + 5;
				}
				else{
					speedUpDistanceX = -4;
				}
				speedUpDistanceY += 8;
			}
			if (me.Pos().x() - kickPos.x() > 5){
				speedUpDistanceX = -7;
			}
		}

		if (testOn){
			//cout << speedUpDistanceX << endl;
			//cout << speedUpDistanceY << endl;
		}
		chase_kick_task.player.pos = CGeoPoint(kickPos.x() + speedUpDistanceX, kickPos.y() - isLeft*speedUpDistanceY);
		chase_kick_task.player.vel = speedUpVel;
		chase_kick_task.player.angle = finalKickDir;
		//cout<<"KickDir"<<finalKickDir<<endl;

		break;
	case WAIT_BALL:
		//���������ӵ�״̬����������С���ܵ���һ���ʺϽ���follow״̬��һ���м�״̬
		//����˼����Ǳ�֤��Xλ�ò�������ͬʱ�����˶��켣�ӽ�Ϊһ��ֱ�ߣ��Ժ��ʵ��ٶ��˶���������·�
		//��С�����õĵ�������ǰλ��+����*ϵ��Ϊ��׼�㣬��ϸ��
		//��������Xλ�õĲ�����wait_factor�������������ŷ���ļн��Լ����ٽǶȣ�
		//�����������ŷ���ļн�ԽС��˵��Խ�ӽ������������״̬����ʱ����Ԥ��ҲҪ��С
		//wait_factor��������������Xλ�ò����Ҫ���أ�����ʵʱ����Xλ�ò������зּ����ã�����Խ����ԽС
		//����speedUpDistanceX��speedUpDistanceY�Ƿ���speed up��΢����ʽ���Գ�λ��΢��
		//cout<<"wait ball"<<endl;
		if (testOn){
			GDebugEngine::Instance()->gui_debug_msg(real_predict_ballPos, "WAIT_BALL");
			GDebugEngine::Instance()->gui_debug_x(real_predict_ballPos, COLOR_CYAN);
		}
		TouchKickPos::Instance()->GenerateTouchKickPos(pVision, robotNum, finalKickDir);
		chase_kick_task.player.pos = TouchKickPos::Instance()->getKickPos();
		chase_kick_task.player.vel = ball.Vel();
		chase_kick_task.player.angle = finalKickDir;
		break;
	case FOLLOW_BALL:
		if (testOn){
			GDebugEngine::Instance()->gui_debug_x(real_predict_ballPos, COLOR_BLUE);
			GDebugEngine::Instance()->gui_debug_msg(ball.Pos(), "FOLLOW_BALL");
		}
		projDist = (projDist < Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE + 3) ?
			Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE + 3 : projDist - 3;
		projDist = (projDist > 1.2*Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE) ?
			1.2*Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE : projDist - 2;

		chase_kick_task.player.pos = real_predict_ballPos + Utils::Polar2Vector(projDist, reverse_finalDir);

		ballVel_factor = sqrt(ball2projDist / (Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE));
		ballVel_factor = ballVel_factor > 1.25 ? 1.25 : ballVel_factor;
		if (ballMovingSeg.IsPointOnLineOnSegment(projMe)){		//С���Ѿ�׷����
			if (meSpeed > ballSpeed + 50)
			{
				ballVel_factor *= -0.5;
			}
			else{
				ballVel_factor *= 0.65;
			}
		}
		else{	//�������� & ��ǰ�ٶ�
			ballVel_factor *= 0.8;
		}
		tmp = ball.Vel()*(ballVel_factor);

		if (myVelSpeedRelative2Final < 15) {
			tmp = tmp + Utils::Polar2Vector(15, Utils::Normalize(finalKickDir));
		}
		else if (myVelSpeedRelative2Final<30) {
			tmp = tmp + Utils::Polar2Vector(15 + myVelSpeedRelative2Final, Utils::Normalize(finalKickDir));
		}
		else {
			tmp = tmp + Utils::Polar2Vector(45, Utils::Normalize(finalKickDir));
		}
		if (tmp.mod() > speed_factor * MaxSpeed){	//����
			chase_kick_task.player.vel = tmp * (speed_factor * MaxSpeed / tmp.mod());
		}
		else{
			chase_kick_task.player.vel = tmp;
		}
		break;

	case GO_KICK_BALL:
		//cout<<"GO_KICK_BALL"<<endl;
		//cout<<"ball"<<isBallInFront<<endl;
		if (testOn){
			GDebugEngine::Instance()->gui_debug_msg(real_predict_ballPos, "GO_KICK_BALL");
			GDebugEngine::Instance()->gui_debug_x(real_predict_ballPos, COLOR_CYAN);
		}
		gokickFactor = (dAngleMeBall2MeDir / Param::Vehicle::V2::KICK_ANGLE) * dist2ball / (2 * Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE);
		if (gokickFactor >= 0.75){
			gokickFactor = 0.75;
		}
		else if (gokickFactor < 0.75){
			if (meOnBallMovingSeg) {
				gokickFactor = 0.4;
			}
			else{
				gokickFactor = 0.7;
			}
		}
		if (!isBallInFront){
			gokickFactor = 1;
			chase_kick_task.player.pos = real_predict_ballPos + Utils::Polar2Vector(gokickFactor*Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER * 3, reverse_finalDir);
			chase_kick_task.player.flag |= PlayerStatus::DODGE_BALL;
		}
		else{
			chase_kick_task.player.pos = real_predict_ballPos
				+ Utils::Polar2Vector(gokickFactor*Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER * 2, reverse_finalDir);
			chase_kick_task.player.flag |= PlayerStatus::DODGE_BALL;
		}

		ballVel_factor = sqrt(ball2projDist / (Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE))*sqrt(ballSpeed / 100);
		ballVel_factor = ballVel_factor > 1.0 ? 1.0 : ballVel_factor;
		if (ballMovingSeg.IsPointOnLineOnSegment(projMe)){		//С���Ѿ�׷����
			if (meSpeed > ballSpeed + 20)
			{
				ballVel_factor *= -0.5;
			}
			else{
				ballVel_factor *= 0.5;
			}
		}
		else{	//�������� & ��ǰ�ٶ�
			if (isBallInFront)
				ballVel_factor *= 0.75;
			else
				ballVel_factor *= 1.25;

		}
		tmp = ball.Vel()*ballVel_factor;
		if (myVelSpeedRelative2Final < 25) {
			tmp = tmp + Utils::Polar2Vector(25, Utils::Normalize(finalKickDir));
		}
		else if (myVelSpeedRelative2Final<50) {
			tmp = tmp + Utils::Polar2Vector(25 + myVelSpeedRelative2Final, Utils::Normalize(finalKickDir));
		}
		else {
			tmp = tmp + Utils::Polar2Vector(75, Utils::Normalize(finalKickDir));
		}
		if (tmp.mod() > speed_factor * MaxSpeed){	//����
			chase_kick_task.player.vel = tmp*(speed_factor*MaxSpeed / tmp.mod());
		}
		else{
			chase_kick_task.player.vel = tmp;
		}
		break;
	/*case RELINQUISH_BALL:
		chase_kick_task.player.pos = CGeoPoint(0, 0);
		break;*/
	case GET_BALL:
		if (testOn){
			GDebugEngine::Instance()->gui_debug_msg(real_predict_ballPos, "GET_BALL");
			GDebugEngine::Instance()->gui_debug_x(real_predict_ballPos, COLOR_CYAN);
			//cout << "Getball" << endl;
		}	
			setSubTask(PlayerRole::makeItNoneTrajGetBall(task().executor,finalKickDir,CVector(0,0),task().player.flag | PlayerStatus::DODGE_BALL,-2));
			break;
	case SPEED_DOWN:
		chase_kick_task.player.pos = myPos;
		chase_kick_task.player.vel = speedUpVel;
		chase_kick_task.player.angle = finalKickDir;
		break;
	default:
		break;
	}

	/*bool needRelinquishBall = chase_kick_task.player.pos.x() > 350 && chase_kick_task.player.pos.y()<125 && chase_kick_task.player.pos.y()>-125 && me.Pos().x() > 200 || (me.Pos().x() > 340 && me.Pos().y()<130 && me.Pos().y()>-130);//��Ҫ�������ʱ������
	if (state() != RELINQUISH_BALL&&needRelinquishBall){
		_relinquishCounter++;
	}
	if (_relinquishCounter == 10){
		setState(RELINQUISH_BALL);
	}*/
	//�����
	//if( state() == FOLLOW_BALL||state()==WAIT_BALL){
	//	if( meOnBallMovingSeg && projDist < 2*Param::Vehicle::V2::PLAYER_SIZE && ballSpeed > 50 && (dAngleMeBall2BallVelDir < Param::Math::PI/4 || dAngleMeBall2BallVelDir > Param::Math::PI*3/4)){
	//		if( fabs(Utils::Normalize(finalKickDir-ballVelDir)) < Param::Math::PI/3.0 || fabs(Utils::Normalize(finalKickDir-ballVelDir)) > Param::Math::PI*2.0/3.0 ){
	//			chase_kick_task.player.pos = ball.Pos() + Utils::Polar2Vector(2.0*Param::Field::MAX_PLAYER_SIZE,Utils::Normalize((me.Pos()-projMe).dir()));
	//			chase_kick_task.player.vel = CVector(0.0,0.0);
	//		}else{
	//			chase_kick_task.player.pos = ball.Pos() + Utils::Polar2Vector(1.5*Param::Field::MAX_PLAYER_SIZE,Utils::Normalize((me.Pos()-projMe).dir()) );
	//			chase_kick_task.player.vel = CVector(0.0,0.0);
	//		}		

	//		if( dAngleMeBall2BallVelDir < Param::Math::PI/5 ){
	//			chase_kick_task.player.pos = chase_kick_task.player.pos + Utils::Polar2Vector(2.0*Param::Field::MAX_PLAYER_SIZE,Utils::Normalize((me.Pos()-projMe).dir()) );
	//		}
	//	}
	//	if( /*fabs(dAngDiffRaw) >= Param::Math::PI/3.0 &&*/ !isBallInFront && dist2ball < Param::Field::MAX_PLAYER_SIZE+5){
	//		double nowdir = Utils::Normalize(self2rawball.dir()+Param::Math::PI);
	//		int sign = Utils::Normalize((nowdir - finalKickDir))>0?1:-1;
	//		nowdir = Utils::Normalize(nowdir+sign*Param::Math::PI/2.0);

	//		chase_kick_task.player.pos = real_predict_ballPos + Utils::Polar2Vector(5.0*Param::Field::MAX_PLAYER_SIZE,nowdir);
	//		chase_kick_task.player.vel = CVector(0,0);
	//	}
	//}

	//chase_kick_task.player.rotvel = 0.0;

	/************************************************************************/
	/* 6.Touch Kick���жϼ���ȡ�ײ�skill											*/
	/************************************************************************/
	/*bool needDribbleBall = dist2ball<Param::Vehicle::V2::PLAYER_SIZE*2;
	if(needDribbleBall){
		DribbleStatus::Instance()->setDribbleCommand(robotNum, 2);
	}
	else{
		DribbleStatus::Instance()->setDribbleCommand(robotNum,0);
	}*/
	bool isCanOpenKick = false;
	// if (me.Pos().x() > 0){
	// 	isCanOpenKick = isToThierGoal  && fabs(dAngleMeDir2FinalKick)<Param::Math::PI * 15 / 180;
	// }
	// else{
	// 	isCanOpenKick = isToThierGoal && fabs(dAngleMeDir2FinalKick)<Param::Math::PI * 8 / 180;
	// }
	double chase_precise = Param::Math::PI * 7 / 180;
	if (me.Pos().x() > 0){
		if(ball.Vel().mod() < 80){
			chase_precise = Param::Math::PI * 5 / 180;
		}else{
			chase_precise = Param::Math::PI * 7 / 180; 
		}
	}else{
		chase_precise = Param::Math::PI * 5 / 180;
	}
	isCanOpenKick = isToThierGoal  && fabs(dAngleMeDir2FinalKick) < chase_precise;
	
	//cout << dAngleMeBall2MeDir /Param::Math::PI *180<< endl;
	//cout << dAngleMeDir2FinalKick / Param::Math::PI * 180 << endl;
	/*if (NormalPlayUtils::faceTheirGoal(pVision, robotNum, Param::Math::PI * 2 / 180)
		|| WorldModel::Instance()->KickDirArrived(pVision->Cycle(), finalKickDir, Param::Math::PI * 2 / 180, robotNum))
	{
		if (verBos)	{ cout << "chase-kickOk" << endl; }
		KickStatus::Instance()->setKick(robotNum, 1200);
	}*/
	/*if (NormalPlayUtils::faceTheirGoal(pVision, robotNum, Param::Math::PI * 2 / 180)
		|| WorldModel::Instance()->KickDirArrived(pVision->Cycle(), finalKickDir, Param::Math::PI * 2 / 180, robotNum))
	{
		cout << "Fuck" << endl;
		KickStatus::Instance()->setKick(robotNum, 1200);
	}*/
	if (isCanOpenKick){
		if (testOn){
			//printf("openKick \n");
		}
		// if (task().player.ischipkick)
		// {
		// 	KickStatus::Instance()->setChipKick(robotNum,1200);
		// }else{
		// 	KickStatus::Instance()->setKick(robotNum, 700);
		// }
		if (me.Pos().x() < -150) {//�����俪��
			double kp = me.Pos().dist(CGeoPoint(450, 0));
			KickStatus::Instance()->setChipKick(robotNum,kp/3.5);//250
		}
		else {
			double finalBallSpeed = 800 - pVision->OurPlayer(robotNum).Vel().mod();
			//cout << "setKickINCHASE" << endl;
			KickStatus::Instance()->setKick(robotNum, finalBallSpeed);
		}
	}
	if (state() != GET_BALL){
		//SubTask�ĵ���
		if (allow_touch_shoot) {						/// ��ʱ����������������
			chase_kick_task.player.ispass = false;
			chase_kick_task.player.angle = Utils::Normalize(finalKickDir);
			if (me.Pos().x() > 220){    //TODO need Compensate for bigangle
				if (me.Pos().y() > 170){
					if (testOn){
						//cout << "Compen" << endl;
					}
					finalKickDir = finalKickDir + Param::Math::PI * 15 / 180;
				}
				else if (me.Pos().y() < -170){
					if (testOn){
						//cout << "Compen" << endl;
					}

					finalKickDir = finalKickDir - Param::Math::PI * 15 / 180;
				}
			}
			//printf("touch \n");
			setSubTask(TaskFactoryV2::Instance()->TouchKick(chase_kick_task));
		}
		else {
			//printf("Goto\n");
			chase_kick_task.player.angle = Utils::Normalize(_compensateDir + chase_kick_task.player.angle);
			if (state() != SPEED_UP){
				chase_kick_task.player.max_acceleration = MAX_TRANSLATION_ACC;
			}
			chase_kick_task.player.pos = checkPointAvoidOurPenalty(pVision, chase_kick_task.player.pos);
			setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(chase_kick_task));
			//if (verBos) cout << "angle " << chase_kick_task.player.angle << endl;
		}
	}


	/*else if (state()!=RELINQUISH_BALL)
	{	
		//cout << "Slow Get Ball" << endl;
		if (ball.Pos().x() > 340 && ball.Pos().y()<130 && ball.Pos().y()>-130){
			setSubTask(PlayerRole::makeItGoto(task().executor, CGeoPoint(0, 0), 0, task().player.flag, 0));
		}
		else{
			setSubTask(PlayerRole::makeItNoneTrajGetBall(task().executor, finalKickDir, CVector(0, 0), task().player.flag, -2));
		}
	}*/
	CStatedTask::plan(pVision);
}

CPlayerCommand* CChaseKickV2::execute(const CVisionModule* pVision)
{
	if( subTask() ){
		return subTask()->execute(pVision);
	}
	if( _directCommand != NULL ){
		return _directCommand;
	}
	return NULL;
}

bool CChaseKickV2::faceTheirGoal(const CVisionModule* pVision, const int executor) {
	double dist = pVision->OurPlayer(executor).Pos().x() - Param::Field::PITCH_LENGTH / 2;
	double buffer = 0.02 * dist + 40;
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

	/*if (testOn) {
		GDebugEngine::Instance()->gui_debug_line(theirLeft, pVision->OurPlayer(executor).Pos());
		GDebugEngine::Instance()->gui_debug_line(theirRight, pVision->OurPlayer(executor).Pos());
		GDebugEngine::Instance()->gui_debug_line(pVision->OurPlayer(executor).Pos() + Utils::Polar2Vector(100, myDir), pVision->OurPlayer(executor).Pos());

		GDebugEngine::Instance()->gui_debug_line(theirLeft, pVision->OurPlayer(executor).RawPos());
		GDebugEngine::Instance()->gui_debug_line(theirRight, pVision->OurPlayer(executor).RawPos());
		GDebugEngine::Instance()->gui_debug_line(pVision->OurPlayer(executor).RawPos() + Utils::Polar2Vector(100, myRawDir), pVision->OurPlayer(executor).RawPos());
	}*/
	return (isToTheirGoal || isRawToTheirGoal);
}

CGeoPoint CChaseKickV2::checkPointAvoidOurPenalty(const CVisionModule* pVision, CGeoPoint targetPoint){
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

		/* �����ߺͽ����߽������������ */
		const CGeoPoint* points[3];
		points[0] = 0;
		points[1] = 0;
		points[2] = 0;

		/* �������м�ֱ���� */
		if (penaltyLineIntersection.Intersectant()) {
			const CGeoPoint& point = penaltyLineIntersection.IntersectPoint();
			if (point.y() <= rightPenaltySegmentEnd.y() && point.y() >= leftPenaltySegmentEnd.y()) {
				points[0] = &point;
			}
		}

		/* ����������Բ���� */
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


		/* �ۺϿ��ǽ��� �ҳ�����ʵ� */
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
			/* ����ƽ���� */
			double leftPostToBallDir = (*defendTarget - LEFT_GOAL_POST).dir();
			double rightPostToBallDir = (*defendTarget - RIGHT_GOAL_POST).dir();
			double defendDir = Utils::Normalize((leftPostToBallDir + rightPostToBallDir) / 2 + Math::PI);

			/* �Ƕ����� */
			if (defendDir > 30 / 180 * Math::PI)
				defendDir -= 10 / 180 * Math::PI;
			else if (defendDir < -30 / 180 * Math::PI)
				defendDir += 10 / 180 * Math::PI;
			recalcpoint = *defendTarget + Utils::Polar2Vector(Vehicle::V2::PLAYER_SIZE*2.5, defendDir);
		}
	}
	return recalcpoint;
}

