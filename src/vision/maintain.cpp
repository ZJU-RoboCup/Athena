#include "maintain.h"
#include "globaldata.h"
#include "staticparams.h"
#include "collisiondetect.h"
#include "visionmodule.h"
#include "field.h"
#include "dealball.h"
namespace {
    int speedTrandferMagicNumber = 10;
    bool whetherTestRobotSpeed = false;
}
CMaintain::CMaintain():file("d:\\test.txt"),out(&file)
{
    detectionBall=detectionFrame.mutable_balls();
    if (::whetherTestRobotSpeed && !file.open(QIODevice::WriteOnly|QIODevice::Text)) {
        std::cout<<"Cannot Open"<<std::endl;
    }

}
CMaintain::~CMaintain(){
    if (::whetherTestRobotSpeed) file.close();
}

void CMaintain::udpSocketConnect(){
    std::string addressStr = "127.0.0.1";
    zeus_port=23333;
    client_port=54320;
    QString groupAddress = QString(addressStr.c_str());
//    udpSocket.bind(QHostAddress::AnyIPv4, zeus_port, QUdpSocket::ShareAddress);
//    udpSocket.joinMulticastGroup(QHostAddress(groupAddress));
}

void CMaintain::udpSocketDisconnect(){
    udpSocket.abort();
    client_udpSocket.abort();
}

void CMaintain::init(){
    result.init();
    result.addBall(GlobalData::instance()->processBall[0].ball[0]);
    for (int i=0;i<GlobalData::instance()->processRobot[0].robotSize[PARAM::BLUE];i++)
        result.addRobot(PARAM::BLUE,GlobalData::instance()->processRobot[0].robot[PARAM::BLUE][i]);
    for (int i=0;i<GlobalData::instance()->processRobot[0].robotSize[PARAM::YELLOW];i++)
        result.addRobot(PARAM::YELLOW,GlobalData::instance()->processRobot[0].robot[PARAM::YELLOW][i]);
    GlobalData::instance()->maintain.push(result);

}

void CMaintain::udpSend(){
    //udp start
    if (result.ballSize>0){
        detectionBall->set_x(result.ball[0].pos.x());
        if (result.ball[0].pos.y()==0) detectionBall->set_y(0.1);else detectionBall->set_y(result.ball[0].pos.y());//to fix a role match bug 2018.6.15
        CVector TransferVel(result.ball[0].velocity.x(),-result.ball[0].velocity.y());
        detectionBall->set_speed_mod(TransferVel.mod()/::speedTrandferMagicNumber);
        detectionBall->set_speed_direction(TransferVel.dir());
        detectionBall->set_valid(Dealball::instance()->getValid());
        detectionBall->set_next_x(result.ball[0].predict_pos.x());
        detectionBall->set_next_y(result.ball[0].predict_pos.y());
        detectionBall->set_last_touch(GlobalData::instance()->lastTouch);
        detectionBall->set_ball_state(GlobalData::instance()->ballStateMachine);
    }
    else
    {
        detectionBall->set_x(-32767);
        detectionBall->set_y(-32767);
        detectionBall->set_speed_mod(0);//todo
    }
    for (int i=0;i<result.robotSize[PARAM::BLUE];i++){
        if (i==PARAM::SENDROBOTNUM) break;//for sending MAX 8 car possible
        detectionRobot[PARAM::BLUE][i]=detectionFrame.add_robots_blue();
        detectionRobot[PARAM::BLUE][i]->set_x(result.robot[PARAM::BLUE][i].pos.x());
        detectionRobot[PARAM::BLUE][i]->set_y(result.robot[PARAM::BLUE][i].pos.y());
        detectionRobot[PARAM::BLUE][i]->set_orientation(result.robot[PARAM::BLUE][i].angel);
        detectionRobot[PARAM::BLUE][i]->set_robot_id(result.robot[PARAM::BLUE][i].id);
        detectionRobot[PARAM::BLUE][i]->set_confidence(1);
        if (::whetherTestRobotSpeed){
            out<<"Blue: "<<result.robot[PARAM::BLUE][i].id<<" "<<result.robot[PARAM::BLUE][i].velocity.mod()<<endl;
            out.flush();
        }
        CVector TransferVel(result.robot[PARAM::BLUE][i].velocity.x(),-result.robot[PARAM::BLUE][i].velocity.y());
        detectionRobot[PARAM::BLUE][i]->set_speed_mod(TransferVel.mod()/::speedTrandferMagicNumber);
        detectionRobot[PARAM::BLUE][i]->set_speed_direction(TransferVel.dir());
        detectionRobot[PARAM::BLUE][i]->set_next_x(result.robot[PARAM::BLUE][i].predict_pos.x());
        detectionRobot[PARAM::BLUE][i]->set_next_y(result.robot[PARAM::BLUE][i].predict_pos.y());
    }
    for (int i=0;i<result.robotSize[PARAM::YELLOW];i++){
        if (i==PARAM::SENDROBOTNUM) break;
        detectionRobot[PARAM::YELLOW][i]=detectionFrame.add_robots_yellow();
        detectionRobot[PARAM::YELLOW][i]->set_x(result.robot[PARAM::YELLOW][i].pos.x());
        detectionRobot[PARAM::YELLOW][i]->set_y(result.robot[PARAM::YELLOW][i].pos.y());
        detectionRobot[PARAM::YELLOW][i]->set_orientation(result.robot[PARAM::YELLOW][i].angel);
        detectionRobot[PARAM::YELLOW][i]->set_robot_id(result.robot[PARAM::YELLOW][i].id);
        detectionRobot[PARAM::YELLOW][i]->set_confidence(1);
        if (::whetherTestRobotSpeed){
            out<<"Yellow: "<<result.robot[PARAM::YELLOW][i].id<<" "<<result.robot[PARAM::YELLOW][i].velocity.mod()<<endl;
            out.flush();
        }
        CVector TransferVel(result.robot[PARAM::YELLOW][i].velocity.x(),-result.robot[PARAM::YELLOW][i].velocity.y());
        detectionRobot[PARAM::YELLOW][i]->set_speed_mod(TransferVel.mod()/::speedTrandferMagicNumber);
        detectionRobot[PARAM::YELLOW][i]->set_speed_direction(TransferVel.dir());
        detectionRobot[PARAM::YELLOW][i]->set_next_x(result.robot[PARAM::YELLOW][i].predict_pos.x());
        detectionRobot[PARAM::YELLOW][i]->set_next_y(result.robot[PARAM::YELLOW][i].predict_pos.y());
    }
    int size = detectionFrame.ByteSize();
    QByteArray buffer(size,0);
    detectionFrame.SerializeToArray(buffer.data(),buffer.size());
    GlobalData::instance()->ctrlCMutex.lock();
    bool sw = GlobalData::instance()->ctrlC;
    GlobalData::instance()->ctrlCMutex.unlock();
    if(!sw){
        udpSocket.writeDatagram(buffer.data(),buffer.size(),QHostAddress("127.0.0.1"),zeus_port+1);
        udpSocket.writeDatagram(buffer.data(),buffer.size(),QHostAddress("127.0.0.1"),zeus_port);
    }
    detectionFrame.clear_robots_blue();
    detectionFrame.clear_robots_yellow();
}

void CMaintain::run(){
    init();
    //CollisionDetect::instance()->visionAlart();
    //Ball Statemachine
    if (CollisionDetect::instance()->ballCloseEnough2Analyze(PARAM::BLUE) ||
            CollisionDetect::instance()->ballCloseEnough2Analyze(PARAM::YELLOW) ||
            CollisionDetect::instance()->ballIsOnEdge(GlobalData::instance()->maintain[0].ball[0].pos))
        //离车近，判断碰撞
    {
        CGeoPoint ballpos(GlobalData::instance()->maintain[0].ball[0].pos);
        for (int i=0;i<GlobalData::instance()->maintain[0].robotSize[PARAM::BLUE];i++){
            CGeoPoint tempos(GlobalData::instance()->maintain[0].robot[PARAM::BLUE][i].pos);
        }
        CollisionDetect::instance()->analyzeData();
    }
    else
    {
        //离车远，判断挑球
        CGeoLine line(GlobalData::instance()->maintain[0].ball[0].pos,GlobalData::instance()->maintain[-7].ball[0].pos);
        CGeoPoint middlePoint(GlobalData::instance()->maintain[-4].ball[0].pos);
        if(line.projection(middlePoint).dist(middlePoint)>1.0)
        //std::cout<<"now its chip dist="<<line.projection(middlePoint).dist(middlePoint)<<std::endl;
        if (line.projection(middlePoint).dist(middlePoint)>CHIP_DIS)
        {
            GlobalData::instance()->ballStateMachine=chip_pass;
        }
        else
            GlobalData::instance()->ballStateMachine=flat_pass;
    }
    //udp start
    udpSend();

}
