#ifndef __GLOBAL_DATA__
#define __GLOBAL_DATA__
#include "singleton.hpp"
#include "dataqueue.hpp"
#include "messageformat.h"
#include <QMutex>
enum ballState {received,touched,kicked,struggle,chip_pass,flat_pass};
class CGlobalData{
public:
    CGlobalData();
    void setCameraMatrix(bool);
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
    int lastTouch;//Be attention it's id!!!
    ballState ballStateMachine;
    QByteArray debugMessages;
    QMutex debugMutex;// debugMessages;
    bool ctrlC;
    QMutex ctrlCMutex;

private:
    CGeoPoint saoConvert(CGeoPoint);
    void  saoConvertEdge();
    int saoAction;
};
typedef Singleton<CGlobalData> GlobalData;
#endif // __GLOBAL_DATA__
