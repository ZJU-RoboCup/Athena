#include "GlobalData.h"

void CGlobalData::setBlueRobot(int cameraId, int i,int robotId,float x,float y,float orientation,float confidence)
{
	receiveMsg[cameraId].RobotINDEX[BLUE][i] = robotId;//Blue
	receiveMsg[cameraId].RobotFound[BLUE][i] = true;
	receiveMsg[cameraId].RobotPosX[BLUE][i] = x;
	receiveMsg[cameraId].RobotPosY[BLUE][i] = y;
	receiveMsg[cameraId].RobotRotation[BLUE][i] = orientation;
	receiveMsg[cameraId].RobotConfidence[BLUE][i] = confidence;
}

void CGlobalData::setYellowRobot(int cameraId, int i,int robotId,float x,float y,float orientation,float confidence)
{
	receiveMsg[cameraId].RobotINDEX[YELLOW][i] = robotId;//yellow
	receiveMsg[cameraId].RobotFound[YELLOW][i] = true;
	receiveMsg[cameraId].RobotPosX[YELLOW][i] = x;
	receiveMsg[cameraId].RobotPosY[YELLOW][i] = y;
	receiveMsg[cameraId].RobotRotation[YELLOW][i] = orientation;
	receiveMsg[cameraId].RobotConfidence[YELLOW][i] = confidence;
}

void CGlobalData::setBall(int cameraId, int i,float x, float y, float confidence)
{
	receiveMsg[cameraId].Ballx[i]= x;
	receiveMsg[cameraId].Bally[i] = y;
	receiveMsg[cameraId].BallConfidence[i] = confidence;
	receiveMsg[cameraId].BallFound[i] = true;
}

void CGlobalData::changeSmsgToRmsg(int id){
	if (smsg.BallFound==true)
	{
		setBall(id,0,smsg.Ballx,smsg.Bally,1);
	}	
	for (int i=0;i<SendCarNum;i++)
	{
		if (smsg.RobotFound[0][i]==true)
		{
			setBlueRobot(id,i,smsg.RobotINDEX[0][i]-1,smsg.RobotPosX[0][i],
				smsg.RobotPosY[0][i],smsg.RobotRotation[0][i]* 3.1415926 /180,1);
		}		
	}
	for (int i=0;i<SendCarNum;i++)
	{
		if (smsg.RobotFound[1][i]==true)
		{
			setYellowRobot(id,i,smsg.RobotINDEX[1][i]-1,smsg.RobotPosX[1][i],
				smsg.RobotPosY[1][i],smsg.RobotRotation[1][i]* 3.1415926 /180,1);
		}		
	}
}