#include "TwoCameraMixer.h"

TwoCameraMixer::TwoCameraMixer(int cameraId1,int cameraId2)
{
	_cameraId1=cameraId1;_cameraId2=cameraId2;
	blueFollow = RobotFollow(BLUE,cameraId1,cameraId2);
	yellowFollow = RobotFollow(YELLOW,cameraId1,cameraId2);
	ballFollow = BallFollow(cameraId1,cameraId2);
}
void TwoCameraMixer::compare( int minAddFrame )
{
	blueFollow.compare(minAddFrame);
	yellowFollow.compare(minAddFrame);
}

void TwoCameraMixer::usort()
{
	blueFollow.Usort();
	yellowFollow.Usort();
}

void TwoCameraMixer::followBallSingle(int court, int minAddFrame, int minLostFrame, float maxdist)
{
	ballFollow.startFollowSingle(court, minAddFrame, minLostFrame, maxdist);
}

void TwoCameraMixer::followRobotSingle(int court, int minAddFrame, int minLostFrame, float maxdist)
{
	blueFollow.startFollowSingle(court, minAddFrame, minLostFrame, maxdist);
	yellowFollow.startFollowSingle(court, minAddFrame, minLostFrame, maxdist);
}
void TwoCameraMixer::followBallDouble( int minAddFrame, int minLostFrame, float maxdist)
{
	ballFollow.startFollowDouble( minAddFrame, minLostFrame, maxdist);
}

void TwoCameraMixer::followRobotDouble( int minAddFrame, int minLostFrame, float maxdist)
{
	blueFollow.startFollowDouble(minAddFrame, minLostFrame, maxdist);
	yellowFollow.startFollowDouble( minAddFrame, minLostFrame, maxdist);
}

void TwoCameraMixer::initSmsg()
{
	ballFollow.initSmsg();
	blueFollow.initSmsg();
	yellowFollow.initSmsg();
}

void TwoCameraMixer::updateMixMsg(int minAddFrame, int minLostFrame, bool isFalseSend)
{
	ballFollow.determinesend(minAddFrame, minLostFrame, isFalseSend);
	blueFollow.determinesend(minAddFrame,minLostFrame,isFalseSend);
	yellowFollow.determinesend(minAddFrame,minLostFrame,isFalseSend);
}

void TwoCameraMixer::reInitFollow()
{
	for(int i = 0; i < ReceiveCarNum; i++){
		blueFollow.followarray[i].Fresh = false;
		yellowFollow.followarray[i].Fresh = false;
	}
	for(int i = 0; i < MaxBallNum; i++){
		ballFollow.followarray[i].Fresh = false;
	}
}

SendVisionMessage TwoCameraMixer::mixAlgorithm(int cameraMode,int minAddFrame, int minLostFrame, float maxdist,int isFalseSend)
{
	switch(cameraMode){
	case TwoCamUp:  case TwoCamDown:  case FourCamera:
		followRobotDouble( minAddFrame, minLostFrame, maxdist);
		followBallDouble( minAddFrame, minLostFrame, maxdist);
		break;
	case SingleCamLeftUp:
		followRobotSingle(LeftUp, minAddFrame, minLostFrame, maxdist);
		followBallSingle(LeftUp, minAddFrame, minLostFrame, maxdist);
		break;
	case SingleCamRightUp:
		followRobotSingle(RightUp, minAddFrame, minLostFrame, maxdist);
		followBallSingle(RightUp, minAddFrame, minLostFrame, maxdist);
		break;
	case SingleCamLeftDown:
		followRobotSingle(LeftDown, minAddFrame, minLostFrame, maxdist);
		followBallSingle(LeftDown, minAddFrame, minLostFrame, maxdist);
		break;
	case SingleCamRightDown:
		followRobotSingle(RightDown, minAddFrame, minLostFrame, maxdist);
		followBallSingle(RightDown, minAddFrame, minLostFrame, maxdist);
		break;
	default:
		break;
	}
	initSmsg();
	compare(minAddFrame);
	if(GlobalData::Instance()->firstStart){
		usort();
	}
	updateMixMsg(minAddFrame,minLostFrame,isFalseSend);
	reInitFollow();
	return GlobalData::Instance()->smsg;
}