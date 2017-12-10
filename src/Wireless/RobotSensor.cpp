#include "RobotSensor.h"
#include <PlayInterface.h>
#include <CommandSender.h>
#include "BestPlayer.h"
#include <WorldModel.h>
#include <fstream>
#include "Global.h"

using namespace std;

namespace {
	// ����ʵ���ǩ
	bool IS_SIMULATION = false;	
	//ofstream stopmsgfile("D:\\stopmsg.txt");
	bool debug = true;
}

CRobotSensor::CRobotSensor()
{
	// �������ʵ���ǩ
	DECLARE_PARAM_READER_BEGIN(General)
	READ_PARAM(IS_SIMULATION)
	DECLARE_PARAM_READER_END

	// ���ݽ��г�ʼ��
	memset(_isValid, false, sizeof(_isValid));
	memset(_timeIndex, 0, sizeof(_timeIndex));
	memset(_updateCycle, 0, sizeof(_updateCycle));
	memset(_valid_count, 2, sizeof(_valid_count));
	memset(_lastKickingChecked, false, sizeof(_lastKickingChecked));
	memset(_lastCheckedKickingCycle, 0, sizeof(_lastCheckedKickingCycle));	

	for(int i=0;i<Param::Field::MAX_PLAYER+1; i ++){
		outputmsg[i]=new char[60];
		memset(outputmsg[i],0,sizeof(outputmsg[i]));
	}

	for(int i = 0; i < Param::Field::MAX_PLAYER+1; i ++) {
		robotInfoBuffer[i].kickId = -1;
		robotInfoBuffer[i].bInfraredInfo = false;
		robotInfoBuffer[i].bControledInfo = false;
		robotInfoBuffer[i].nKickInfo = 0;
		robotInfoBuffer[i].changeNum = 0;
		robotInfoBuffer[i].kickCount = 0;

		_lastInfraredInfo[i] = false;
		_lastBallCtrlInfo[i] = false;
	}

	for (int i=0;i < Param::Field::MAX_PLAYER+1; i ++){
		_robotMsg[i].robotNum = 0;
		_robotMsg[i].capacity = 0;
		_robotMsg[i].battery = 0;
		_robotMsg[i].infare = false;
	}

}

void CRobotSensor::Update(int cycle)
{
	// ���治����ֱ�����Է���
	if (IS_SIMULATION) {
		return;
	}
	// �����Ի�ȡ����С������λ������Ϣ

	//string _refereeMsg = "";

	//if (! vision->gameState().canMove()) {
	//	_refereeMsg = "gameHalt";
	//} else if( vision->gameState().gameOver()/* || _pVision->gameState().isOurTimeout() */){
	//	_refereeMsg = "gameOver";
	//} else if( vision->gameState().isOurTimeout() ){
	//	_refereeMsg = "ourTimeout";
	//} else if(!vision->gameState().allowedNearBall()){
	//	// �Է�����
	//	if(vision->gameState().theirIndirectKick()){
	//		_refereeMsg = "theirIndirectKick";
	//	} else if (vision->gameState().theirDirectKick()){
	//		_refereeMsg = "theirDirectKick";
	//	} else if (vision->gameState().theirKickoff()){
	//		_refereeMsg = "theirKickOff";
	//	} else if (vision->gameState().theirPenaltyKick()){
	//		_refereeMsg = "theirPenaltyKick";
	//	} else{
	//		_refereeMsg = "gameStop";
	//	}
	//} 


	if(vision->GetCurrentRefereeMsg()!="gameStop"){
		for (int i = 1; i <= Param::Field::MAX_PLAYER; i ++) {
			// ��λ������Ϣ��ȡ
			int tempIndex = CommandSender::Instance()->getRobotInfo(i, &rawDataBuffer);
			
			if (tempIndex < 0) {
				continue;
			}

			// ǿ�Ʊ���һ��ʱ�����߳�
			if (_lastKickingChecked[i] || robotInfoBuffer[i].nKickInfo > 0) {
				if (cycle - _lastCheckedKickingCycle[i] > 5) {
					_lastKickingChecked[i] = false;
					robotInfoBuffer[i].nKickInfo = 0;
				}
			}

			// ������Ч(tempIndex��ʾ���ݵ����к�)
			if (tempIndex != _timeIndex[i]){
				_timeIndex[i] = tempIndex;
				_isValid[i] = true;
				_valid_count[i] = 2;
				_updateCycle[i] = cycle;

				UpdateBallDetected(i);

				// ����������İ�
				//if (rawDataBuffer.nKickInfo) {
					UpdateKickerInfo(i, cycle);
				//}
				
			} else {
				_lastKickingChecked[i] = false;
			}
		}
	}else{
		//cout<<"in stop"<<endl;
		static bool hasreceived[Param::Field::MAX_PLAYER]={false};
		static bool timeout[Param::Field::MAX_PLAYER]={false};
		static int count[Param::Field::MAX_PLAYER]={0};
		static int i=1;
		//i��ʾÿ����λ�ľ��߳���
		static bool noneedcheck=false;
		static int cycle=0;
		if ((vision->Cycle()-cycle)>6){
			noneedcheck=false;
			cycle=WorldModel::Instance()->vision()->Cycle()-cycle;
			for (int num=1;num<Param::Field::MAX_PLAYER+1;num++){
				memset(outputmsg[num],0,sizeof(outputmsg[num]));
			}
			//cout<<"firstin cycle:"<<cycle<<endl;
		}
		  cycle = WorldModel::Instance()->vision()->Cycle();
		if (i>Param::Field::MAX_PLAYER){ 
			i=1;
			noneedcheck=true;
			//cout<<"********************************"<<endl;
			//printf("RealNum:  %d\t%d\t%d\t%d\t%d\t%d\n",_robotMsg[1].robotNum,_robotMsg[2].robotNum,_robotMsg[3].robotNum,_robotMsg[4].robotNum,
			//	_robotMsg[5].robotNum,_robotMsg[6].robotNum);
			//printf("Battery: ");
			//for(int j = 1; j <= 6; j++ ){
			//	printf("%.2f\t", (_robotMsg[j].battery + 3.951) / 13.51 );
			//}
			//printf("\n");
			//printf("Capcity:  %d\t%d\t%d\t%d\t%d\t%d\n",_robotMsg[1].capacity,_robotMsg[2].capacity,_robotMsg[3].capacity,_robotMsg[4].capacity,
			//	_robotMsg[5].capacity,_robotMsg[6].capacity);
			//cout<<"Infared:  "<<_robotMsg[1].infare<<"\t"<<_robotMsg[2].infare<<"\t"<<_robotMsg[3].infare<<"\t"<<_robotMsg[4].infare<<"\t"<<_robotMsg[5].infare<<"\t"
			//	<<_robotMsg[6].infare<<endl;
			//cout<<"********************************"<<endl;
			return;
		}
		int realnum= PlayInterface::Instance()->getRealIndexByNum(i);
		if ( !noneedcheck){
			if (realnum){
				if ((!hasreceived[i] || !timeout[i])&&!noneedcheck){
					CommandSender::Instance()->setstop(i,true);
					int tempIndex = CommandSender::Instance()->getRobotInfo(i, &rawDataBuffer);
				//int p=rawDataBuffer.battery;
				//int f=rawDataBuffer.nRobotNum;
				//cout<<cycle<<"  i= "<<i<<"  robotnum=  "<<f<<"  tempIndex= "<<tempIndex<<"  battery=  "<<p<<endl;
					if (tempIndex != _timeIndex[i] && tempIndex>=0) {
						_timeIndex[i] = tempIndex;
						_isValid[i] = true;
						_valid_count[i] = 2;
						_updateCycle[i] = cycle;
						hasreceived[i]=true;
						int RobotNum=rawDataBuffer.nRobotNum;
						int batterymsg=rawDataBuffer.battery;
						int capacity=rawDataBuffer.capacity;
						bool infaredmsg=rawDataBuffer.bInfraredInfo;
						_robotMsg[i].robotNum = RobotNum;
						_robotMsg[i].battery = batterymsg;
						_robotMsg[i].capacity = capacity;
						_robotMsg[i].infare = infaredmsg;
						memset(outputmsg[i],0,sizeof(outputmsg[i]));
						if (batterymsg>0 && infaredmsg==false){
							sprintf(outputmsg[i],"num: %d bat: %d cap: %d good",RobotNum,batterymsg,capacity);
						}else if (batterymsg>0 && infaredmsg==true){
							sprintf(outputmsg[i],"num: %d bat: %d cap: %d bad",RobotNum,batterymsg,capacity);
						}else{
							;
						}
					//stopmsgfile<<RobotNum<<'\t'<<batterymsg<<'\t'<<capacity<<'\t'<<infaredmsg<<endl;
						hasreceived[i]=false;
						count[i]=0;
						RobotNum=0;
						batterymsg=0;
						capacity=0;
						CommandSender::Instance()->setstop(i,false);
						i++;
						if (i<=Param::Field::MAX_PLAYER){
							CommandSender::Instance()->setstop(i,true);
						}
					}else{
						count[i]++;
						if (count[i]==10) {
						/*cout<<"Packet Number is :		"<<count[i]<<endl;*/
							count[i] = 0;
							timeout[i]=true;
							CommandSender::Instance()->setstop(i,false);
							timeout[i]=false;
						//cout<<i<<" "<<"not matched"<<endl;
							i++;
							if (i<=Param::Field::MAX_PLAYER){
								CommandSender::Instance()->setstop(i,true);
							}
						}
					}
				}
			}else{
			i++;
			}
		}
}

	return ;
}

bool CRobotSensor::IsInfraredOn(int num)				
{ 
	if(IS_SIMULATION){
		return BestPlayer::Instance()->isOurPlayerStrictControlBall(num);
	} else{
		return robotInfoBuffer[num].bInfraredInfo; 
	}
}

void CRobotSensor::UpdateBallDetected(int num)
{
	// ���������Լ����������
	int realnum = PlayInterface::Instance()->getRealIndexByNum(num);
	if (realnum != rawDataBuffer.nRobotNum) {
		return ;
	}

	// ���ں����ź�ʱ�ı���ϴ� [8/7/2011 cliffyin]
	// ����
	bool currentInfraredInfo = rawDataBuffer.bInfraredInfo;
	bool InfraredInfoReset = false;
	if (_lastInfraredInfo[num] != currentInfraredInfo) {
		InfraredInfoReset = true;
		_lastInfraredInfo[num] = currentInfraredInfo;
	}

	if (InfraredInfoReset) {
		robotInfoBuffer[num].bInfraredInfo = currentInfraredInfo;
		//cout<<"Num : "<<num<<" Real : "<<rawDataBuffer.nRobotNum<<endl;
		//cout<<"Change CurrentInfraredInfo : "<<currentInfraredInfo<<endl;
	}

	// ����
	bool currentBallCtrl = rawDataBuffer.bControledInfo;
	bool BallCtrlReset = false;
	if (_lastBallCtrlInfo[num] != currentBallCtrl) {
		BallCtrlReset = true;
		_lastBallCtrlInfo[num] = currentBallCtrl;
	}

	if (BallCtrlReset) {
		robotInfoBuffer[num].bControledInfo = currentBallCtrl;
	/*	cout<<"Num : "<<num<<" Real : "<<rawDataBuffer.nRobotNum<<endl;
		cout<<"Change CurrentBallCtrl : "<<currentBallCtrl<<endl;*/
	}

	return ;
}

void CRobotSensor::UpdateKickerInfo(int num, int cycle)
{
	// ǰ�����Ѿ����ذ������߳����ź�
	//std::cout << "Kick~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" <<rawDataBuffer.nKickInfo << endl;
	robotInfoBuffer[num] = rawDataBuffer;
	//if (num == 3 || num == 2) cout << num << " infra : " << rawDataBuffer.bInfraredInfo << endl;
	_lastKickingChecked[num] = true;
	_lastCheckedKickingCycle[num] = cycle;
	
	return ;
}

void CRobotSensor::setKickingChecked(int num, int cycle)
{
	/*_lastCheckedKickingCycle[num] = cycle;
	_lastKickingChecked[num] = true;*/

	return ;
}