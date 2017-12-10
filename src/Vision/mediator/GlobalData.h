#ifndef GLOBAL_DATA
#define GLOBAL_DATA

#include "utils\singleton.h"
#include "net\message.h"

class CGlobalData 
{
public:
	CGlobalData(){ followCheckCycle = 10;cameraNum=MaxNumOfCamera;firstStart=false;}
	~CGlobalData(){}
	SendVisionMessage smsg;
	ReceiveVisionMessage receiveMsg[MaxNumOfCamera+2];
	ReceiveVisionMessage receiveMsgBuffer[MaxNumOfCamera+2];
	bool firstStart;
	int cameraNum;
	int followCheckCycle;
	void copyReceiveMsg()
	{
		for (int i=0;i<MaxNumOfCamera;i++)
		{
			receiveMsgBuffer[i]=receiveMsg[i];
		}
	}
	void setBlueRobot(int cameraId, int i,int robotId,float x,float y,float orientation,float confidence);
	void setYellowRobot(int cameraId, int i,int robotId,float x,float y,float orientation,float confidence);
	void setBall(int cameraId, int i,float x, float y, float confidence);
	void changeSmsgToRmsg(int);
};

typedef NormalSingleton< CGlobalData > GlobalData; 
#endif
