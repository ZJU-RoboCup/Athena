#ifndef _TRANSMIT_H_
#define _TRANSMIT_H_

#include "message.h"
#include "follow/BallFollow.h"
#include "follow/RobotFollow.h"
#include "follow/TwoCameraMixer.h"

typedef unsigned char BYTE;
const int MixPartNum=2;


class Transmit
{
public:
	Transmit();
	~Transmit();
	void initial();
	void initSmsg();
	void setCycle(int, int);
	SendVisionMessage smsgUpdate(int cameraMode,int minAddFrame, int minLostFrame,float maxdist,bool isFalseSend);
	void setFirstStart();
	void initRmsgArray();
	void usortCounter();
private:
	TwoCameraMixer _cameraMixer[MixPartNum];
	TwoCameraMixer _lastCameraMixer;

};
#endif
