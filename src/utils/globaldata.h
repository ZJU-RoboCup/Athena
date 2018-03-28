#ifndef __GLOBAL_DATA__
#define __GLOBAL_DATA__
#include "singleton.hpp"
#include "dataqueue.hpp"
#include "messageformat.h"
#include "immortals/Source/Reality/Vision/WorldState.h"
class CGlobalData{
public:
    CGlobalData();
    bool cameraUpdate[PARAM::CAMERA];
    bool cameraControl[PARAM::CAMERA];
    bool processControl[3];
    int cameraID[PARAM::CAMERA];//show the mapping of cameras  id
    double robotPossible[2][PARAM::ROBOTMAXID];
    CameraFix cameraFixMatrix[PARAM::CAMERA];
    SingleCamera cameraMatrix[PARAM::CAMERA];
    DataQueue<ReceiveVisionMessage> camera[PARAM::CAMERA];
    DataQueue<ReceiveVisionMessage> processBall;
    DataQueue<ReceiveVisionMessage> processRobot;
    DataQueue<ReceiveVisionMessage> maintain;
    DataQueue<ReceiveVisionMessage> transformed[PARAM::CAMERA];
    DataQueue<ReceiveVisionMessage> modelFixed[PARAM::CAMERA];
    DataQueue<OriginMessage> vision;
    WorldState immortalsVisionState;
    float montageMaxX;
    float montageMaxY;
    int maxLostFrame;
};

typedef Singleton<CGlobalData> GlobalData;
#endif // __GLOBAL_DATA__
