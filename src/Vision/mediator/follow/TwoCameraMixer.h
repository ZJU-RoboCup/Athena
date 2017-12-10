#ifndef _TWO_CAMERA_MIXER_
#define _TWO_CAMERA_MIXER_

#include "BallFollow.h"
#include "RobotFollow.h"

class TwoCameraMixer
{
public:
	TwoCameraMixer(){}
	TwoCameraMixer(int,int);
	void compare(int minAddFrame);
	void usort();
	void followRobotSingle(int court, int minAddFrame, int minLostFrame, float maxdist);
	void followBallSingle( int camera, int minAddFrame, int minLostFrame, float maxdist);
	void followRobotDouble( int minAddFrame, int minLostFrame, float maxdist);
	void followBallDouble( int minAddFrame, int minLostFrame, float maxdist);
	void initSmsg();
	void updateMixMsg(int minAddFrame, int minLostFrame, bool isFalseSend);
	void reInitFollow();
	SendVisionMessage mixAlgorithm(int cameraMode,int minAddFrame, int minLostFrame, float maxdist,int isFalseSend);
private:
	BallFollow ballFollow;
	RobotFollow yellowFollow;
	RobotFollow blueFollow;
	int _cameraId1,_cameraId2;
};

#endif