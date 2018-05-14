#include "visionmodule.h"
#include "globaldata.h"
#include "transform.h"
#include "modelfix.h"
#include "montage.h"
#include "maintain.h"
#include "dealball.h"
#include "dealrobot.h"
#include "messages_robocup_ssl_wrapper.pb.h"
#include "field.h"
#include "parammanager.h"
#include <QElapsedTimer>
#include <QtDebug>
namespace{
    auto zpm = ZSS::ZParamManager::instance();
}
CVisionModule::CVisionModule(QObject *parent)
    : QObject(parent)
    , udpSocket()
{
    //std::fill_n(cameraControl,PARAM::CAMERA,true);
    std::fill_n(GlobalData::instance()->cameraUpdate,PARAM::CAMERA,false);
    //cameraControl
    std::fill_n(GlobalData::instance()->cameraControl,PARAM::CAMERA,true);
    std::fill_n(GlobalData::instance()->processControl,3,true);
    zpm->loadParam(saoAction,"field/saoAction",0);
}
void CVisionModule::udpSocketConnect(){
    QString groupAddress;
    zpm->loadParam(groupAddress,"vision/address","224.5.23.2");
    zpm->loadParam(port,"vision/port",10005);
    udpSocket.bind(QHostAddress::AnyIPv4, port, QUdpSocket::ShareAddress);
    udpSocket.joinMulticastGroup(QHostAddress(groupAddress));
    zpm->loadParam(saoAction,"field/saoAction",0);
    connect(&udpSocket,SIGNAL(readyRead()),this,SLOT(storeData()),Qt::DirectConnection);
}
void CVisionModule::udpSocketDisconnect(){
    disconnect(&udpSocket,0,this,0);
    udpSocket.abort();
}
CGeoPoint CVisionModule::saoConvert(CGeoPoint originPoint){
    CGeoPoint result;
    switch (saoAction){
    case 0:
        result.setX(originPoint.x());
        result.setY(originPoint.y());
        break;
    case 1:
        result.setX(originPoint.y()+3000);
        result.setY(-originPoint.x());
        break;
    case 2:
        result.setX(originPoint.x() * 3 / 2);
        result.setY(originPoint.y() * 3 / 2);
        break;
    default:
        result.setX(originPoint.x());
        result.setY(originPoint.y());
        break;
    }
    return result;
}

double CVisionModule::saoConvert(double direction){
    if (saoAction == 1)
        return (direction-3.1415926/2);
    else
        return direction;
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
        int ballSize = detection.balls_size();
        int blueSize = detection.robots_blue_size();
        int yellowSize = detection.robots_yellow_size();
        for (int i = 0; i < ballSize; i++) {
            const SSL_DetectionBall& ball = detection.balls(i);
            if (Field::inChosenArea(saoConvert(CGeoPoint(ball.x(),ball.y())))){
                message.addBall(saoConvert(CGeoPoint(ball.x(),ball.y())));
            }
        }
        for (int i = 0; i < blueSize; i++) {
            const SSL_DetectionRobot& robot = detection.robots_blue(i);
            if (Field::inChosenArea(saoConvert(CGeoPoint(robot.x(),robot.y())))){
                message.addRobot(BLUE,robot.robot_id(),saoConvert(CGeoPoint(robot.x(),robot.y())),saoConvert(robot.orientation()));
            }
            //qDebug() << "BLUE : " << robot.robot_id() << robot.orientation();
        }
        for (int i = 0; i < yellowSize; i++) {
            const SSL_DetectionRobot& robot = detection.robots_yellow(i);
            if (Field::inChosenArea(robot.x(),robot.y())){
                message.addRobot(YELLOW,robot.robot_id(),saoConvert(CGeoPoint(robot.x(),robot.y())),saoConvert(robot.orientation()));
            }
            //qDebug() << "YELL : " << robot.robot_id() << robot.orientation();
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
    counter++;
    Dealball::instance()->run();
    Dealrobot::instance()->run();
    Maintain::instance()->run();
    return true;
}
quint16 CVisionModule::getFPS(){
    static QElapsedTimer timer;
    static bool ifStart = false;
    static quint64 lastCount;
    static quint16 result;
    if (!ifStart) {
        ifStart = true;
        timer.start();
        lastCount = counter;
        return 0;
    }
    result = (counter - lastCount)*1000.0/timer.restart();
    lastCount = counter;
    return result;
}
