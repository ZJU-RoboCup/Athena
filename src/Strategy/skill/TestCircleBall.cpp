#include "TestCircleBall.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include "ControlModel.h"

#include <utils.h>
#include <ControlModel.h>

namespace
{
	//ÅäÖÃ×´Ì¬»ú
	enum TestCircleBall
	{
		GO =1,
		CIRCLEBALL,
		ADJUST,
		KICK,
	};
	bool IS_SIMULATION = false;
	int sinT =30;
}

CTestCircleBall::CTestCircleBall()
{
	DECLARE_PARAM_READER_BEGIN(General)
	READ_PARAM(IS_SIMULATION)
	DECLARE_PARAM_READER_END
	//_lastCycle = 0;
	count = 0;
	angeDiff_Use = Param::Math::PI * 2;
}

void CTestCircleBall::plan(const CVisionModule* pVision)
{
	static int _lastCycle = pVision->Cycle();
	//cout <<"LastCycle:"<< _lastCycle << endl;
	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1){
		setState(BEGINNING);
	//	old_state = BEGINNING;
		//cout<<"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"<<endl;
		_lastRotVel = 0;
		count = 0;
	}

	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const int vecNumber = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
	const CVector self2ball = ballPos - me.Pos();
	const CVector ball2self = me.Pos() - ballPos;
	const double finalDir = task().player.angle;

	TaskT myTask(task());

	myTask.player.flag = myTask.player.flag | PlayerStatus::DODGE_OUR_DEFENSE_BOX;
	
	int new_state = state();
	int old_state;
	do{
		old_state = new_state;
		switch (old_state) {
		case BEGINNING:		
			new_state = GO;
			break;

		case GO:	
			if (ball2self.mod() < Param::Vehicle::V2::PLAYER_SIZE +Param::Field::BALL_SIZE + 5) {
				new_state = CIRCLEBALL;
			}
			break;

		case CIRCLEBALL: 
			if (count > sinT)
			{
				count = 0;
				new_state = KICK;
			}
			break;
			/*	case ADJUST:
			if (abs(self2ball.dir() - me.Dir()) * 180 / Param::Math::PI < 4 )
			{
			new_state = KICK;
			}
			break;*/
		case KICK: //
			if ( ball2self.mod()>Param::Vehicle::V2::PLAYER_SIZE * 20) {
				new_state = GO;
			} 
			break;

		default:
			new_state = GO;
			break;
		}
		cout << new_state << endl;
	} while (old_state != new_state);

	setState(new_state);

	if (GO == state())
	{
		cout << "It`s state GO"<<endl;
		myTask.player.pos = ballPos + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE + 1,(me.Pos() - ball.Pos()).dir()) ;
		myTask.player.angle = finalDir;
		setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(myTask));

	} else if (CIRCLEBALL == state()){

		cout << "It`s state Circle Ball"<<endl;

		double angeDiff = Utils::Normalize(finalDir - me.Dir());
		if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1){
			_lastCycle = pVision->Cycle();
		}
		count  = count + pVision->Cycle() - _lastCycle;
		if (count == 0)
		{
			cout << "FinalDir:"<<finalDir * 180 / Param::Math::PI<<endl;
			cout <<"MeDir:"<<me.Dir() * 180 / Param::Math::PI<<endl;
			//double expectedAngel = 0.5 * Param::Math::PI * (180.0 / Param::Math::PI);
			cout << "angelDiff:"<<angeDiff * 180 / Param::Math::PI<<endl;
		    if (abs(angeDiff)*180.0 / Param::Math::PI < 80)
		    {
				angeDiff_Use = angeDiff;
			}else{
				double expectedAngel = angeDiff * (180.0 / Param::Math::PI);
		//		double commandAngel = (expectedAngel + 33.1429) / 1.3452;
				double commandAngel = (expectedAngel + 33.1429) / 1.5;
				angeDiff_Use = commandAngel * Param::Math::PI / 180.0;
				if (angeDiff < 0)
				{
					angeDiff = -angeDiff;
					expectedAngel = angeDiff * (180.0 / Param::Math::PI);
				//	commandAngel = (expectedAngel + 33.1429) / 1.3452;
					 commandAngel = (expectedAngel + 33.1429) / 1.5;
					angeDiff_Use = -commandAngel * Param::Math::PI / 180.0;
				}
				if (abs(angeDiff_Use)<93 * Param::Math::PI / 180.0)
				{
					angeDiff_Use = angeDiff_Use;
				}else if (abs(angeDiff_Use)<96 * Param::Math::PI / 180.0)
				{
					angeDiff_Use = angeDiff_Use * 1.5 / 1.7;
				}else {
					angeDiff_Use = angeDiff_Use * 1.5 / 1.8;
				}
			}
			cout << "angeDiff_Use:"<<angeDiff_Use* 180 / Param::Math::PI<<endl;			
		}

//		cout << "Count:" << count <<endl;
		rotVel = angeDiff_Use / 2 * Param::Math::PI / sinT * sin(Param::Math::PI / sinT * count)* Param::Vision::FRAME_RATE;
		if (count > sinT)
		{
			rotVel = 0;
		}

		double r = ball.Pos().dist(me.Pos());
		if (r <= Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + Param::Field::BALL_SIZE) {
			r = Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER + Param::Field::BALL_SIZE + 1;
		}
		double  curRotVel = rotVel;
		double R = r+5;
		double curVy = -curRotVel * R;
	//	cout<<"R:"<<R << endl;
		CVector localVel(0, curVy);
		CVector globalVel ;
		if (abs(angeDiff_Use) < Param::Math::PI / 2.5)
		{
			globalVel = localVel.rotate(me.Dir());
		}else{
		//	cout << "here!!!!!!!"<<endl;
			if (abs(angeDiff_Use) < 75 *Param::Math::PI / 180.0)
			{
				R = r+8;
			}else if (abs(angeDiff_Use)  < 80 *Param::Math::PI / 180.0 )
			{
				R = r+10 ;
			}else if(abs(angeDiff_Use)  < 85 *Param::Math::PI / 180.0){
				R = R + 10;
			}else if(abs(angeDiff_Use)  < 90 *Param::Math::PI / 180.0){
				R = r + 15;
			}else if(abs(angeDiff_Use)  < 95 *Param::Math::PI / 180.0){
				R = r + 15;
			}else if(abs(angeDiff_Use)  < 100 *Param::Math::PI / 180.0){
				R = r + 15;
			}else{
				R = r + 20;
			}
		//	R = r+10;
			curVy = -curRotVel * R;
			localVel = CVector(0, curVy);
			globalVel= localVel.rotate(me.Dir()  + Param::Math::PI / 17);
			if (rotVel < 0)
			{
				globalVel = localVel.rotate(me.Dir() - Param::Math::PI / 17);
			}
		}
		myTask.player.speed_x = globalVel.x() ;
		myTask.player.speed_y = globalVel.y() ;
		myTask.player.rotate_speed = rotVel;
		setSubTask(TaskFactoryV2::Instance()->Speed(myTask));
	}else if(ADJUST == state()) 
	{
		/*	cout << "It`s state ADJUST"<<endl;
			*/
		double dir = finalDir;
		if (finalDir > 0)
		{
			dir = Utils::Normalize(finalDir - Param::Math::PI /2);
		}else{
			dir=Utils::Normalize( finalDir + Param::Math::PI / 2);
		}
		myTask.player.pos = ballPos + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE + 2,-dir) ;
		myTask.player.angle = finalDir;
		/*	cout<<"TaskPosX:"<<myTask.player.pos.x()<<"	"<<"TaskPosY:"<<myTask.player.pos.y()<<endl;
		cout << "MePosX:" << me.Pos().x() << "	"<<"MePosY:" <<me.Pos().y()<<endl;
		cout << "Dist:"<<me.Pos().dist(myTask.player.pos)<<endl;*/
		//	myTask.player.flag = myTask.player.flag | PlayerStatus::DODGE_BALL;
		//	GDebugEngine::Instance()->gui_debug_x(myTask.player.pos,COLOR_ORANGE); */
		setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(myTask));
	}
	else if (KICK == state()){
		cout << "It`s state KICK"<<endl;
		//myTask.player.speed_x = 0;
		//myTask.player.speed_y = 0;
		//myTask.player.rotate_speed = 0;
		//setSubTask(TaskFactoryV2::Instance()->Speed(myTask));
		//myTask.player.angle = me.Dir();
		//setSubTask(TaskFactoryV2::Instance()->NoneTrajGetBall(myTask));
		myTask.player.flag = myTask.player.flag | PlayerStatus::NOT_DRIBBLE;
		setSubTask(TaskFactoryV2::Instance()->PassBall(myTask));
		//	setSubTask(PlayerRole::makeItSimpleGoto(task().executor, me.Pos()+Utils::Polar2Vector(40,me.Dir()), me.Dir(),Utils::Polar2Vector(20, finalDir),0));
	}

	_lastCycle = pVision->Cycle();	
	return CStatedTask::plan(pVision);


}
CPlayerCommand* CTestCircleBall::execute(const CVisionModule* pVision)
{
	if(subTask()) {
		return subTask()->execute(pVision);
	}

	return NULL;
}