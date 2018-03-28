#include "visionmodule.h"
#include "singleparams.h"
#include "globaldata.h"

#include "transform.h"
#include "modelfix.h"
#include "montage.h"
#include "maintain.h"
#include "dealball.h"
#include "dealrobot.h"
#include "immortals/immortalsvision.h"
#include "proto/cpp/messages_robocup_ssl_wrapper.pb.h"  // what for?
#include <QtDebug>
CVisionModule::CVisionModule(QObject *parent)
    : QObject(parent)
    , udpSocket(){
    //std::fill_n(cameraControl,PARAM::CAMERA,true);
    std::fill_n(GlobalData::instance()->cameraUpdate,PARAM::CAMERA,false);
    //cameraControl
    std::fill_n(GlobalData::instance()->cameraControl,PARAM::CAMERA,true);
    std::fill_n(GlobalData::instance()->processControl,3,true);
}
void CVisionModule::udpSocketConnect(){
    std::string addressStr = SingleParams::instance()->_("vision.address");
    port = SingleParams::instance()->_("vision.port.big");
    groupAddress = QString(addressStr.c_str());
    udpSocket.bind(QHostAddress::AnyIPv4, port, QUdpSocket::ShareAddress);
    udpSocket.joinMulticastGroup(QHostAddress(groupAddress));
    connect(&udpSocket,SIGNAL(readyRead()),this,SLOT(storeData()),Qt::DirectConnection);
}
void CVisionModule::udpSocketDisconnect(){
    disconnect(&udpSocket,0,this,0);
    udpSocket.abort();
}
void CVisionModule::storeData(){
    static QByteArray datagram;
    while (udpSocket.hasPendingDatagrams()) {
        datagram.resize(udpSocket.pendingDatagramSize());
        udpSocket.readDatagram(datagram.data(), datagram.size());
        parse((void*)datagram.data(),datagram.size());
    }
    if (collectNewVision()) {
        std::fill_n(GlobalData::instance()->cameraUpdate,PARAM::CAMERA,false);
        dealWithData();
        immortalsVision();
        emit needDraw();
    }
}
void CVisionModule::parse(void * ptr,int size){
    static SSL_WrapperPacket packet;
    ReceiveVisionMessage message;
    packet.ParseFromArray(ptr,size);
    if (packet.has_detection()) {
        const SSL_DetectionFrame& detection = packet.detection();
        message.camID = detection.camera_id();
        // add for ImmortalsVision
        ImmortalsVision::instance()->frame[message.camID] = packet.detection();
        // end of ImmortalsVision
        int ballSize = detection.balls_size();
        int blueSize = detection.robots_blue_size();
        int yellowSize = detection.robots_yellow_size();
        for (int i = 0; i < ballSize; i++) {
            const SSL_DetectionBall& ball = detection.balls(i);
            message.addBall(ball.x(),ball.y());
        }
        for (int i = 0; i < blueSize; i++) {
            const SSL_DetectionRobot& robot = detection.robots_blue(i);
            message.addRobot(BLUE,robot.robot_id(),robot.x(),robot.y(),robot.orientation());
        }
        for (int i = 0; i < yellowSize; i++) {
            const SSL_DetectionRobot& robot = detection.robots_yellow(i);
            message.addRobot(YELLOW,robot.robot_id(),robot.x(),robot.y(),robot.orientation());
        }
        GlobalData::instance()->camera[message.camID].push(message);
        GlobalData::instance()->cameraUpdate[message.camID] = true;
    }
}
bool CVisionModule::collectNewVision(){
    for (int i=0;i<PARAM::CAMERA;i++) {
        if (GlobalData::instance()->cameraControl[i] && !GlobalData::instance()->cameraUpdate[i])
            return false;
    }
    return true;
}

bool CVisionModule::dealWithData(){
    Dealball::instance()->run(GlobalData::instance()->processControl[0]);
    Dealrobot::instance()->run(GlobalData::instance()->processControl[1]);
    Maintain::instance()->run(GlobalData::instance()->processControl[2]);
    return true;
}
bool CVisionModule::immortalsVision(){
    ImmortalsVision::instance()->ProcessVision(&GlobalData::instance()->immortalsVisionState);
    return true;//whattt?
}
