#include "GoAndTurnKickV4.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include <utils.h>
#include <DribbleStatus.h>
#include "BallSpeedModel.h"
#include <RobotSensor.h>
#include <KickStatus.h>

namespace{
	//>�������״̬��
	enum get_ball_state{
		DIRECTGOTO = 1,
		GETBALL,
		AVOIDBALL
	};
	//>AVOIDBALL״̬�µı���״̬�ֲ�
	enum avoid_ball_state{
		NOAVOID = 1,
		BALLBEHINDME,
		BALLBESIDEME
		//�����
	};
	avoid_ball_state ab_state = NOAVOID;
	//>GETBALL��״̬�ֲ�
	enum getball_state{
		SMALLANGLE = 1,
		LARGEANGLE
	};
	getball_state gb_state = LARGEANGLE;
	//>״ִ̬�в鿴�� TODO

	//>��Ҫ�õ��ĳ���
	const double newVehicleBuffer = 0.6;               // С��ͻ�����PLAYER_FRONT_TO_CENTER����
	const double ballPredictBaseTime = 0.2;            // ��Ԥ��Ļ���ʱ��
	const double SpeedLimitForPredictTime = 120;       // ���ڸ��ٶ�������Ӵ�Ԥ��ʱ��
	const double nearBallRadius = 12;                  // С���뾶+��뾶+2~3��Ƶ���ԣ��,�ж��Ƿ���ҪAVOIDBALL�İ뾶
	const double ball2myheadLimit = 3;                 // С��ͳ��쳤7cm����뾶2cm����Ҫ�ȶ���ס����Ҫ(7-2)/2=2.5cm �ټ�1.0����ֵ
	const double avoidBallSuccessDist = 25;            // �ж�����ɹ��ľ��� 25
	// ��ת��ʱ���⼸��ֵ�����޸ģ�directGetBalDistҪ��minGetBallDist��3cm
	double minGetBallDist = 17;				   // ��С�������
	double directGetBallDist = 20;               // ֱ�ӳ���ȥ����ľ���
	double maxGetBallDist = 30;				   // 25
	const double AllowFaceToFinalDist = 200;           // ����þ����ʼת��
	const double transverseBallSpeed = 20;             // ���������Ӱ�����ͺ������� ori��30
	const bool Allow_Start_Dribble = true;             // ���������ֵ
	const double DribbleLevel = 3;                     // �������ȵȼ�	
	const double extremeAngle = Param::Math::PI/* * 176 / 180.0*/;
	const double directGetBballDirLimit = Param::Math::PI/6.0;
	const int maxFrared = 100;	//���⼫��ֵ
	// ��Beginʱ��ס��Ҫת�ĽǶ�
	double turnAngleInBegin = 0;
	//>������
	bool Verbose = false;                              // ����ģʽ
	const bool USE_BALL_SPEED_MODEL = false;           // Ӧ������ģ��

	//�õ��ľ�̬����
	bool trueNeedAvoidBall = false;
	int avoidBallCount = 0;
	int fraredOn = 0;
	int fraredOff = 0;
}

CGoAndTurnKickV4::CGoAndTurnKickV4()
{
	_lastCycle = 0;
}

void CGoAndTurnKickV4::plan(const CVisionModule* pVision)
{
	static bool isIn = true;
	if (Verbose)
	{
		if (isIn == true)
		{
			setState(BEGINNING);
			cout<<endl<<"get into CGetBallV3 !"<<endl;
			isIn = false;
		}
	}
	// �ڲ�״̬��������
	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1 ){
		setState(BEGINNING);
		ab_state = NOAVOID;
		gb_state = LARGEANGLE;
		trueNeedAvoidBall = false;
		avoidBallCount = 0;
		isIn = true;
	}
	//////////////////////////////////////////////////////////////////////////
	//�Ӿ���������
	const MobileVisionT& ball = pVision->Ball();
	const int robotNum = task().executor;
	CTRL_METHOD mode = task().player.specified_ctrl_method;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const CGeoPoint myhead = me.Pos()+Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + newVehicleBuffer,me.Dir());
	const CVector self2ball = ball.Pos() - me.Pos();
	const CVector ball2self = me.Pos() - ball.Pos();
	const CVector head2ball = ball.Pos() - myhead;
	const double StopDist = task().player.rotvel;
	// ��С���λ����Ԥ��
	double dAngle_ball2myhead_ballvel = Utils::Normalize(Utils::Normalize(head2ball.dir()+Param::Math::PI)-ball.Vel().dir());
	double dAngle_self2ball_medir = Utils::Normalize(self2ball.dir()-me.Dir());
	double BallPosWithVelFactorTmp = ballPredictBaseTime;
	double finalDir = /*(task().player.pos - ball.Pos()).dir()*/task().player.angle;
	double dAngle_finalDir2ballVel = Utils::Normalize(finalDir - ball.Vel().dir());
	CGeoPoint ballPosWithVel = CGeoPoint(0,0);
	CVector ballVel = ball.Vel();
	// ���ٹ��󣬱������򣬼Ӵ�Ԥ��
	if (ball.Vel().mod() > SpeedLimitForPredictTime && fabs(dAngle_finalDir2ballVel) > Param::Math::PI * 2 / 3) {
		BallPosWithVelFactorTmp += 0.2;
	}
	// ���ڳ���ǰ������Ԥ��ʱ���Ӱ�죬���Ǻ���
	bool frared = RobotSensor::Instance()->IsInfraredOn(robotNum);
	//����С���湦�ܵĺ���
	if ( frared ){
		fraredOn = fraredOn >= maxFrared ? maxFrared : fraredOn+1;
		fraredOff = 0;
	}
	else{
		fraredOn = 0;
		fraredOff = fraredOff >= maxFrared ? maxFrared : fraredOff+1;
	}
	if (fabs(dAngle_self2ball_medir) < Param::Vehicle::V2::KICK_ANGLE) {
		if (fabs(dAngle_ball2myhead_ballvel) < Param::Math::PI/6.0) {                      // �򳯳�ͷ������,Ԥ��ʱ���С
			BallPosWithVelFactorTmp *= sin(fabs(dAngle_ball2myhead_ballvel));
		} else if (fabs(dAngle_ball2myhead_ballvel) > Param::Math::PI*5/6.0 &&
			fabs(Utils::Normalize(ball.Vel().dir() - finalDir)) < Param::Math::PI/6.0)               // ��׷��Ԥ��ʱ������
		{
			if (fraredOn < 20)
			{
				BallPosWithVelFactorTmp += 0.2;
				BallPosWithVelFactorTmp *= cos(fabs(dAngle_ball2myhead_ballvel)) * -1;
			}else{
				//�������ʱ���⴦�������ٴ�СԤ��ֵ��Ϊ0
				ballVel = Utils::Polar2Vector(0.01,ball.Vel().dir());
			}
		}
	}
	//Ԥ��BallPosWithVelFactorTmpʱ��֮�����λ��
	ballPosWithVel = ball.Pos() + ballVel * BallPosWithVelFactorTmp;                           // ���Լ���Ԥ�����λ��
	//С����Ԥ��
	if (ball.Vel().mod() < 20.0 && ball.Vel().mod() * sin(fabs(Utils::Normalize(ball.Vel().dir() - finalDir))) > 10)
	{
		ballPosWithVel = ball.Pos() + Utils::Polar2Vector(Param::Field::BALL_SIZE * 1.2,ball.Vel().dir());
	}
	//GDebugEngine::Instance()->gui_debug_x(ballPosWithVel,COLOR_YELLOW);
	// ������ر�����ֵ
	const CGeoLine myheadLine = CGeoLine(myhead,Utils::Normalize(me.Dir() + Param::Math::PI/2.0));
	const CGeoPoint ball2myheadLine_ProjPoint = myheadLine.projection(ballPosWithVel);
	double reverse_finalDir = Utils::Normalize(finalDir+Param::Math::PI);
	double dAngDiff_self2ball_finaldir = fabs(Utils::Normalize(self2ball.dir()-finalDir));
	CGeoLine ball_finalDir_line = CGeoLine(ball.Pos(),ball.Pos()+Utils::Polar2Vector(50,Utils::Normalize(finalDir)));

	//////////////////////////////////////////////////////////////////////////
	//����Task��ʼ��
	TaskT getball_task(task());
	getball_task.player.angle = finalDir;
	getball_task.player.rotvel = 0.0;

	// ��ǩ����˵������
	/*
	�����ǩ: PlayerStatus::GOTO_GRASP_BALL	 --> ���ܵ��ø�������ϲ��Ƿ������ã���ǿ���������ã���������
	�����ǩ: PlayerStatus::DODGE_BALL		 --> ���ܵ��ø�������ϲ��Ƿ������ã���ǿ�в������ã�����������
	�����ǩ: PlayerStatus::DRIBBLING		 --> ����ϲ�����Ƿ�������
	*/
	getball_task.player.flag = getball_task.player.flag | PlayerStatus::DODGE_OUR_DEFENSE_BOX;//����ҷ�����!!
	getball_task.player.flag = getball_task.player.flag;	//���������ǩ
	getball_task.player.flag = getball_task.player.flag & (~PlayerStatus::DODGE_BALL);		//ȡ�������ǩ
	if( !(getball_task.player.flag & PlayerStatus::DRIBBLING) )
		getball_task.player.flag = getball_task.player.flag & (~PlayerStatus::DRIBBLING);	//ȡ�������ǩ

	//////////////////////////////////////////////////////////////////////////
	//״̬�ж�ģ��
	bool isBallFrontOfMyhead = myhead.dist(ball2myheadLine_ProjPoint) < ball2myheadLimit ? true : false; //������ߵ�ͶӰ���ڿ�������ķ�Χ��
	bool isInNearBallCircle = me.Pos().dist(ball.Pos()) < nearBallRadius ? true : false ;            //�Ƿ���AVOIDBALLСȦ֮��
	bool isInDirectGetBallCircle = me.Pos().dist(ballPosWithVel) < directGetBallDist ? true : false ;    //�Ƿ���ֱ�ӳ���ȥ�������֮��
	bool isGetBallDirReached = fabs(dAngDiff_self2ball_finaldir) < directGetBballDirLimit;
	bool isCanDirectGetBall = isBallFrontOfMyhead && isInDirectGetBallCircle && isGetBallDirReached;     //��Ҫ������:�Ƿ���ֱ����ǰ����
	bool dirOut = fabs(dAngDiff_self2ball_finaldir) > Param::Math::PI /5.0;
	bool canNOTDirectGetBall = !isBallFrontOfMyhead || !isInDirectGetBallCircle || dirOut;
	bool isBallBesideMe = false;                                                                         
	bool isBallBehindMe = false;
	if (isInNearBallCircle)
	{
		if (canNOTDirectGetBall)
		{
			if (fabs(dAngDiff_self2ball_finaldir) > Param::Math::PI / 2.0)
			{
				isBallBehindMe = true;
			} else isBallBesideMe = true;
		}
	}
	bool needAvoidBall = isBallBehindMe || isBallBesideMe;                                                //��Ҫ���������Ƿ���Ҫ�����
	if (Verbose)
	{
		//cout<<"isInNearBallCircle :"<<me.Pos().dist(ball.Pos())<<endl;
	}
		
	if (Verbose)
	{
		//cout<<"FrontOfMyhead: "<<isBallFrontOfMyhead<<"  InGetBallCircle: "<<isInDirectGetBallCircle<<"  DirReached: "<<isGetBallDirReached<<" dirOut: "<<dirOut<<endl;
	}
	//ͨ�������һ������Ƿ���ҪavoidBall
	if (RobotSensor::Instance()->IsInfraredOn(robotNum))
	{
		needAvoidBall = false;
		if (Verbose)
		{
			//cout<<"fraredon!!! NONEED avoidBall"<<endl;
		}		
	}
	double isAvoidBallSuccess = nearBallRadius *(1.2 + dAngDiff_self2ball_finaldir / Param::Math::PI);
	bool avoidBallSuccess = me.Pos().dist(ball.Pos()) > isAvoidBallSuccess ? true : false;          //��Ҫ���������Ƿ�����ɹ����������ִ��ģ���м���
	if (Verbose)
	{
		//cout<<" avoidBallSuccess :  "<<avoidBallSuccess<<endl;
	}
	if (AVOIDBALL != getState())
	{
		if (needAvoidBall)
		{
			avoidBallCount++;
		} else avoidBallCount = 0;
		if (avoidBallCount > 15)//15
		{
			trueNeedAvoidBall = true;
		}
	}
    if (Verbose)
    {
		//cout<<"avoidBallCount: "<<avoidBallCount<<"  trueNeedAvoidBall: "<<trueNeedAvoidBall<<endl;
    }
	//////////////////////////////////////////////////////////////////////////
	//״̬����ģ�� *����״̬��������ڲ鿴״̬��תTODO
	if (BEGINNING == getState())             //��ǰ״̬ΪBEGINNING
	{
		turnAngleInBegin = fabs(Utils::Normalize(me.Dir() - finalDir))*180/Param::Math::PI;
		if (isCanDirectGetBall)
		{
			setState(DIRECTGOTO);
			if (Verbose)
			{
				cout<<"-->DirectGoto";
			}
		} else if (trueNeedAvoidBall)
		{
			setState(AVOIDBALL);
			if (Verbose)
			{
				cout<<"-->AvoidBall";
			}
			trueNeedAvoidBall = false;
			avoidBallCount = 0;
			if (isBallBehindMe)
			{
				ab_state = BALLBEHINDME;
			} else if (isBallBesideMe)
			{
				ab_state = BALLBESIDEME;
			}
		} else {
			setState(GETBALL);
			gb_state = LARGEANGLE;
			if (Verbose)
			{
				cout<<"-->GetBall";				
			}
		}
	} else if (DIRECTGOTO == getState())     //��ǰ״̬ΪDIRECTGOTO
	{
		if (RobotSensor::Instance()->IsInfraredOn(robotNum))
		{ //ʲô������������ת״̬
		}
		else if (trueNeedAvoidBall)
		{
			setState(AVOIDBALL);
			if (Verbose)
			{
				cout<<"-->AvoidBall";
			}
			trueNeedAvoidBall = false;
			avoidBallCount = 0;
			if (isBallBehindMe)
			{
				ab_state = BALLBEHINDME;
			} else if (isBallBesideMe)
			{
				ab_state = BALLBESIDEME;
			}
		} else if (canNOTDirectGetBall)
		{
			setState(GETBALL);
			gb_state = LARGEANGLE;
			if (Verbose)
			{
				cout<<"-->GetBall";				
			}
		}		
	} else if (GETBALL == getState())        //��ǰ״̬ΪGETBALL
	{
		if (isCanDirectGetBall)
		{
			setState(DIRECTGOTO);
			if (Verbose)
			{
				cout<<"-->DirectGoto";
			}
		}else if (RobotSensor::Instance()->IsInfraredOn(robotNum))
		{ //ʲô������������ת״̬
		}
		else if (trueNeedAvoidBall)
		{
			setState(AVOIDBALL);
			if (Verbose)
			{
				cout<<"-->AvoidBall";				
			}
			trueNeedAvoidBall = false;
			avoidBallCount = 0;
			if (isBallBehindMe)
			{
				ab_state = BALLBEHINDME;
			} else if (isBallBesideMe)
			{
				ab_state = BALLBESIDEME;
			}
		}
	} else if (AVOIDBALL == getState())      //��ǰ״̬ΪAVOIDBALL
	{
		if (avoidBallSuccess || isCanDirectGetBall)
		{
			setState(GETBALL);
			gb_state = LARGEANGLE;
			if (Verbose)
			{
				cout<<"-->GetBall";				
			}
			ab_state = NOAVOID;
		}
	}
	//״̬�鿴
	if (Verbose)
	{
		if (DIRECTGOTO == getState())
		{
			GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(170, -150), "DirectGoto",COLOR_CYAN);
		} else if (GETBALL == getState())
		{
			GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(170, -150), "GetBall",COLOR_CYAN);
		} else if (AVOIDBALL == getState())
		{
			GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(170, -150), "AvoidBall",COLOR_CYAN);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	//״ִ̬��ģ��
	int nowState = getState();
	switch (nowState)
	{
	case DIRECTGOTO:{
		getball_task.player.pos = ballPosWithVel + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + newVehicleBuffer + Param::Field::BALL_SIZE + StopDist - 2.5,reverse_finalDir); //  5.85
		break;}
	case GETBALL:{
		getball_task.player.pos = ball.Pos();
	/*	if (LARGEANGLE == gb_state)
		{
			if (fabs(dAngDiff_self2ball_finaldir) <= Param::Math::PI - extremeAngle + Param::Math::PI * 5 / 180)
			{
				gb_state = SMALLANGLE;
			}
		} else if (SMALLANGLE == gb_state )
		{
			if (fabs(dAngDiff_self2ball_finaldir) > Param::Math::PI * 3 / 180.0)
			{
				gb_state = LARGEANGLE;
			}
		}*/
		if(SMALLANGLE == gb_state)	{
			if (Verbose)
			{
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, -150), "small angle",COLOR_CYAN);
			}
			double getBallBuffer = -3 + 1 * me.Pos().dist(ballPosWithVel)/50;  //����ʱ��Ƶ�����-2 + 7 * me.Pos().dist(ballPosWithVel)/50;
			if (getBallBuffer > 2)
			{
				getBallBuffer = 2;
			}
			double getBallDist = Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + newVehicleBuffer + Param::Field::BALL_SIZE + getBallBuffer;
			if (getBallDist > me.Pos().dist(ballPosWithVel) )
			{
				getBallDist = Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + newVehicleBuffer + Param::Field::BALL_SIZE + StopDist - 2.5;
			}
			getball_task.player.pos = ballPosWithVel + Utils::Polar2Vector(getBallDist,reverse_finalDir); //�þ���ҪС��directGetBallDist
		} else if(LARGEANGLE == gb_state)
		{
			if (Verbose)
			{
				GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, -150), "large angle",COLOR_CYAN);
			}
			//�Ƕȼ���
			double theta_Dir = (pVision->OurPlayer(robotNum).RawPos() - pVision->Ball().Pos()).dir();//ball2self.dir();
			int sign = Utils::Normalize((theta_Dir - finalDir)) > 0 ? 1 : -1;			
			theta_Dir = Utils::Normalize(theta_Dir + sign * Param::Math::PI * 65 / 180.0); //Param::Math::PI * 75 / 180.0
			if (Verbose)
			{
				//GDebugEngine::Instance()->gui_debug_line(ballPosWithVel, ballPosWithVel+Utils::Polar2Vector(100,theta_Dir),COLOR_WHITE);
			}
			int sign2 = Utils::Normalize((theta_Dir - finalDir)) > 0 ? 1 : -1;		
			if (ball.Vel().mod() * sin(fabs(Utils::Normalize(ball.Vel().dir() - finalDir))) < transverseBallSpeed)//�������Ŀ�귽���С��ʱ��
			{
				if (sign * sign2 < 0 || fabs(Utils::Normalize(theta_Dir - finalDir)) > extremeAngle)
				{
					theta_Dir = Utils::Normalize(finalDir + sign * extremeAngle);				
				}
			}	
			//�������
			double theta = fabs(Utils::Normalize(theta_Dir - finalDir));
			// ��ͬ��ʼ�Ƕȸ���������벻һ��
			/*minGetBallDist = -0.04444444*turnAngleInBegin+34;
			if (minGetBallDist > 25) {
				minGetBallDist = 25;
			}
			if (minGetBallDist < 17) {
				minGetBallDist = 17;
			}
			maxGetBallDist = -0.04444444*turnAngleInBegin+34;
			if (maxGetBallDist > 30) {
				maxGetBallDist = 30;
			}
			if (maxGetBallDist < 26) {
				maxGetBallDist = 26;
			}*/

			directGetBallDist = minGetBallDist + 3;
			

			double getBallDist = minGetBallDist + (maxGetBallDist - minGetBallDist) * (1 + cos(theta));
			if (getBallDist > maxGetBallDist) {
				getBallDist = maxGetBallDist;
			}
			if (getBallDist < minGetBallDist) {
				if (theta < Param::Math::PI * 170 / 180.0) {
					getBallDist = minGetBallDist;
				}
			}
			//cout<<"getBallDist: "<<getBallDist<<" "<<theta<<endl;
			getball_task.player.pos = ballPosWithVel + Utils::Polar2Vector(getBallDist,theta_Dir);
		}		
		break;}
	case AVOIDBALL:{//TODO ��������Ӱ�죬�������ٶȵ������������ܵ�
		if (BALLBEHINDME == ab_state)
		{
			double theta_Dir = ball2self.dir();
			double theta = Utils::Normalize(theta_Dir - finalDir);
			int sign = theta > 0 ? 1 : -1;
			theta_Dir = Utils::Normalize(theta_Dir + sign * Param::Math::PI * 60 / 180);
			getball_task.player.pos = ballPosWithVel + Utils::Polar2Vector(30,theta_Dir);
		} else if (BALLBESIDEME == ab_state)
		{
			double theta_Dir = reverse_finalDir;
			getball_task.player.pos = ballPosWithVel + Utils::Polar2Vector(30,theta_Dir);
		}		
		break;}
	default: break;
	}
	// ��������ĳ��򣬱Ƚ�Զ����ʱ������ȡ���ٵķ���֮����ת
	double diffAngleVel2Final = fabs(dAngDiff_self2ball_finaldir);
	int sign = diffAngleVel2Final > Param::Math::PI / 2.0 ? 1 : 0;
	if( ball.Pos().dist(me.Pos()) > AllowFaceToFinalDist ) {
		getball_task.player.angle = Utils::Normalize(self2ball.dir() + sign*Param::Math::PI);
	}
	// ����Զ�������������,���Ըò���Ҫ�ﵽ���㣺1С��������Զʱ��Ч���� 2��Ҫ���̫��������
	//double extr_dist = 0.0;
	//if (self2ball.mod() > 200 ) {    
	//	extr_dist = self2ball.mod() * self2ball.mod() / 1000; 
	//	if (extr_dist > 160) {
	//		extr_dist = 160;
	//	}
	//	CGeoPoint fast_point = getball_task.player.pos + Utils::Polar2Vector(extr_dist,(getball_task.player.pos - me.Pos()).dir());
	//	if (! Utils::OutOfField(fast_point,0)) {
	//		getball_task.player.pos = fast_point;
	//	}		
	//}
	// �����趨
	// ����������������ֹͣ���� [8/19/2011 cliffyin]
	//if (KickStatus::Instance()->needKick(robotNum)) {
	//	DribbleStatus::Instance()->setDribbleCommand(robotNum,0);
	//} else {
	//	if (Allow_Start_Dribble && me.Pos().dist(ball.Pos()) < 50 && !(task().player.flag & PlayerStatus::NOT_DRIBBLE)) 
	//	{
	//		DribbleStatus::Instance()->setDribbleCommand(robotNum,DribbleLevel);
	//	}
	//}
	
	// ���ͼ�λ���
	if(Verbose) {
		//GDebugEngine::Instance()->gui_debug_x(ball.Pos(),COLOR_WHITE);
		//GDebugEngine::Instance()->gui_debug_x(ballPosWithVel,COLOR_BLACK);
		GDebugEngine::Instance()->gui_debug_x(getball_task.player.pos,COLOR_RED);
		GDebugEngine::Instance()->gui_debug_line(getball_task.player.pos,getball_task.player.pos + getball_task.player.vel,COLOR_RED);
	}
	// ���õײ����
	getball_task.player.is_specify_ctrl_method = true;
	getball_task.player.specified_ctrl_method = mode;
	if( DIRECTGOTO == getState()) {
		setSubTask(TaskFactoryV2::Instance()->GotoPosition(getball_task));
	} else {
		setSubTask(TaskFactoryV2::Instance()->GotoPosition(getball_task));
	}

	_lastCycle = pVision->Cycle();
	CStatedTask::plan(pVision);
}

CPlayerCommand* CGoAndTurnKickV4::execute(const CVisionModule* pVision)
{
	if( subTask() ){
		return subTask()->execute(pVision);
	}

	return NULL;
}