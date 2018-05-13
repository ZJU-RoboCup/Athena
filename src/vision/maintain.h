#ifndef MAINTAIN_H
#define MAINTAIN_H
#include "singleton.hpp"
#include "vision_detection.pb.h"
#include <messageformat.h>
#include <QUdpSocket>

class CMaintain
{
public:
    CMaintain();
    void run();
    void init();
    void ChipPrediction();
    void StateMachine();
    void udpSocketConnect();
    void udpSocketDisconnect();

private:
    ReceiveVisionMessage result;
    QUdpSocket udpSocket;
    QUdpSocket client_udpSocket;
    QHostAddress groupAddress;
    int interface;
    int zeus_port;
    int client_port;
    Vision_DetectionFrame detectionFrame;
    Vision_DetectionBall* detectionBall;
    Vision_DetectionRobot* detectionRobot[2][PARAM::ROBOTMAXID];

};
typedef Singleton<CMaintain> Maintain;

#endif // MAINTAIN_H
