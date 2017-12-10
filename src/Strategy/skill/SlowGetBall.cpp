#include "SlowGetBall.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include <utils.h>
#include <DribbleStatus.h>
#include <RobotSensor.h>
#include <CommandFactory.h>
#include <KickStatus.h>

namespace{
	enum slow_get_ball_state
	{
		S_GETBALL = 1,
		S_GOTOWARD,
		S_RUSH,
		S_STOP
	};

	//>��Ҫ�õ��ĳ���
	const double newVehicleBuffer = -0.2;//0.6               // С��ͻ�����PLAYER_FRONT_TO_CENTER����
	const int dribblePower =3;// 1;                        // ��������
	const double low_speed = 10;//10                       // �����ǰ��С�ٶ�
	const double stopDist = 3; //3
	const double getBallOverDist = Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + newVehicleBuffer + Param::Field::BALL_SIZE + stopDist - 2.5;
	const int steadyCycle = 10;                         // ȷ������ס���֡��
	static CGeoPoint startPoint = CGeoPoint(0,0);
	//>������
	bool Verbose = false;                              // ����ģʽ

	//�õ��ľ�̬����
	bool trueBallCatched = false;
	int catchBallCount = 0;
	int actionCount = 0;
	int cntJumpOutGoTowards = 0;//��GO_TOWARD״̬����ȥ�ļ�����
}

CSlowGetBall::CSlowGetBall()
{
	_lastCycle = 0;
}

void CSlowGetBall::plan(const CVisionModule* pVision)
{
	//��һ�ν��붯�����
	static bool isIn = true;
	if (Verbose)
	{
		if (isIn == true)
		{
			setState(BEGINNING);
			//cout << endl << "get into SLOW_GET_BALL !" << endl;
			isIn = false;
		}
	}
	//�����ж�ʱ�������״̬����
	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1 ){
		//cout << "Clear: get into SLOW_GET_BALL !" << endl;
		setState(BEGINNING);
		catchBallCount = 0;
		trueBallCatched = false;
		actionCount = 0;
	}
	KickStatus::Instance()->clearAll();
	//�Ӿ���������
	const MobileVisionT& ball = pVision->Ball();
	const int robotNum = task().executor;
	const int flags = task().player.flag;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const CGeoPoint myhead = me.Pos()+Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + newVehicleBuffer,me.Dir());
	const double finalDir = task().player.angle;
	const CGeoLine myheadLine = CGeoLine(myhead,Utils::Normalize(me.Dir() + Param::Math::PI/2.0));
	const CGeoPoint ball2myheadLine_ProjPoint = myheadLine.projection(ball.Pos());
	
	//״̬�ж�ģ��
	bool isBallFrontOfMyhead = myhead.dist(ball2myheadLine_ProjPoint) < 3.5 ? true : false;                            //2.0
	bool dirReached = fabs(Utils::Normalize(finalDir - me.Dir())) < Param::Math::PI*7/180.0 ? true : false;  //Param::Math::PI*5/180.0
	bool speedReached = me.Vel().mod() < 10.0 ? true : false;                                                           //5.0
	bool distReached = me.Pos().dist(ball.Pos()) < getBallOverDist + 5.0 ? true : false;                               //getBallOverDist + 2.0
	bool getballComplete = isBallFrontOfMyhead && dirReached && speedReached && distReached;   //�ж��Ƿ���ת��S_GOTOWARD״̬
	bool fraredOn = false;
	bool ballControled = false;
	if (RobotSensor::Instance()->IsInfraredOn(robotNum))
	{
		fraredOn = true;
	}	
	if (RobotSensor::Instance()->isBallControled(robotNum))
	{
		ballControled = true;
	}
	bool ballCatched = fraredOn /*&& ballControled*/;
	if (S_GOTOWARD == getState())                    // ���ж�ballCatched����֡����϶�ballCatched
	{
		if (ballCatched)
		{
			catchBallCount++;
		} else catchBallCount = 0;
		if (catchBallCount > steadyCycle)
		{
			trueBallCatched = true;
		}
	}
	static int fraredLongOff = 0;
	if (!fraredOn)
	{
		fraredLongOff++;
	} else fraredLongOff = 0;

	//�������
	if (Verbose)
	{
		cout << "check kick status : !!!!!!!!!!! " << ((flags & PlayerStatus::FORCE_KICK) | (flags & PlayerStatus::KICK_WHEN_POSSIABLE)) << endl;
		//cout<<isBallFrontOfMyhead<<dirReached<<speedReached<<distReached<<"   getBallComplete: "<<getballComplete<<endl;
		//cout<<fraredOn<<ballControled<<"   ballCatched: "<<ballCatched<<"   catchBallCount : "<<catchBallCount<<"   trueCatchBall: "<<trueBallCatched<<endl<<endl;
	}

	//״̬������ģ��
	if (BEGINNING == getState())
	{
		setState(S_GETBALL);
		actionCount = 0;
	}
	else if (S_GETBALL == getState())
	{
		if (getballComplete || isBallFrontOfMyhead && actionCount > 300)//************TODO�������ǰ���⣬�˴���ǿӲ����
		{
			setState(S_GOTOWARD);
			cntJumpOutGoTowards = 0;
			actionCount = 0;
			startPoint = ball.Pos();
		} else if (actionCount > 60)
		{
			setState(BEGINNING);
			actionCount = 0;
		}
	}
	else if (S_GOTOWARD == getState())
	{
		//cout<<trueBallCatched<<"  "<<startPoint.dist(me.Pos())<<endl;
		if (trueBallCatched) //|| startPoint.dist(me.Pos()) < 3)//6 //û���ϴ���ʱ�� ���Ӿ�����
		{
			//if(!trueBallCatched){
			//	cout<<"use!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
			//}
			setState(S_STOP);
			actionCount = 0;
			catchBallCount = 0;
			trueBallCatched = false;

		} else{
			if (actionCount > 150 && !fraredOn || !dirReached || !isBallFrontOfMyhead)//�������ǰ��;�У��ۻ�����״̬��ע����actionCount���Ƚϵ�ֵ��û������
			{
				if (++cntJumpOutGoTowards > 5)
				{				
					setState(BEGINNING);
					actionCount = 0;
				}
			}else{
				cntJumpOutGoTowards = 0;
			}
		} 
	} else if (S_STOP == getState())
	{
		/*if (startPoint.dist(ball.Pos()) > 5)
		{
			setState(S_RUSH);
			actionCount = 0;
		}
		else*/ if (me.Pos().dist(ball.Pos()) > 50 /* || fraredLongOff > 30*/)
		{
			setState(BEGINNING);
			actionCount = 0;
			catchBallCount = 0;
			trueBallCatched = false;
		} 
	} else if (S_RUSH == getState())
	{
		if (me.Pos().dist(ball.Pos()) > 50)
		{
			setState(BEGINNING);
			actionCount = 0;
			catchBallCount = 0;
			trueBallCatched = false;
		}
	}
	if (Verbose)
	{
		if (S_GETBALL == getState())
		{
			cout<<"S_GETBALL"<<endl;
			GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(170, -150), "S_GETBALL",COLOR_CYAN);
		} else if (S_GOTOWARD == getState())
		{
			cout<<"S_GOTOWARD"<<endl;
			GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(170, -150), "S_GOTOWARD",COLOR_CYAN);
		} else if (S_STOP == getState())
		{
			cout<<"S_STOP"<<endl;
			GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(170, -150), "S_STOP",COLOR_CYAN);
		} else if (S_RUSH == getState())
		{
			cout<<"S_RUSH"<<endl;
			GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(170, -150), "S_RUSH",COLOR_CYAN);
		}
	}
	//״ִ̬��
	int myState = getState();
	double x_speed = low_speed * cos(finalDir);
	double y_speed = low_speed * sin(finalDir);
	//������
	DribbleStatus::Instance()->setDribbleCommand(robotNum,dribblePower);
	KickStatus::Instance()->setBothKick(robotNum,0,0);
	switch (myState)
	{
	case S_GETBALL:
		actionCount++;
		setSubTask(PlayerRole::makeItNoneTrajGetBall(robotNum,finalDir,CVector(0,0),flags,stopDist));
		break;
	case S_GOTOWARD:
		actionCount++;
		setSubTask(PlayerRole::makeItRun(robotNum,x_speed,y_speed,0,PlayerStatus::DRIBBLING));
		break;
	case S_STOP:
		setSubTask(PlayerRole::makeItRun(robotNum,0.0,0.0,0.0,PlayerStatus::DRIBBLING));
		break;
	case S_RUSH:
		setSubTask(PlayerRole::makeItNoneTrajGetBall(robotNum,finalDir,CVector(0,0),flags,-2));
		break;
	default: break;
	}
	//����actionCount
	if (actionCount > 500)
	{
		actionCount = 500;
	}

	_lastCycle = pVision->Cycle();
	CStatedTask::plan(pVision);
}

CPlayerCommand* CSlowGetBall::execute(const CVisionModule* pVision)
{
	if( subTask() ){
		return subTask()->execute(pVision);
	}	
	return NULL;
}
