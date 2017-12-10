#include "ChaseKickV1.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "Factory.h"
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
#include "KickStatus.h"
#include "TouchKickPos.h"
#include "BallSpeedModel.h"
#include <GDebugEngine.h>
#include "DribbleStatus.h"

namespace
{
	//2.�켣�����㷨ʹ�ñ���
	int TRAJECTORY_METHORD = 1;

	enum {
		RUSH_TO_BALL = 1,	//���ٽӽ���
		WAIT_BALL = 2,		//�ȴ�����ǰ
		FOLLOW_BALL = 3,	//����������
		GO_KICK_BALL = 4	//��ǰ������
	};

	//״̬�л���ر���
	const double RUSH_TO_BALL_CRITICAL_DIST = 100;	//100cm
	const double FOLLOWBALL_CRITICAL_DIST = 50;		//50cm
	const double GO_KICK_BALL_CRITICAL_DIST = 2 * Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE;

	//4.����˥��ģ�͵Ĳ���
	const double STEP_TIME = 1.0 / Param::Vision::FRAME_RATE;

	//Ԥ�����
	const double MAX_TIME = 5; // �����ʱ��(/s)
	double CM_PREDICT_FACTOR = 1.5;
	const double Ball_Moving_Fast_Speed = 80;	//1m/s
												//
	const double speed_factor = 0.7;

}

CChaseKickV1::CChaseKickV1()
{
	{
		DECLARE_PARAM_READER_BEGIN(CGotoPositionV2)
		READ_PARAM(TRAJECTORY_METHORD)
			DECLARE_PARAM_READER_END
	}

	_directCommand = NULL;
	_lastCycle = 0;
}

void CChaseKickV1::plan(const CVisionModule* pVision)
{
	//�ս��뱾skill��Ϊ��ʼ״̬����BEGINNING����Ҫ��һЩ������
	if (pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1) {
		setState(BEGINNING);
	}

	//////////////////////////////////////////////////////////////////////////
	/*1.������˥��ģ��Ϊ���������е���ģ�⣺* ȱ����ֻ����ƽ������û�п���ת���� *
	��ȡxml�����õ�С���˶����������OmniAuto							*/
	//////////////////////////////////////////////////////////////////////////
	_directCommand = NULL;
	const MobileVisionT& ball = pVision->Ball();
	const int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const int playerFlag = task().player.flag;
	const bool needDribble = playerFlag & PlayerStatus::DRIBBLING;
	if (needDribble) {
		DribbleStatus::Instance()->setDribbleCommand(robotNum, 1);
	}
	const double KickPower = 9999;

	const CRobotCapability* robotCap = RobotCapFactory::Instance()->getRobotCap(pVision->Side(), robotNum);
	const double MaxSpeed = robotCap->maxSpeed(0);								//speed
	const double MaxAcceleration = robotCap->maxAcceleration(CVector(0, 0), 0);	//acceleration

	CGeoPoint ball_predict_pos = ball.Pos();
	const double ballVelDir = ball.Vel().dir();
	double ball_time = 0;
	double ball_speed = ball.Vel().mod();
	double robot_time = 0.0;
	int increase_step = 5;

	const double distball2me = ball.Pos().dist(me.Pos());
	double robot_ahead_time = 0.2;
	if (distball2me > 150) {		//TODO
		robot_ahead_time = 0.05;
	}
	else if (distball2me > 75) {
		robot_ahead_time = 0.0;
	}
	else {
		robot_ahead_time = -0.10;
	}
	//ģ�������Ԥ��㼰ʱ��: 1-С��������2-���ڽ���;3-��ʱ�������ʱ��
	long predict_cycle = 0;
	do {
		ball_predict_pos = BallSpeedModel::Instance()->posForTime(predict_cycle, pVision);
		predict_cycle += increase_step;
		ball_time += increase_step*STEP_TIME;
		CGeoPoint tmpPoint = ball_predict_pos + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER, Utils::Normalize(task().player.angle + Param::Math::PI));
		if (CMU_TRAJ == TRAJECTORY_METHORD)	//CMU�����ٵ���켣����
			robot_time = expectedCMPathTime(me, tmpPoint, MaxAcceleration, MaxSpeed, CM_PREDICT_FACTOR) + STEP_TIME*Param::Latency::TOTAL_LATED_FRAME;
		else
			robot_time = expectedPathTime(me, tmpPoint, MaxAcceleration, MaxSpeed) + STEP_TIME*Param::Latency::TOTAL_LATED_FRAME;
	} while (robot_time + robot_ahead_time > ball_time && !Utils::OutOfField(ball_predict_pos)
		&& robot_time <= MAX_TIME && ball_time <= MAX_TIME);

	//�������
	const double cal_time = robot_time;				//ʱ��
	const CGeoPoint predict_posBall = ball_predict_pos;	//Ԥ���
	const double dist_rawball2predictball = ball.Pos().dist(ball_predict_pos);	//ԭʼ�㵽Ԥ������

																				///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
																				//2.��ǰ������Ϣ����Ҫ��ͼ����Ϣ
																				///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const double meSpeed = me.Vel().mod();
	const double finalKickDir = task().player.angle;									//���õ�������
	const CVector self2ball = predict_posBall - me.Pos();								//С����Ԥ��������
	const CVector self2rawball = ball.Pos() - me.Pos();									//С������ǰ������
	const double dist2predictball = self2ball.mod();									//С����Ԥ�������
	const double dist2ball = self2rawball.mod();										//С������ǰ�����
	const double reverse_finalDir = Utils::Normalize(finalKickDir + Param::Math::PI);		//����������ķ���
	const double dAngDiff = Utils::Normalize(self2ball.dir() - finalKickDir);
	const double dAngDiffRaw = Utils::Normalize(self2rawball.dir() - finalKickDir);		//С������ǰ�� - ������ �н�	TSB
	const CVector ballVel = ball.Vel();
	const double ballSpeed = ballVel.mod();
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
	const double dAngleFinalKick2BallVelDir = fabs(Utils::Normalize(finalKickDir - ballVelDir));
	const CGeoPoint myPos = me.Pos();
	const CGeoLine self2targetLine = CGeoLine(myPos, myPos + Utils::Polar2Vector(800, Utils::Normalize(finalKickDir)));			//С��������Ŀ����ֱ��
	const CGeoSegment self2targetSeg = CGeoSegment(myPos, myPos + Utils::Polar2Vector(800, Utils::Normalize(finalKickDir)));
	const CGeoLine ballMovingLine = CGeoLine(ball.Pos(), ball.Pos() + Utils::Polar2Vector(800, Utils::Normalize(ballVelDir)));		//����ֱ��
	const CGeoSegment ballMovingSeg = CGeoSegment(ball.Pos(), ball.Pos() + Utils::Polar2Vector(800, Utils::Normalize(ballVelDir)));
	const CGeoLineLineIntersection self2targetLine_ballMovingLine = CGeoLineLineIntersection(self2targetLine, ballMovingLine);
	CGeoPoint self2targetLine_ballMovingLine_secPos = predict_posBall;
	if (self2targetLine_ballMovingLine.Intersectant()) {
		self2targetLine_ballMovingLine_secPos = self2targetLine_ballMovingLine.IntersectPoint();	//С����Ŀ������ߺ������ߵĽ���
	}
	const bool isIntersectionPosOnself2targetSeg = self2targetSeg.IsPointOnLineOnSegment(self2targetLine_ballMovingLine_secPos);	//������С��Ŀ����߶�
	const bool isIntersectionPosOnBallMovingSeg = ballMovingSeg.IsPointOnLineOnSegment(self2targetLine_ballMovingLine_secPos);		//�����������߶�
	const CGeoPoint ballProj = self2targetLine.projection(ball.Pos());
	const bool ballOnTargetSeg = self2targetSeg.IsPointOnLineOnSegment(ballProj);

	const CGeoPoint projMe = ballMovingSeg.projection(me.Pos());					//С�������ƶ����ϵ�ͶӰ��
	double projDist = projMe.dist(me.Pos());										//С����ͶӰ��ľ���
	const double ball2projDist = projMe.dist(ball.Pos());							//ͶӰ�㵽��ľ���
	const bool meOnBallMovingSeg = ballMovingSeg.IsPointOnLineOnSegment(projMe);	//ͶӰ���Ƿ��������߶�����

	const double dAngeMeVel2BallVel = Utils::Normalize(me.Vel().dir() - ballVelDir);			//С���ٶȷ�������ٷ���ļн�
	const double dSpeedMe2Ball = fabs(ballSpeed - me.Vel().mod()*cos(dAngeMeVel2BallVel));	//���������߷��������ٶ�


	bool isCanDirectKick = false;
	//������Ϣ������û��??
	if (isSensored) {
		if (dAngleMeDir2FinalKick < Param::Math::PI / 10) {
			isCanDirectKick = true;
		}
	}
	//ͼ����Ϣ������
	double go_kick_factor = self2rawball.mod() / GO_KICK_BALL_CRITICAL_DIST;
	go_kick_factor = go_kick_factor > 1.0 ? 1.0 : go_kick_factor;	//�Ƕȿ���,����
	go_kick_factor = go_kick_factor < 0.5 ? 0.5 : go_kick_factor;	//�Ƕȿ���,����
	double DirectKickAllowAngle = go_kick_factor*Param::Vehicle::V2::KICK_ANGLE;
	if (!isBallSpeedFast) {		//�����٣�����ԭʼ����Ϣ
		if (fabs(Utils::Normalize(self2rawball.dir() - me.Dir())) < /*1.5**/DirectKickAllowAngle
			&& dAngleMeDir2FinalKick < Param::Math::PI / 20) {		//��������ǰ�� ��  С���ѳ���Ŀ�귽��
			isCanDirectKick = true;
		}
		if (fabs(Utils::Normalize(self2rawball.dir() - me.Dir())) < /*1.5**/DirectKickAllowAngle
			&& dAngleMeDir2FinalKick < Param::Math::PI / 20
			&& dAngleMeBall2BallVelDir < Param::Math::PI / 4
			|| dAngleMeBall2BallVelDir > 3 * Param::Math::PI / 4) {		//���ٷ����䷴���� �� Ŀ�������� ��һ��
			isCanDirectKick = true;
		}
		if (ballSpeed < 50 && dist2ball < 2.5*Param::Vehicle::V2::PLAYER_SIZE
			&& fabs(Utils::Normalize(self2rawball.dir() - me.Dir())) < Param::Math::PI / 4.0) {
			isCanDirectKick = true;
		}
	}
	else {						//����٣�����ԭʼ����Ϣ
		if (fabs(Utils::Normalize(self2rawball.dir() - me.Dir())) </* 1.25**/DirectKickAllowAngle
			&& dAngleMeDir2FinalKick < Param::Math::PI / 30
			&& self2rawball.mod() <= GO_KICK_BALL_CRITICAL_DIST) {	//��������ǰ�� ��  С���ѳ���Ŀ�귽��
			isCanDirectKick = true;
		}
		if (fabs(Utils::Normalize(self2rawball.dir() - me.Dir())) </* 1.25**/DirectKickAllowAngle
			&& dAngleMeDir2FinalKick < Param::Math::PI / 20
			&& dAngleMeBall2BallVelDir < Param::Math::PI / 6
			|| dAngleMeBall2BallVelDir > 5 * Param::Math::PI / 6) {		//���ٷ����䷴���� �� Ŀ�������� ��һ��
			isCanDirectKick = true;
		}
	}
	// ��TouchKickPos�ƹ����ģ�cliffyin
	double dAngleForKickDir = fabs(Utils::Normalize(finalKickDir - self2ball.dir()));
	double extra_buffer = 0.0;
	bool is_ball_just_front = fabs(Utils::Normalize(self2rawball.dir() - me.Dir())) < Param::Vehicle::V2::KICK_ANGLE
		&& self2rawball.mod() < 2.5*Param::Vehicle::V2::PLAYER_SIZE;
	if (is_ball_just_front) {
		extra_buffer = -Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER*cos(dAngleForKickDir)*self2ball.mod() / (1.5*Param::Vehicle::V2::PLAYER_SIZE) - 2;
	}
	else {
		extra_buffer = max(5.0 - 2 * cos(dAngleForKickDir), 3.5);
		extra_buffer = min(extra_buffer, 1.5);
	}

	//cout<<"aaaaaaaaa: "<< fabs(Utils::Normalize(self2rawball.dir()-me.Dir()))<<" "<< self2rawball.mod()<<" "<<is_ball_just_front<<endl;
	isCanDirectKick = isCanDirectKick || is_ball_just_front;
	isBallInFront = isBallInFront || is_ball_just_front;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//3.Conditions definition
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool allow_follow = dist2ball < FOLLOWBALL_CRITICAL_DIST;
	bool allow_gokick = isCanDirectKick;
	bool need_rush = dist2ball > FOLLOWBALL_CRITICAL_DIST + 30;
	bool need_follow = dist2ball > GO_KICK_BALL_CRITICAL_DIST + 15 || fabs(Utils::Normalize(self2rawball.dir() - me.Dir())) > Param::Math::PI / 2.0/*2*Param::Vehicle::V2::KICK_ANGLE*/;

	bool allow_wait = false;
	bool not_allow_wait = !allow_wait;
	bool allow_touch_shoot = (fabs(dAngDiffRaw) <= Param::Math::PI / 180.0 * 7);
	//&& (fabs(Utils::Normalize(ballVelDir - Utils::Normalize(self2rawball.dir() + Param::Math::PI))) < Param::Math::PI / 2.5);
	// && (ballSpeed > 25);
	//bool allow_drift_shoot = fabs(ballVelDir)<Param::Math::PI*100/180 && fabs(ballVelDir)>Param::Math::PI*80/180;
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//4.����״̬��ά��
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (ballSpeed >= Ball_Moving_Fast_Speed)
	{
		int new_state = state();
		int old_state;
		do {
			old_state = new_state;
			switch (old_state) {
			case BEGINNING:			// ֱ������ GOTO
				new_state = RUSH_TO_BALL;
				break;
			case RUSH_TO_BALL:
				if (allow_follow) {
					new_state = FOLLOW_BALL;
				}
				if (allow_wait) {
					new_state = WAIT_BALL;
				}
				break;
			case WAIT_BALL:
				if (not_allow_wait)
					new_state = FOLLOW_BALL;
				break;
			case FOLLOW_BALL:
				if (allow_gokick) {
					new_state = GO_KICK_BALL;
				}
				if (need_rush) {
					new_state = RUSH_TO_BALL;
				}
				break;
			case GO_KICK_BALL:
				if (need_follow) {
					new_state = FOLLOW_BALL;
				}
			default:
				new_state = RUSH_TO_BALL;
				break;
			}
		} while (false);
		setState(new_state);
		//��¼��ǰ����
		_lastCycle = pVision->Cycle();
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//5.���ݵ�ǰ��Ϣ�ۺϾ������Ԥ��λ��: ����Խ������������ԽС��Ԥ����Ӧ��֮ԽС//��Ԥ��λ�ü�������
		// TODO��TODO  TODO
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		double predict_factor = 1.0;	//��λ��Ԥ������
		double ballVel_factor = 1.0;	//����Ӱ������
		const CVector rawBall2predictBall = predict_posBall - ball.Pos();
		if (meOnBallMovingSeg) {	//С��ͶӰ�������ƶ������棬�������Ѿ�������ǰ
			predict_factor = sqrt(self2rawball.mod() / 150)*sqrt(sqrt(ballSpeed / 250));
			predict_factor *= sqrt(dSpeedMe2Ball / 100);

		}
		else {
			predict_factor = sqrt(ballSpeed / 250)*sqrt(sqrt(self2rawball.mod() / 150));
			predict_factor *= sqrt(dSpeedMe2Ball / 150);
		}
		if (ballSpeed < 50) {
			predict_factor += 0.5;
			predict_factor = predict_factor>1.25 ? 1.25 : predict_factor;
		}
		else {
			predict_factor -= 0.25;
			predict_factor = predict_factor>0.75 ? 0.75 : predict_factor;
			predict_factor = predict_factor<0.25 ? 0.25 : predict_factor;
		}
		CVector extra_ball_vel = rawBall2predictBall * predict_factor;
		///////////////////??
		if (fabs(Utils::Normalize(extra_ball_vel.dir() - ball.Vel().dir())) > Param::Math::PI / 3.0) {
			extra_ball_vel = extra_ball_vel * (-1.0);
		}

		CGeoPoint real_predict_ballPos = ball.Pos() + extra_ball_vel;

		//some tempt variables
		CGeoPoint proj_temp;
		CVector tmp;
		double gokickFactor = 1.0;
		double myVelSpeedRelative2Final = me.Vel().mod()*cos(Utils::Normalize(me.Vel().dir() - finalKickDir));
		TaskT chase_kick_task(task());
		chase_kick_task.player.flag |= (PlayerStatus::ALLOW_DSS | PlayerStatus::DODGE_BALL);
		switch (state())
		{
		case RUSH_TO_BALL:
			//cout<<"RUSH_TO_BALL"<<endl;
			GDebugEngine::Instance()->gui_debug_msg(real_predict_ballPos, "RUSH_TO_BALL");
			GDebugEngine::Instance()->gui_debug_x(real_predict_ballPos, COLOR_CYAN);
			if (fabs(dAngDiffRaw) <= Param::Math::PI / 2.0) {
				//�趨�ܵĵ㣬�����Ǳ���
				chase_kick_task.player.pos = real_predict_ballPos + Utils::Polar2Vector(1.5*Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE, reverse_finalDir);
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
				if (chase_kick_task.player.vel.mod() > speed_factor * MaxSpeed) {	//����
					chase_kick_task.player.vel = chase_kick_task.player.vel * (speed_factor * MaxSpeed / chase_kick_task.player.vel.mod());
				}
				else {
					chase_kick_task.player.vel = chase_kick_task.player.vel;
				}
			}
			else {	//���Ǳ���
				double nowdir = Utils::Normalize(self2rawball.dir() + Param::Math::PI);
				int sign = Utils::Normalize((nowdir - finalKickDir))>0 ? 1 : -1;
				nowdir = Utils::Normalize(nowdir + sign*Param::Math::PI / 2.0);

				chase_kick_task.player.pos = ball.Pos() + Utils::Polar2Vector(1.5*Param::Field::MAX_PLAYER_SIZE, nowdir);
				chase_kick_task.player.vel = CVector(0, 0);
			}
			break;

		case WAIT_BALL:
			//cout<<"WAIT_BALL"<<endl;
			chase_kick_task.player.pos = self2targetLine_ballMovingLine_secPos
				+ Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE, Utils::Normalize(reverse_finalDir));
			proj_temp = ballMovingSeg.projection(chase_kick_task.player.pos);
			for (int i = 0; i < 10; i++) {
				if (proj_temp.dist(chase_kick_task.player.pos) <= 1.5*Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE + 3) {
					chase_kick_task.player.pos = chase_kick_task.player.pos
						+ Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE, Utils::Normalize(reverse_finalDir));
					proj_temp = ballMovingSeg.projection(chase_kick_task.player.pos);
				}
				break;
			}

			break;

		case FOLLOW_BALL:
			//cout<<"FOLLOW_BALL"<<endl;
			GDebugEngine::Instance()->gui_debug_msg(ball.Pos(), "FOLLOW_BALL");
			projDist = (projDist < Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE + 5) ?
				Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE + 5 : projDist - 3;
			projDist = (projDist > 1.5*Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE) ?
				1.5*Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE : projDist - 2;
			if (!isBallInFront && ballSpeed <= 50) {
				projDist = 1.5*Param::Vehicle::V2::PLAYER_SIZE;
				chase_kick_task.player.pos = ball.Pos() + Utils::Polar2Vector(projDist, reverse_finalDir);
			}
			else {
				chase_kick_task.player.pos = real_predict_ballPos + Utils::Polar2Vector(projDist, reverse_finalDir);
			}

			if (ball.Vel().mod() < 20) {	//���ٹ�С�����������٣������������������
				chase_kick_task.player.vel = ball.Vel() + Utils::Polar2Vector(50, finalKickDir);
			}
			else {
				ballVel_factor = sqrt(ball2projDist / (Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE));
				ballVel_factor = ballVel_factor > 1.25 ? 1.25 : ballVel_factor;
				if (ballMovingSeg.IsPointOnLineOnSegment(projMe)) {		//С���Ѿ�׷����
					ballVel_factor *= -0.5;
				}
				else {	//�������� & ��ǰ�ٶ�
					ballVel_factor *= 0.75;
				}
				tmp = ball.Vel()*ballVel_factor;
				if (myVelSpeedRelative2Final < 15) {
					tmp = tmp + Utils::Polar2Vector(15, Utils::Normalize(finalKickDir));
				}
				else if (myVelSpeedRelative2Final<30) {
					tmp = tmp + Utils::Polar2Vector(15 + myVelSpeedRelative2Final, Utils::Normalize(finalKickDir));
				}
				else {
					tmp = tmp + Utils::Polar2Vector(45, Utils::Normalize(finalKickDir));
				}
				if (tmp.mod() > speed_factor * MaxSpeed) {	//����
					chase_kick_task.player.vel = tmp * (speed_factor * MaxSpeed / tmp.mod());
				}
				else {
					chase_kick_task.player.vel = tmp;
				}
			}
			break;

		case GO_KICK_BALL:
			//cout<<"GO_KICK_BALL"<<endl;
			gokickFactor = (dAngleMeBall2MeDir / Param::Vehicle::V2::KICK_ANGLE) * dist2ball / (2 * Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE);
			if (gokickFactor > 1.25) {
				gokickFactor = 1.25;
			}
			else if (gokickFactor < 0.75) {
				if (meOnBallMovingSeg) {
					gokickFactor = 0.5;
				}
				else {
					gokickFactor = 0.75;
				}
			}
			if (!isBallInFront) {
				gokickFactor = 1.5;
				chase_kick_task.player.pos = ball.Pos() + Utils::Polar2Vector(gokickFactor*Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER, reverse_finalDir);
			}
			else {
				chase_kick_task.player.pos = real_predict_ballPos
					+ Utils::Polar2Vector(gokickFactor*Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER, reverse_finalDir);
			}

			ballVel_factor = sqrt(ball2projDist / (Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE))*sqrt(ballSpeed / 100);
			ballVel_factor = ballVel_factor > 1.0 ? 1.0 : ballVel_factor;
			if (ballMovingSeg.IsPointOnLineOnSegment(projMe)) {		//С���Ѿ�׷����
				ballVel_factor *= -0.5;
			}
			else {	//�������� & ��ǰ�ٶ�
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
			if (tmp.mod() > speed_factor * MaxSpeed) {	//����
				chase_kick_task.player.vel = tmp*(speed_factor*MaxSpeed / tmp.mod());
			}
			else {
				chase_kick_task.player.vel = tmp;
			}
			break;
		default:
			break;
		}
		//�����
		if (state() == FOLLOW_BALL || state() == GO_KICK_BALL) {
			if (meOnBallMovingSeg && projDist < 2 * Param::Vehicle::V2::PLAYER_SIZE && ballSpeed > 50 && (dAngleMeBall2BallVelDir < Param::Math::PI / 4 || dAngleMeBall2BallVelDir > Param::Math::PI * 3 / 4)) {
				if (fabs(Utils::Normalize(finalKickDir - ballVelDir)) < Param::Math::PI / 3.0 || fabs(Utils::Normalize(finalKickDir - ballVelDir)) > Param::Math::PI*2.0 / 3.0) {
					chase_kick_task.player.pos = ball.Pos() + Utils::Polar2Vector(2.0*Param::Field::MAX_PLAYER_SIZE, Utils::Normalize((me.Pos() - projMe).dir()));
					chase_kick_task.player.vel = CVector(0.0, 0.0);
				}
				else {
					chase_kick_task.player.pos = ball.Pos() + Utils::Polar2Vector(1.5*Param::Field::MAX_PLAYER_SIZE, Utils::Normalize((me.Pos() - projMe).dir()));
					chase_kick_task.player.vel = CVector(0.0, 0.0);
				}

				if (dAngleMeBall2BallVelDir < Param::Math::PI / 5) {
					chase_kick_task.player.pos = chase_kick_task.player.pos + Utils::Polar2Vector(2.0*Param::Field::MAX_PLAYER_SIZE, Utils::Normalize((me.Pos() - projMe).dir()));
				}
			}
			if ( /*fabs(dAngDiffRaw) >= Param::Math::PI/3.0 &&*/ !isBallInFront && dist2ball < Param::Field::MAX_PLAYER_SIZE + 5) {
				double nowdir = Utils::Normalize(self2rawball.dir() + Param::Math::PI);
				int sign = Utils::Normalize((nowdir - finalKickDir))>0 ? 1 : -1;
				nowdir = Utils::Normalize(nowdir + sign*Param::Math::PI / 2.0);

				chase_kick_task.player.pos = real_predict_ballPos + Utils::Polar2Vector(5.0*Param::Field::MAX_PLAYER_SIZE, nowdir);
				chase_kick_task.player.vel = CVector(0, 0);
			}
		}
		chase_kick_task.player.rotvel = 0.0;

		/************************************************************************/
		/* 6.Touch Kick���жϼ���������											*/
		/************************************************************************/
		if (allow_touch_shoot) {						/// ��ʱ����������������
			chase_kick_task.player.ispass = false;
			chase_kick_task.player.angle = Utils::Normalize(finalKickDir);
			TouchKickPos::Instance()->GenerateTouchKickPos(pVision, robotNum, finalKickDir);
			chase_kick_task.player.pos = TouchKickPos::Instance()->getKickPos();
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(chase_kick_task));
		}
		else if (ballSpeed < 35/* && ballInfront*/) {		/// ������������ٹ�С��������
			chase_kick_task.player.vel = Utils::Polar2Vector(25, task().player.angle);
			chase_kick_task.player.angle = task().player.angle;
			chase_kick_task.player.flag = playerFlag;
			setSubTask(TaskFactoryV2::Instance()->NoneTrajGetBall(chase_kick_task));
			//	setSubTask(TaskFactoryV2::Instance()->GetBallV4(chase_kick_task));
		}
		else {
			CVector middleSelf2ball = self2ball*0.5 + (ball.Pos() - me.Pos())*0.5;
			double dAngleFront = Utils::Normalize(middleSelf2ball.dir() - me.Dir());
			int signFront = dAngleFront > 0.0 ? +1 : -1;	// +1/�ұ� -1/���
			if (GO_KICK_BALL == state()) {
				//fix me
				if (fabs(dAngDiff) < Param::Math::PI / 36 && self2ball.mod() < Param::Vehicle::V2::PLAYER_SIZE*1.5
					&& ball.Vel().mod() < 35) {
					double direct_Vx = 50 * sqrt(self2ball.mod() / (Param::Vehicle::V2::PLAYER_SIZE*1.5));
					double direct_Vy = -signFront * 15 * sqrt((Param::Vehicle::V2::PLAYER_SIZE*1.5) / (self2ball.mod() + 2));
					_directCommand = CmdFactory::Instance()->newCommand(CPlayerSpeedV2(robotNum, direct_Vx, direct_Vy, 0, 0));
				}
				else {
					setSubTask(TaskFactoryV2::Instance()->GotoPosition(chase_kick_task));
				}

				if (ball.Pos().x() < -Param::Field::PITCH_LENGTH / 2.0 - 50) {
					setSubTask(TaskFactoryV2::Instance()->GotoPosition(chase_kick_task));
				}
			}
			else {
				setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(chase_kick_task));
			}
		}
	}
	else
	{
		//cout << "FUCK BALL 0" <<endl;
		setSubTask(PlayerRole::makeItNoneTrajGetBallNew(task().executor, finalKickDir, CVector(0, 0), task().player.flag, -1));
	}
	CStatedTask::plan(pVision);
}

CPlayerCommand* CChaseKickV1::execute(const CVisionModule* pVision)
{
	if (subTask()) {
		return subTask()->execute(pVision);
	}
	if (_directCommand != NULL) {
		return _directCommand;
	}
	return NULL;
}