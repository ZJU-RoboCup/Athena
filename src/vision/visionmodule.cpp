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
#include "globalsettings.h"
#include <QElapsedTimer>
#include <QtDebug>
namespace{
    auto zpm = ZSS::ZParamManager::instance();
    auto vpm =ZSS::VParamManager::instance();
    bool IF_EDGE_TEST= false;
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
}
void CVisionModule::udpSocketConnect(bool real){
    real ? zpm->loadParam(port,"AlertPorts/Vision4Real",10005) : zpm->loadParam(port,"AlertPorts/Vision4Sim",10020);
    zpm->loadParam(saoAction,"Alert/SaoAction",0);
    GlobalData::instance()->setCameraMatrix(real);
    qDebug() << "VisionPort : " << port;
    udpSocket.bind(QHostAddress::AnyIPv4, port, QUdpSocket::ShareAddress);
    udpSocket.joinMulticastGroup(QHostAddress(ZSS::SSL_ADDRESS));
    connect(&udpSocket,SIGNAL(readyRead()),this,SLOT(storeData()),Qt::DirectConnection);
}
void CVisionModule::udpSocketDisconnect(){
    if (IF_EDGE_TEST){
        for (int i=0;i<PARAM::CAMERA;i++){
            SingleCamera& currentCamera = GlobalData::instance()->cameraMatrix[i];
            vpm->changeParam("Camera"+QString::number(i)+"Leftmin",currentCamera.leftedge.min);
            vpm->changeParam("Camera"+QString::number(i)+"Leftmax",currentCamera.leftedge.max);
            vpm->changeParam("Camera"+QString::number(i)+"Rightmin",currentCamera.rightedge.min);
            vpm->changeParam("Camera"+QString::number(i)+"Rightmax",currentCamera.rightedge.max);
            vpm->changeParam("Camera"+QString::number(i)+"Upmin",currentCamera.upedge.min);
            vpm->changeParam("Camera"+QString::number(i)+"Upmax",currentCamera.upedge.max);
            vpm->changeParam("Camera"+QString::number(i)+"Downmin",currentCamera.downedge.min);
            vpm->changeParam("Camera"+QString::number(i)+"Downmax",currentCamera.downedge.max);
        }
    }
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
            //std::cout<<"camera id:"<<message.camID<<" pos:"<<ball.x()<<" "<<ball.y()<<std::endl;
            if (GlobalSettings::instance()->inChosenArea(saoConvert(CGeoPoint(ball.x(),ball.y())))){
                message.addBall(saoConvert(CGeoPoint(ball.x(),ball.y())));
            }
        }
        for (int i = 0; i < blueSize; i++) {
            const SSL_DetectionRobot& robot = detection.robots_blue(i);
            if (GlobalSettings::instance()->inChosenArea(saoConvert(CGeoPoint(robot.x(),robot.y())))
                    && robot.robot_id()<12){
                message.addRobot(BLUE,robot.robot_id(),saoConvert(CGeoPoint(robot.x(),robot.y())),saoConvert(robot.orientation()));
            }
            //qDebug() << "BLUE : " << robot.robot_id() << robot.orientation();
        }
        for (int i = 0; i < yellowSize; i++) {
            const SSL_DetectionRobot& robot = detection.robots_yellow(i);
            if (GlobalSettings::instance()->inChosenArea(saoConvert(CGeoPoint(robot.x(),robot.y())))
                    && robot.robot_id()<12){
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
    if (IF_EDGE_TEST) edgeTest();
    Dealball::instance()->run();
    Dealrobot::instance()->run();
    Maintain::instance()->run();
    return true;
}

void CVisionModule::edgeTest(){
    for (int i=0;i<PARAM::CAMERA;i++){
        if (GlobalData::instance()->cameraControl[i]==true){
            SingleCamera& currentCamera = GlobalData::instance()->cameraMatrix[i];
            for(int j=0;j<GlobalData::instance()->camera[i][0].ballSize;j++)
            {
                Ball currentball=GlobalData::instance()->camera[i][0].ball[j];
                if (currentball.pos.x()<currentCamera.leftedge.min) currentCamera.leftedge.min=currentball.pos.x();
                if (currentball.pos.x()>currentCamera.rightedge.min) currentCamera.rightedge.min=currentball.pos.x();
                if (currentball.pos.y()>currentCamera.upedge.min) currentCamera.upedge.min=currentball.pos.y();
                if (currentball.pos.y()<currentCamera.downedge.min) currentCamera.downedge.min=currentball.pos.y();
            }
        }
    }
    for (int i=0;i<PARAM::CAMERA;i++){
        if (GlobalData::instance()->cameraControl[i]==true){
            SingleCamera& currentCamera = GlobalData::instance()->cameraMatrix[i];
            currentCamera.leftedge.max=currentCamera.leftedge.min;
            currentCamera.rightedge.max=currentCamera.rightedge.min;
            currentCamera.upedge.max=currentCamera.upedge.min;
            currentCamera.downedge.max=currentCamera.downedge.min;
            for (int j=0;j<PARAM::CAMERA;j++)
            if(GlobalData::instance()->cameraControl[j]==true && i!=j)
            {
                SingleCamera otherCamera = GlobalData::instance()->cameraMatrix[j];
                if (currentCamera.leftedge.max<otherCamera.rightedge.min && currentCamera.campos.x()>otherCamera.rightedge.min) currentCamera.leftedge.max=otherCamera.rightedge.min;
                if (currentCamera.rightedge.max>otherCamera.leftedge.min && currentCamera.campos.x()<otherCamera.leftedge.min) currentCamera.rightedge.max=otherCamera.leftedge.min;
                if (currentCamera.upedge.max>otherCamera.downedge.min && currentCamera.campos.y()<otherCamera.downedge.min) currentCamera.upedge.max=otherCamera.downedge.min;
                if (currentCamera.downedge.max<otherCamera.upedge.min && currentCamera.campos.y()>otherCamera.upedge.min) currentCamera.downedge.max=otherCamera.upedge.min;
            }
        }
    }
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
