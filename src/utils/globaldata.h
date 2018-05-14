#ifndef __GLOBAL_DATA__
#define __GLOBAL_DATA__
#include "singleton.hpp"
#include "dataqueue.hpp"
#include "messageformat.h"
#define maxLostFrame  30
enum ballState {received,touched,kicked,struggle,chip_pass,flat_pass};
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
    float montageMaxX;
    float montageMaxY;
    //int maxLostFrame;
    int lastTouch;
    ballState ballStateMachine;
};
typedef Singleton<CGlobalData> GlobalData;
#endif // __GLOBAL_DATA__
