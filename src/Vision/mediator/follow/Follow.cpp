#include "Follow.h"


Follow::Follow()
{
	initFollowArray();
}

void Follow::initFollowArray()
{
	for(int i = 0; i< MaxFollowNum; i++){
		followarray[i].AddFrame = 0;
		followarray[i].LostFrame = 5;
		followarray[i].Confidence = 0;
		followarray[i].Fresh = false;
		followarray[i].PosX = -ERRORNUM;
		followarray[i].PosY = -ERRORNUM;
		followarray[i].Rotation = -ERRORNUM;
		followarray[i].Num = MaxFollowNum;
		followarray[i].Send = false;
	}
}

void Follow::cancel(int minLostFrame, int num)
{
	followarray[num].AddFrame = 0;
	followarray[num].LostFrame = minLostFrame;
	followarray[num].Confidence = 0;
	followarray[num].Fresh = false;
	followarray[num].PosX = -ERRORNUM;
	followarray[num].PosY = -ERRORNUM;
	followarray[num].Rotation = -ERRORNUM;
	followarray[num].Num = MaxFollowNum;
	followarray[num].Send = false;
}

int Follow::getFollowNum(int num)
{
	for(int i = 0; i < MaxFollowNum; i++){
		if(followarray[i].Num == num && num != ReceiveCarNum)
			return i;
	}
	return -1;
}

bool Follow::FindInArray(int num)
{
	for(int i = 0; i < MaxFollowNum; i++){
		if(followarray[i].Num == num)
			return true;
		}
	return false;
}

double Follow::distances(float a0,float b0,float a1, float b1)
{
	double res=(double)(a0-a1)*(a0-a1)+(double)(b0-b1)*(b0-b1);
	return res;
}