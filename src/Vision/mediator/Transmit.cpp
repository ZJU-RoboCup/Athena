#include "Transmit.h"
#include "GlobalData.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <iostream>
#include <stdio.h>

using namespace std;
namespace{
	int sortCount = 0;
}

Transmit::Transmit()
{
	initRmsgArray();
	for (int i=0;i<GlobalData::Instance()->cameraNum/2;i++)
	{
		//cout << "cameraNum" << GlobalData::Instance()->cameraNum<< endl;
		_cameraMixer[i]=TwoCameraMixer(2*i,2*i+1);
	}
	_lastCameraMixer=TwoCameraMixer(MaxNumOfCamera,MaxNumOfCamera+1);
}

Transmit::~Transmit()
{

}

void Transmit::setFirstStart()
{ 
	GlobalData::Instance()->firstStart = true;
	sortCount = 0; 
}
void Transmit::usortCounter()
{
	if (GlobalData::Instance()->firstStart)
	{
		sortCount++;
		if(sortCount == 60){
			GlobalData::Instance()->firstStart = false;
			sortCount = 0;
		}
	}
}
void Transmit::setCycle(int followcycle, int cycle)
{
	 GlobalData::Instance()->followCheckCycle = followcycle;
	 GlobalData::Instance()->smsg.Cycle = cycle;
}

void Transmit::initRmsgArray()
{
	for(int i = 0; i < MaxNumOfCamera+2 ; i++){
		for(int j = 0; j < MaxBallNum; j++){
			GlobalData::Instance()->receiveMsg[i].Ballx[j] = -ERRORNUM;  //add 2010 +
			GlobalData::Instance()->receiveMsg[i].Bally[j] = -ERRORNUM;
			GlobalData::Instance()->receiveMsg[i].BallFound[j] = false;
		}
		for(int j = 0; j < ReceiveCarNum; j++){
			GlobalData::Instance()->receiveMsg[i].RobotINDEX[BLUE][j] = MaxFollowNum;
			GlobalData::Instance()->receiveMsg[i].RobotFound[BLUE][j] = false;
			GlobalData::Instance()->receiveMsg[i].RobotPosX[BLUE][j] = -ERRORNUM;
			GlobalData::Instance()->receiveMsg[i].RobotPosY[BLUE][j] = -ERRORNUM;
			GlobalData::Instance()->receiveMsg[i].RobotRotation[BLUE][j] = -ERRORNUM;
			GlobalData::Instance()->receiveMsg[i].RobotINDEX[YELLOW][j] = MaxFollowNum;
			GlobalData::Instance()->receiveMsg[i].RobotFound[YELLOW][j] = false;
			GlobalData::Instance()->receiveMsg[i].RobotPosX[YELLOW][j] = -ERRORNUM;
			GlobalData::Instance()->receiveMsg[i].RobotPosY[YELLOW][j] = -ERRORNUM;
			GlobalData::Instance()->receiveMsg[i].RobotRotation[YELLOW][j] = -ERRORNUM;
		}
	}
}

SendVisionMessage Transmit::smsgUpdate(int cameraMode,int minAddFrame, int minLostFrame,float maxdist,bool isFalseSend)
{
	usortCounter();
	int startNum=0;
	if (cameraMode==TwoCamDown||cameraMode==SingleCamLeftDown||cameraMode==SingleCamRightDown)
		startNum=1;
	for (int i=startNum;i<GlobalData::Instance()->cameraNum/2+startNum;i++)
	{
		//minLostFrame±ØÐë¸Ä³É1
		_cameraMixer[i].mixAlgorithm(cameraMode,1,1,maxdist,isFalseSend);
		GlobalData::Instance()->changeSmsgToRmsg(i+MaxNumOfCamera);
	}
	if (cameraMode==FourCamera)
	{
		_lastCameraMixer.mixAlgorithm(cameraMode,minAddFrame,minLostFrame,maxdist,isFalseSend);
	}
	initRmsgArray();
	return GlobalData::Instance()->smsg;
}
