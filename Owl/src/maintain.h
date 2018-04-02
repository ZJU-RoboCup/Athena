#ifndef MAINTAIN_H
#define MAINTAIN_H
#include "singleton.hpp"
#include "proto/cpp/vision_detection.pb.h"
#include <messageformat.h>
#include <QUdpSocket>

class CMaintain
{
public:
    CMaintain();
    void run (bool);
    void init();
    //void CollisionDetection();
    void ChipPrediction();
    void StateMachine();
    void udpSocketConnect();
    void udpSocketDisconnect();

private:
    ReceiveVisionMessage result;
    QUdpSocket udpSocket;
    QHostAddress groupAddress;
    int interface;
    int port;
    Vision_DetectionFrame detectionFrame;
    Vision_DetectionBall* detectionBall;
    Vision_DetectionRobot* detectionRobot[2][PARAM::ROBOTMAXID];

};
typedef Singleton<CMaintain> Maintain;

#endif // MAINTAIN_H
