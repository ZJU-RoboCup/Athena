#include "DriftKick.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include <utils.h>
#include <ControlModel.h>
#include <cornell/Trajectory.h>
#include <RobotCapability.h>
#include <CMmotion.h>
#include <TimeCounter.h>
#include <KickStatus.h>
#include <RobotSensor.h>
#include "utils.h"
#include "ShootRangeList.h"
#include <CommandFactory.h>
#include "TouchKickPos.h"
#include "BallSpeedModel.h"
#include <GDebugEngine.h>
#include "PlayInterface.h"

namespace
{
	//2.�켣�����㷨ʹ�ñ���
	int TRAJECTORY_METHORD = 1;

	enum{
		RUSH_TO_BALL = 1,	//���ٽӽ���
		FOLLOW_BALL = 2,	//����������
		GO_KICK_BALL = 3,	//��ǰ������
		SPEED_UP=4,
		WAIT_BALL=5,
		Get_Ball=6
	};

	bool verBos=false;
	//״̬�л���ر���
	const double RUSH_TO_BALL_CRITICAL_DIST = 100;	//100cm
	const double FOLLOWBALL_CRITICAL_DIST = 50;		//50cm
	const double GO_KICK_BALL_CRITICAL_DIST = 2*Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE;

	//Ԥ�����
	double CM_PREDICT_FACTOR = 1.5;
	const double Ball_Moving_Fast_Speed = 50;	//1m/s
	//
	const double speed_factor = 0.7;
	const double Left_Reach_Allowance=5;
	const double Right_Reach_Allowance=5;

	const int State_Counter_Num=5;

	const double crossWiseFactor[13]={1.5,1.5,1.5,1.5,1.5,1.5,1.5,1.5,1.5,1.5,1.5,1.5};
	const double verticalFactor[13]= {1.5,1.5,1.5,1.5,1.5,1.5,1.5,1.5,1.5,1.5,1.5,1.5};

	const double MaxSpeed=350;
}

CDriftKick::CDriftKick()
{
	{
		DECLARE_PARAM_READER_BEGIN(CGotoPositionV2)
		READ_PARAM(TRAJECTORY_METHORD)
			DECLARE_PARAM_READER_END
	}	

	_directCommand = NULL;
	_lastCycle = 0;
	_stateCounter=0;
	_goKickCouter=0;
}

void CDriftKick::plan(const CVisionModule* pVision)
{
	//�ս��뱾skill��Ϊ��ʼ״̬����BEGINNING����Ҫ��һЩ������
	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1 ){
		setState(BEGINNING);
		_goKickCouter=0;
	}

	_directCommand = NULL;
	const MobileVisionT& ball = pVision->Ball();
	const int robotNum = task().executor;
	const int realNum=PlayInterface::Instance()->getRealIndexByNum(robotNum);
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const int playerFlag = task().player.flag;

	const CGeoPoint predict_posBall = BallSpeedModel::Instance()->posForTime(20, pVision);	//Ԥ���

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//2.��ǰ������Ϣ����Ҫ��ͼ����Ϣ
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const double meSpeed = me.Vel().mod();
	double finalKickDir = task().player.angle;									//���õ�������

	//��������ǶȲ���
	//if (fabs(ballVelDir)>Param::Math::PI/6){
	//	finalKickDir=finalKickDir-ballVelDir/3;
	//	GDebugEngine::Instance()->gui_debug_line(me.Pos(),me.Pos()+Utils::Polar2Vector(800,Utils::Normalize(finalKickDir)),COLOR_BLACK);
	//}
	const CVector self2ball = predict_posBall - me.Pos();								//С����Ԥ��������
	const CVector self2rawball = ball.Pos() - me.Pos();									//С������ǰ������
	const double dist2predictball = self2ball.mod();									//С����Ԥ�������
	const double dist2ball = self2rawball.mod();										//С������ǰ�����
	const double reverse_finalDir = Utils::Normalize(finalKickDir+Param::Math::PI);		//����������ķ���
	const double dAngDiff = Utils::Normalize(self2ball.dir()-finalKickDir);
	const double dAngDiffRaw = Utils::Normalize(self2rawball.dir()-finalKickDir);		//С������ǰ�� - ������ �н�	TSB
	const CVector ballVel = ball.Vel();
	const double ballSpeed = ballVel.mod();
	const double ballVelDir = ball.Vel().dir();
	bool isBallSpeedFast = (ballSpeed >= Ball_Moving_Fast_Speed)?true:false;		//�����趨����ֵ�ж������Ƿ��㹻��

	//?????
	double allowInfrontAngleBuffer = (dist2ball/(Param::Vehicle::V2::PLAYER_SIZE))*Param::Vehicle::V2::KICK_ANGLE < Param::Math::PI/5.0?
		(dist2ball/(Param::Vehicle::V2::PLAYER_SIZE))*Param::Vehicle::V2::KICK_ANGLE:Param::Math::PI/5.0;
	bool isBallInFront = fabs(Utils::Normalize(self2rawball.dir()-me.Dir())) < allowInfrontAngleBuffer
		&& dist2ball < (2.5*Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE);

	//�����ź�
	double isSensored = RobotSensor::Instance()->IsInfoValid(robotNum) && RobotSensor::Instance()->IsInfraredOn(robotNum);	//�Ƿ��м�⵽����
	//ͼ���Ӿ�
	const double dAngleMeDir2FinalKick = fabs(Utils::Normalize(me.Dir()-finalKickDir));				//��ǰ���������������н�
	const double dAngleMeBall2BallVelDir = fabs(Utils::Normalize(self2rawball.dir() - ballVelDir));	//�������������߼н�
	const double dAngleMeBall2MeDir = fabs(Utils::Normalize(self2rawball.dir() - me.Dir()));		//��������С������н�
	const double dAngleBall2FinalKick=fabs(Utils::Normalize(self2rawball.dir() - finalKickDir));	
	const double dAngleFinalKick2BallVelDir = fabs(Utils::Normalize(finalKickDir - ballVelDir));	
	const double antiKickDir=Utils::Normalize(finalKickDir+Param::Math::PI);
	const CGeoPoint myPos = me.Pos();
	const CGeoLine self2targetLine = CGeoLine(myPos,myPos+Utils::Polar2Vector(800,Utils::Normalize(finalKickDir)));			//С��������Ŀ����ֱ��
	const CGeoSegment self2targetSeg = CGeoSegment(myPos,myPos+Utils::Polar2Vector(800,Utils::Normalize(finalKickDir)));
	const CGeoLine ballMovingLine = CGeoLine(ball.Pos(),ball.Pos()+Utils::Polar2Vector(800,Utils::Normalize(ballVelDir)));		//����ֱ��
	const CGeoSegment ballMovingSeg = CGeoSegment(ball.Pos(),ball.Pos()+Utils::Polar2Vector(800,Utils::Normalize(ballVelDir)));	
	const CGeoLineLineIntersection self2targetLine_ballMovingLine = CGeoLineLineIntersection(self2targetLine,ballMovingLine);
	CGeoPoint self2targetLine_ballMovingLine_secPos = predict_posBall;
	if( self2targetLine_ballMovingLine.Intersectant() ){
		self2targetLine_ballMovingLine_secPos = self2targetLine_ballMovingLine.IntersectPoint();	//С����Ŀ������ߺ������ߵĽ���
	}
	const bool isIntersectionPosOnself2targetSeg = self2targetSeg.IsPointOnLineOnSegment(self2targetLine_ballMovingLine_secPos);	//������С��Ŀ����߶�
	const bool isIntersectionPosOnBallMovingSeg = ballMovingSeg.IsPointOnLineOnSegment(self2targetLine_ballMovingLine_secPos);		//�����������߶�
	const CGeoPoint ballProj = self2targetLine.projection(ball.Pos());
	const bool ballOnTargetSeg = self2targetSeg.IsPointOnLineOnSegment(ballProj);

	CGeoPoint kickPos=ball.Pos()+Utils::Polar2Vector(10,antiKickDir);
	CGeoPoint goalPos1=CGeoPoint(Param::Field::PITCH_LENGTH/2,-40);
	CGeoPoint goalPos2=CGeoPoint(Param::Field::PITCH_LENGTH/2,40);
	const CGeoSegment goalSeg=CGeoSegment(goalPos1,goalPos2);
	const CGeoLine	goalLine=CGeoLine(goalPos1,goalPos2);
	CGeoLineLineIntersection goalLine_ballVel=CGeoLineLineIntersection(goalLine,ballMovingLine);
	bool isBallVelOnGoalLine=false;
	if (goalLine_ballVel.Intersectant()){
		CGeoPoint goalLine_ballVel_secPos=goalLine_ballVel.IntersectPoint();
		if (goalSeg.IsPointOnLineOnSegment(goalLine_ballVel_secPos)){
			isBallVelOnGoalLine=true;
		}
	}		
	const CGeoPoint projMe = ballMovingSeg.projection(me.Pos());					//С�������ƶ����ϵ�ͶӰ��
	double projDist = projMe.dist(me.Pos());										//С����ͶӰ��ľ���
	const double ball2projDist = projMe.dist(ball.Pos());							//ͶӰ�㵽��ľ���
	const bool meOnBallMovingSeg = ballMovingSeg.IsPointOnLineOnSegment(projMe);	//ͶӰ���Ƿ��������߶�����

	const double dAngeMeVel2BallVel = Utils::Normalize(me.Vel().dir()-ballVelDir);			//С���ٶȷ�������ٷ���ļн�
	const double dSpeedMe2Ball = fabs(ballSpeed - me.Vel().mod()*cos(dAngeMeVel2BallVel));	//���������߷��������ٶ�


	bool isCanDirectKick = false;
	//������Ϣ������û��??
	if( isSensored ){	
		if( dAngleMeDir2FinalKick < Param::Math::PI/10 ){
			isCanDirectKick = true;
		}
	}
	//ͼ����Ϣ������
	double go_kick_factor = self2rawball.mod() / GO_KICK_BALL_CRITICAL_DIST;	
	go_kick_factor = go_kick_factor > 1.0? 1.0 : go_kick_factor;	//�Ƕȿ���,����
	go_kick_factor = go_kick_factor < 0.5? 0.5 : go_kick_factor;	//�Ƕȿ���,����
	double DirectKickAllowAngle = go_kick_factor*Param::Vehicle::V2::KICK_ANGLE;
	//����٣�����ԭʼ����Ϣ
	if( fabs(Utils::Normalize(self2rawball.dir()-me.Dir())) </* 1.25**/DirectKickAllowAngle
		&& dAngleMeDir2FinalKick < Param::Math::PI/30
		&& self2rawball.mod() <= GO_KICK_BALL_CRITICAL_DIST ){	//��������ǰ�� ��  С���ѳ���Ŀ�귽��
			isCanDirectKick = true;
	}
	if( fabs(Utils::Normalize(self2rawball.dir() - me.Dir())) </* 1.25**/DirectKickAllowAngle
		&& dAngleMeDir2FinalKick < Param::Math::PI/20 
		&& dAngleFinalKick2BallVelDir<Param::Math::PI/18&&isBallVelOnGoalLine
		&& dAngleMeBall2BallVelDir < Param::Math::PI/15
		|| dAngleMeBall2BallVelDir > 14*Param::Math::PI/15){		//���ٷ����䷴���� �� Ŀ�������� ��һ��
			isCanDirectKick = true;
	}

	bool is_ball_just_front = fabs(Utils::Normalize(self2rawball.dir()-me.Dir())) < Param::Vehicle::V2::KICK_ANGLE
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
	bool need_follow = dist2ball > GO_KICK_BALL_CRITICAL_DIST + 15	||  fabs(Utils::Normalize(self2rawball.dir() - me.Dir())) > Param::Math::PI/2.0/*2*Param::Vehicle::V2::KICK_ANGLE*/;


	bool allow_touch_shoot = (fabs(dAngDiffRaw) <= Param::Math::PI/2.0)
		&& (fabs(Utils::Normalize(ballVelDir - Utils::Normalize(self2rawball.dir()+Param::Math::PI))) < Param::Math::PI/2.5)
		&& (ballSpeed > 30)&&ball.Pos().x()>me.Pos().x()+8&&fabs(ballVelDir)>Param::Math::PI/1.8;

	//bool allow_touch_shoot=false;

	bool isReached;	//���Ƿ�ϵ������߻����ұ�
	int  isLeft=-1; //��Ҫ�ϵ��ķ���
	//cout<<ballVelDir<<endl;
	//cout<<isLeft<<endl;
	if (sin(ballVelDir)*Param::Field::PITCH_LENGTH/2<ball.Pos().x()*sin(ballVelDir)-ball.Pos().y()*cos(ballVelDir)){
		isLeft=1;
	}
	if (isLeft==1){
		isReached=kickPos.y()>me.Pos().y()+Left_Reach_Allowance;
	}else{
		isReached=kickPos.y()<me.Pos().y()-Right_Reach_Allowance;
	}
	bool badAngle=(fabs(ballVelDir)>Param::Math::PI/15&&dAngleFinalKick2BallVelDir>Param::Math::PI/15
		||dAngleFinalKick2BallVelDir>Param::Math::PI/10)&&(!isBallVelOnGoalLine);
	bool need_speed_up=badAngle&&!isReached||(badAngle&&ballSpeed>150&&kickPos.x()<me.Pos().x());
	bool need_wait_ball=badAngle&&isReached&&dist2ball>45&&(kickPos.x()-me.Pos().x())/fabs(kickPos.y()-me.Pos().y())<1;
	bool wait_follow= isReached&&(kickPos.x()-me.Pos().x())/fabs(kickPos.y()-me.Pos().y())>1.2
		||dist2ball<30||dAngleBall2FinalKick<Param::Math::PI/6 
		|| fabs(Utils::Normalize(me.Vel().dir()-ballVelDir))<Param::Math::PI*45/180 
		|| ballSpeed<meSpeed*cos(Utils::Normalize(me.Vel().dir()-ballVelDir))+50;

	bool bigAngle=fabs(ballVelDir)>Param::Math::PI*100/180||ball.Pos().x()<me.Pos().x()+12;
	bool notBigAngle=fabs(ballVelDir)<Param::Math::PI*90/180&&ball.Pos().x()>me.Pos().x()+15;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//4.����״̬��ά��
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (ballSpeed>=Ball_Moving_Fast_Speed)
	{
		int new_state = state();
		int old_state;
		{
			old_state = new_state;
			switch (old_state) {
			case BEGINNING:			// ֱ������ GOTO
				new_state = RUSH_TO_BALL;
				if (bigAngle){
					new_state=Get_Ball;
				}else{
					if (need_speed_up){
						new_state=SPEED_UP;
					}else if (need_wait_ball){
						new_state=WAIT_BALL;
					}
				}
				break;
			case RUSH_TO_BALL:
				if (need_speed_up){
					new_state=SPEED_UP;
				}else if (need_wait_ball)
				{
					new_state=WAIT_BALL;
				}
				else if (allow_follow) {
					new_state = FOLLOW_BALL;
				}
				break;
			case FOLLOW_BALL:
				if (allow_gokick) {
					new_state = GO_KICK_BALL;
				}else if (need_speed_up)
				{
					new_state=SPEED_UP;
				}else if (need_rush){
					new_state=RUSH_TO_BALL;
				}				
				break;
			case GO_KICK_BALL:
				_goKickCouter++;
				if (need_follow){
					new_state=FOLLOW_BALL;
				}
				if (_goKickCouter==30){
					new_state=FOLLOW_BALL;
					setState(FOLLOW_BALL);
					_stateCounter=State_Counter_Num;
					_goKickCouter=0;
				}
				break;
			case SPEED_UP:
				if (isReached){
					new_state=FOLLOW_BALL;
				}else if (allow_gokick){
					new_state=GO_KICK_BALL;
				}
				break;
			case WAIT_BALL:
				if (allow_gokick)
				{
					new_state=GO_KICK_BALL;
				}
				else if (wait_follow)
				{
					new_state=FOLLOW_BALL;
				}
				else if (need_speed_up||wait_follow)
				{
					new_state=SPEED_UP;
				}
				break;
			case Get_Ball:
				if (!bigAngle){
					new_state=FOLLOW_BALL;
				}
				break;
			default:
				new_state = RUSH_TO_BALL;
				break;
			}
		}
		if (state()==BEGINNING){
			setState(new_state);
		}else{
			if (_stateCounter==0){
				setState(new_state);
				_stateCounter++;
			}else{
				if (new_state==state()){
					_stateCounter=min(State_Counter_Num,_stateCounter+1);
				}else{
					_stateCounter=max(0,_stateCounter-1);
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
		if( meOnBallMovingSeg ){	//С��ͶӰ�������ƶ������棬�������Ѿ�������ǰ
			predict_factor = sqrt(self2rawball.mod()/150)*sqrt(sqrt(ballSpeed/250));
			predict_factor *= sqrt(dSpeedMe2Ball/100);

		} else {
			predict_factor = sqrt(ballSpeed/300)*sqrt(sqrt(self2rawball.mod()/150));
			predict_factor *= sqrt(dSpeedMe2Ball/150);
		}
		predict_factor -= 0.25;
		predict_factor = predict_factor>0.75?0.75:predict_factor;
		predict_factor = predict_factor<0.25?0.25:predict_factor;
		CVector extra_ball_vel = rawBall2predictBall * predict_factor ;
		//??
		if (fabs(Utils::Normalize(extra_ball_vel.dir()-ball.Vel().dir())) > Param::Math::PI/3.0) {
			extra_ball_vel = extra_ball_vel * (-1.0);
		}

		CGeoPoint real_predict_ballPos = ball.Pos() + extra_ball_vel*1.2;

		//some tempt variables
		CGeoPoint proj_temp;
		CVector tmp;

		double speedUpDistanceX=0,speedUpDistanceY=0;
		CVector speedUpVel;

		CGeoPoint wait_ballPos=ball.Pos()+ball.Vel()/2;
		double wait_factor=1;

		double gokickFactor = 1.0;
		double myVelSpeedRelative2Final = me.Vel().mod()*cos(Utils::Normalize(me.Vel().dir()-finalKickDir));
		TaskT chase_kick_task(task());
		//cout<<"meballdist"<<ball.Pos().x()-me.Pos().x()<<endl;
		if(verBos) cout<<"chaseState:"<<state()<<endl;
		switch( state() )
		{
			//rush��speed up��wait ball�����м�״̬���Գ�λ�ý��дֵ����ó���Ŵ���һ�������λ�ã�
			//rush�����ٷ���Ƚ�����ֱ�ӳ������ţ�speed up�����ٷ���ϴ��ҳ������������ϣ�wait up�����ٷ���ϴ󣬳�����������
			//follow ball ��go kick ball���Ǹ�������ٶȽ��е������ٵ�Ԥ�����ӣ��Ӷ��ó���ȷ������������ţ�
		case RUSH_TO_BALL:
			//cout<<"RUSH_TO_BALL"<<endl;
			GDebugEngine::Instance()->gui_debug_msg(real_predict_ballPos,"RUSH_TO_BALL");
			GDebugEngine::Instance()->gui_debug_x(real_predict_ballPos,COLOR_CYAN);
			if( fabs(dAngDiffRaw) <= Param::Math::PI/2.0 ){
				//�趨�ܵĵ㣬�����Ǳ���
				chase_kick_task.player.pos = real_predict_ballPos + Utils::Polar2Vector(3*Param::Vehicle::V2::PLAYER_SIZE+Param::Field::BALL_SIZE,reverse_finalDir);
				//add front vel
				if( myVelSpeedRelative2Final < 50 ) {
					chase_kick_task.player.vel = chase_kick_task.player.vel + Utils::Polar2Vector(50,Utils::Normalize(finalKickDir));
				} else if(myVelSpeedRelative2Final<75) {
					chase_kick_task.player.vel = chase_kick_task.player.vel + Utils::Polar2Vector(30+myVelSpeedRelative2Final,Utils::Normalize(finalKickDir));
				} else {
					chase_kick_task.player.vel = chase_kick_task.player.vel + Utils::Polar2Vector(25+myVelSpeedRelative2Final,Utils::Normalize(finalKickDir));
				}
				if( chase_kick_task.player.vel.mod() > speed_factor * MaxSpeed ){	//����
					chase_kick_task.player.vel = chase_kick_task.player.vel * (speed_factor * MaxSpeed /chase_kick_task.player.vel.mod());
				}
			}else{	//���Ǳ��� ��ӳ��󷽹�������������
				double nowdir = Utils::Normalize(self2rawball.dir()+Param::Math::PI);
				int sign = Utils::Normalize((nowdir - finalKickDir))>0?1:-1;
				nowdir = Utils::Normalize(nowdir+sign*Param::Math::PI/2.0);
				chase_kick_task.player.pos = ball.Pos() + Utils::Polar2Vector(1.5*Param::Field::MAX_PLAYER_SIZE,nowdir);
				chase_kick_task.player.vel = CVector(0,0);
			}
			break;
		case SPEED_UP:
			//speed_up״̬�����������ŽǶȣ���ֹճ��Ϊ֮���follow��kick�γ����õ�����λ�úͽǶ�
			//speedUpDistance��speedUpVelӰ�������ǶȵĴ�С��
			//speedUpDistanceYӦ�������ģ��泵��yֵ�Ĳ�ֵԽ��ԽС��Ӱ�������ǶȾ��루�򳵺�����룩��������أ�
			//speedUpDistanceX��Ӱ����������룬�����ٷ�����أ���Ƕ�ʱӦΪ��ֵ��
			//speedUpVel����Ϊ���ٷ���������
			//cout<<"speed Up"<<endl;
			speedUpDistanceY=max(kickPos.y()-me.Pos().y(),0)+crossWiseFactor[realNum]*Param::Vehicle::V2::PLAYER_SIZE+max(ballSpeed-130,0)*sin(fabs(ballVelDir))*0.2;
			speedUpDistanceY=min(speedUpDistanceY,50);
			speedUpDistanceX=15-sin(fabs(ballVelDir))*10+kickPos.x()-me.Pos().x();
			speedUpDistanceX=min(speedUpDistanceX,20);
			speedUpDistanceX=max(speedUpDistanceX,-5);
			speedUpVel=ballVel/ballSpeed*((ballSpeed-80)*1.8+(fabs(ballVelDir)/3.14*180-20)*2);

			if (fabs(ballVelDir)>Param::Math::PI*75/180){
				speedUpDistanceX=-10;
				speedUpVel=CVector(0,0);
				double diffY=fabs(kickPos.y()-ball.Pos().y());
				if (dist2ball>80){
					speedUpDistanceX=(me.Pos().x()-kickPos.x())/4*3;
				}else if (dist2ball>40){
					speedUpDistanceX=(me.Pos().x()-kickPos.x())/3*2;
				}else{
					speedUpDistanceX=-5;
				}
			}
			if (me.Pos().x()-kickPos.x()>0){
				speedUpDistanceX=-20;
			}

			chase_kick_task.player.pos=CGeoPoint(kickPos.x()+speedUpDistanceX,kickPos.y()-isLeft*speedUpDistanceY);				
			chase_kick_task.player.vel=speedUpVel;
			chase_kick_task.player.angle=finalKickDir;

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
			speedUpDistanceY=fabs(ball.Pos().y()-me.Pos().y())+Param::Vehicle::V2::PLAYER_SIZE+(ballSpeed-130)/2.5+sin(fabs(ballVelDir)-Param::Math::PI/10)*50;
			speedUpDistanceY=min(speedUpDistanceY,50);
			speedUpDistanceX=15-fabs(ballVelDir)/3.14*18*2;
			if ((ball.Pos().x()-me.Pos().x())>50){
				wait_factor=1;
			}
			else if((ball.Pos().x()-me.Pos().x())>35){
				wait_factor=0.8;
			}else if ((ball.Pos().x()-me.Pos().x())>25){
				wait_factor=0.65;
			}
			else if ((ball.Pos().x()-me.Pos().x())>15){
				wait_factor=0.4;
			}else{
				wait_factor=0.1;
			}
			chase_kick_task.player.pos=ball.Pos()+ball.Vel()/ballSpeed*((ballSpeed-100)/2*sin(dAngleBall2FinalKick)*wait_factor)/*+ Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER*wait_factor,reverse_finalDir)*/;
			chase_kick_task.player.pos.setY(chase_kick_task.player.pos.y()-isLeft*speedUpDistanceY);
			chase_kick_task.player.pos.setX(chase_kick_task.player.pos.x()-sin(fabs(ballVelDir)-Param::Math::PI/10)*30);
			chase_kick_task.player.vel=ballVel/ballSpeed*((ballSpeed-80)*1.8-30+(fabs(ballVelDir)/3.14*180-20)*2);;
			chase_kick_task.player.angle=finalKickDir;
			//cout<<"pos"<<wait_factor<<endl;
			//����Ѿ��ӽ���ת״̬�ˣ������speed up��������ֱ�ӽ���ʵʱλ����Ϊ��׼�㣬���������٣���ֹ���ܹ����ټ���X��Y����ĵ���ϵ��
			if((ball.Pos().x()-me.Pos().x())<25&&dAngleBall2FinalKick<Param::Math::PI/4){
				chase_kick_task.player.pos=CGeoPoint(ball.Pos().x()+speedUpDistanceX,ball.Pos().y()-isLeft*speedUpDistanceY*0.5);
			}
			if ((ball.Pos().x()-me.Pos().x())<15&&dist2ball<50&&dAngleBall2FinalKick>Param::Math::PI/4&&fabs(ballSpeed-meSpeed)<100){
				chase_kick_task.player.pos=CGeoPoint(ball.Pos().x(),ball.Pos().y()+10);
			}

			break;
		case FOLLOW_BALL:
			GDebugEngine::Instance()->gui_debug_x(real_predict_ballPos,COLOR_BLUE);
			//cout<<"FOLLOW_BALL"<<endl;
			GDebugEngine::Instance()->gui_debug_msg(ball.Pos(),"FOLLOW_BALL");
			projDist = ( projDist < Param::Vehicle::V2::PLAYER_SIZE+Param::Field::BALL_SIZE+5 )?
				Param::Vehicle::V2::PLAYER_SIZE+Param::Field::BALL_SIZE+5 : projDist-3;
			projDist = ( projDist > 1.5*Param::Vehicle::V2::PLAYER_SIZE+Param::Field::BALL_SIZE )?
				1.5*Param::Vehicle::V2::PLAYER_SIZE+Param::Field::BALL_SIZE : projDist-2;

			chase_kick_task.player.pos = real_predict_ballPos + Utils::Polar2Vector(projDist,reverse_finalDir);


			ballVel_factor = sqrt(ball2projDist/(Param::Vehicle::V2::PLAYER_SIZE+Param::Field::BALL_SIZE));
			ballVel_factor = ballVel_factor > 1.25?1.25:ballVel_factor;				
			if( ballMovingSeg.IsPointOnLineOnSegment(projMe) ){		//С���Ѿ�׷����
				ballVel_factor *= -0.5;
			}else{	//�������� & ��ǰ�ٶ�
				ballVel_factor *= 0.75;				
			}
			tmp = ball.Vel()*ballVel_factor;

			if( myVelSpeedRelative2Final < 15 ) {
				tmp = tmp + Utils::Polar2Vector(15,Utils::Normalize(finalKickDir));
			} else if(myVelSpeedRelative2Final<30) {
				tmp = tmp + Utils::Polar2Vector(15+myVelSpeedRelative2Final,Utils::Normalize(finalKickDir));
			} else {
				tmp = tmp + Utils::Polar2Vector(45,Utils::Normalize(finalKickDir));
			}
			if( tmp.mod() > speed_factor * MaxSpeed ){	//����
				chase_kick_task.player.vel = tmp * (speed_factor * MaxSpeed /tmp.mod());
			}else{
				chase_kick_task.player.vel = tmp;
			}
			break;

		case GO_KICK_BALL:
			//cout<<"GO_KICK_BALL"<<endl;
			//cout<<"ball"<<isBallInFront<<endl;
			gokickFactor = (dAngleMeBall2MeDir/Param::Vehicle::V2::KICK_ANGLE) * dist2ball/(2*Param::Vehicle::V2::PLAYER_SIZE+Param::Field::BALL_SIZE);
			if( gokickFactor > 1.25 ){
				gokickFactor = 1.25;
			} else if( gokickFactor < 0.75 ){
				if (meOnBallMovingSeg) {
					gokickFactor = 0.5;
				}else{
					gokickFactor = 0.75;
				}
			}
			if( !isBallInFront ){
				gokickFactor = 1.5;
				chase_kick_task.player.pos = ball.Pos() + Utils::Polar2Vector(gokickFactor*Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER,reverse_finalDir);
			}else{
				chase_kick_task.player.pos = real_predict_ballPos
					+ Utils::Polar2Vector(gokickFactor*Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER,reverse_finalDir);
			}

			ballVel_factor = sqrt(ball2projDist/(Param::Vehicle::V2::PLAYER_SIZE+Param::Field::BALL_SIZE))*sqrt(ballSpeed/100);
			ballVel_factor = ballVel_factor > 1.0?1.0:ballVel_factor;
			if( ballMovingSeg.IsPointOnLineOnSegment(projMe) ){		//С���Ѿ�׷����
				ballVel_factor *= -0.5;
			}else{	//�������� & ��ǰ�ٶ�
				if( isBallInFront )
					ballVel_factor *= 0.75;	
				else
					ballVel_factor *= 1.25;	

			}
			tmp = ball.Vel()*ballVel_factor;
			if( myVelSpeedRelative2Final < 25 ) {
				tmp = tmp + Utils::Polar2Vector(25,Utils::Normalize(finalKickDir));
			} else if(myVelSpeedRelative2Final<50) {
				tmp = tmp + Utils::Polar2Vector(25+myVelSpeedRelative2Final,Utils::Normalize(finalKickDir));
			} else {
				tmp = tmp + Utils::Polar2Vector(75,Utils::Normalize(finalKickDir));
			}
			if( tmp.mod() > speed_factor * MaxSpeed ){	//����
				chase_kick_task.player.vel = tmp*(speed_factor*MaxSpeed/tmp.mod());
			}else{
				chase_kick_task.player.vel = tmp;
			}				
			break;
		default :
			break;
		}
		//�����
		if( state() == FOLLOW_BALL||state()==WAIT_BALL){
			if( meOnBallMovingSeg && projDist < 2*Param::Vehicle::V2::PLAYER_SIZE && ballSpeed > 50 && (dAngleMeBall2BallVelDir < Param::Math::PI/4 || dAngleMeBall2BallVelDir > Param::Math::PI*3/4)){
				if( fabs(Utils::Normalize(finalKickDir-ballVelDir)) < Param::Math::PI/3.0 || fabs(Utils::Normalize(finalKickDir-ballVelDir)) > Param::Math::PI*2.0/3.0 ){
					chase_kick_task.player.pos = ball.Pos() + Utils::Polar2Vector(2.0*Param::Field::MAX_PLAYER_SIZE,Utils::Normalize((me.Pos()-projMe).dir()));
					chase_kick_task.player.vel = CVector(0.0,0.0);
				}else{
					chase_kick_task.player.pos = ball.Pos() + Utils::Polar2Vector(1.5*Param::Field::MAX_PLAYER_SIZE,Utils::Normalize((me.Pos()-projMe).dir()) );
					chase_kick_task.player.vel = CVector(0.0,0.0);
				}		

				if( dAngleMeBall2BallVelDir < Param::Math::PI/5 ){
					chase_kick_task.player.pos = chase_kick_task.player.pos + Utils::Polar2Vector(2.0*Param::Field::MAX_PLAYER_SIZE,Utils::Normalize((me.Pos()-projMe).dir()) );
				}
			}
			if( /*fabs(dAngDiffRaw) >= Param::Math::PI/3.0 &&*/ !isBallInFront && dist2ball < Param::Field::MAX_PLAYER_SIZE+5){
				double nowdir = Utils::Normalize(self2rawball.dir()+Param::Math::PI);
				int sign = Utils::Normalize((nowdir - finalKickDir))>0?1:-1;
				nowdir = Utils::Normalize(nowdir+sign*Param::Math::PI/2.0);

				chase_kick_task.player.pos = real_predict_ballPos + Utils::Polar2Vector(5.0*Param::Field::MAX_PLAYER_SIZE,nowdir);
				chase_kick_task.player.vel = CVector(0,0);
			}
		}

		chase_kick_task.player.rotvel = 0.0;

		/************************************************************************/
		/* 6.Touch Kick���жϼ���ȡ�ײ�skill											*/
		/************************************************************************/
		if (state()==Get_Ball)
		{
			setSubTask(PlayerRole::makeItNoneTrajGetBall(task().executor,finalKickDir,CVector(0,0),0,-2));
		}else{
			if (allow_touch_shoot) {						/// ��ʱ����������������
				chase_kick_task.player.ispass = false;
				chase_kick_task.player.angle = Utils::Normalize(finalKickDir);
				setSubTask(TaskFactoryV2::Instance()->TouchKick(chase_kick_task));
			} else {
				//cout<<"angle"<<chase_kick_task.player.angle<<endl;
				//cout<<"ballvelangle"<<ballVelDir<<endl;
				setSubTask(TaskFactoryV2::Instance()->GotoPosition(chase_kick_task));
			}
		}

	}
	else
	{
		setSubTask(PlayerRole::makeItNoneTrajGetBall(task().executor,finalKickDir,CVector(0,0),task().player.flag,-2));
	}
	CStatedTask::plan(pVision);
}

CPlayerCommand* CDriftKick::execute(const CVisionModule* pVision)
{
	if( subTask() ){
		return subTask()->execute(pVision);
	}
	if( _directCommand != NULL ){
		return _directCommand;
	}
	return NULL;
}
