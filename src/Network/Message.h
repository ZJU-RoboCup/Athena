#ifndef __MESSAGE_B_H__
#define __MESSAGE_B_H__

#include <param.h>
#include <utils.h>
#include <ServerInterface.h>
#include <OptionModule.h>
class Message
{
public:
	Message(COptionModule *pOption);
	~Message(void){};
	//void message2RawVisionData(RawVisionData &visionData);
	void message2VisionInfo(CServerInterface::VisualInfo& info);
public:
	unsigned long _cycle;
	bool _ballFound;
	float _ballx;
	float _bally;

	int _CameraID;
	int _ballImageX;
	int _ballImageY;

	unsigned char  RobotINDEX[2][Param::Field::MAX_PLAYER];
	bool  RobotFound[2][Param::Field::MAX_PLAYER];
	float RobotPosX[2][Param::Field::MAX_PLAYER];
	float RobotPosY[2][Param::Field::MAX_PLAYER];
	float RobotRotation[2][Param::Field::MAX_PLAYER];

	//unsigned char _ourRobotIndex[Param::Field::MAX_PLAYER];
	//bool _ourRobotFound[Param::Field::MAX_PLAYER];
	//float _ourRobotPosX[Param::Field::MAX_PLAYER];
	//float _ourRobotPosY[Param::Field::MAX_PLAYER];
	//float _ourRobotRotation[Param::Field::MAX_PLAYER];
	//unsigned char _enemyRobotIndex[Param::Field::MAX_PLAYER];
	//bool _enemyRobotFound[Param::Field::MAX_PLAYER];
	//float _enemyRobotPosX[Param::Field::MAX_PLAYER];
	//float _enemyRobotPosY[Param::Field::MAX_PLAYER];
	//float _enemyRobotRotation[Param::Field::MAX_PLAYER];
	
private:
	inline float msgX2InfoX(float x) { return (x/10.0); }
	inline float msgY2InfoY(float y) { return (-y/10.0); }
	inline float msgAngle2InfoAngle(float angle) { return -Utils::Deg2Rad(angle); }
};

#endif