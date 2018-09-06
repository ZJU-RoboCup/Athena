#include "dealrobot.h"
#include "globaldata.h"
#include "staticparams.h"
#include "parammanager.h"
#include "matrix2d.h"
#include <iostream>
#include <qdebug.h>
#define FRAME_RATE 60
namespace{
    auto zpm = ZSS::ZParamManager::instance();
    int DIFF_VECHILE_MAX_DIFF = 600;//1000
}
CDealrobot::CDealrobot()
{
    zpm->loadParam(fieldWidth,"field/canvasWidth",13200);
    zpm->loadParam(fieldHeight,"field/canvasHeight",9900);
    zpm->loadParam(minBelieveFrame,"AlertFusion/carMinBelieveFrame",3);
    zpm->loadParam(maxLostFrame,"AlertFusion/carMaxLostFrame",15);
    //minBelieveFrame=3;
    //maxLostFrame=15;
    upPossible=0.1;
    decidePossible=minBelieveFrame* upPossible;
    downPossible =(1.0 -decidePossible)/maxLostFrame;
}

double CDealrobot::posDist(CGeoPoint pos1, CGeoPoint pos2)
{
    return std::sqrt((pos1.x() - pos2.x())*(pos1.x() -pos2.x())+(pos1.y() -pos2.y())*((pos1.y() -pos2.y())));
}
bool CDealrobot::isOnField(CGeoPoint p){
    if (p.x()<fieldWidth/2 && p.x()>-fieldWidth/2 && p.y()<fieldHeight/2 && p.y()>-fieldHeight/2)
        return true;
    else
        return false;
}
double CDealrobot::calculateWeight(int camID,CGeoPoint roboPos){
    SingleCamera camera=GlobalData::instance()->cameraMatrix[camID];
    if (roboPos.x()>camera.leftedge.max && roboPos.x()<camera.rightedge.max &&
            roboPos.y()>camera.downedge.max && roboPos.y()<camera.upedge.max)
        return 1;
    else if (roboPos.x()<camera.leftedge.max && roboPos.x()>camera.leftedge.min)
        return abs(roboPos.x()-camera.leftedge.min)/abs(camera.leftedge.max-camera.leftedge.min);
    else if (roboPos.x()>camera.rightedge.max && roboPos.x()<camera.rightedge.min)
        return abs(roboPos.x()-camera.rightedge.min)/abs(camera.rightedge.max-camera.rightedge.min);
    else if (roboPos.y()<camera.downedge.max && roboPos.y()>camera.downedge.min)
        return abs(roboPos.y()-camera.downedge.min)/abs(camera.downedge.max-camera.downedge.min);
    else if (roboPos.y()>camera.upedge.max && roboPos.y()<camera.upedge.min)
        return abs(roboPos.y()-camera.upedge.min)/abs(camera.upedge.max-camera.upedge.min);
    else return 0.01;//to deal with can see out of border situation
}
void CDealrobot::init(){
    zpm->loadParam(filteDir,"Vision/FilteDirection",false);
    result.init();
    for (int roboId=0; roboId<PARAM::ROBOTMAXID;roboId++)
        for (int camId=0;camId<PARAM::CAMERA;camId++){
           robotSeqence[PARAM::BLUE][roboId][camId].fill(-1,-32767,-32767,0);
           robotSeqence[PARAM::YELLOW][roboId][camId].fill(-1,-32767,-32767,0);
        }

    for (int i=0;i<PARAM::CAMERA;i++){
        //if (GlobalData::instance()->cameraControl[i]==true)
        {
            for (int j=0;j<GlobalData::instance()->camera[i][0].robotSize[PARAM::BLUE];j++){
                Robot robot=GlobalData::instance()->camera[i][0].robot[PARAM::BLUE][j];
                if ( GlobalData::instance()->robotPossible[PARAM::BLUE][robot.id]<decidePossible)
                     robotSeqence[PARAM::BLUE][robot.id][i]=robot;
                else if  (lastRobot[PARAM::BLUE][robot.id].pos.dist(robot.pos)<DIFF_VECHILE_MAX_DIFF)
                    robotSeqence[PARAM::BLUE][robot.id][i]=robot;
            }
            for (int j=0;j<GlobalData::instance()->camera[i][0].robotSize[PARAM::YELLOW];j++){
                Robot robot=GlobalData::instance()->camera[i][0].robot[PARAM::YELLOW][j];
                if ( GlobalData::instance()->robotPossible[PARAM::YELLOW][robot.id]<decidePossible)
                     robotSeqence[PARAM::YELLOW][robot.id][i]=robot;
                else if  (lastRobot[PARAM::YELLOW][robot.id].pos.dist(robot.pos)<DIFF_VECHILE_MAX_DIFF)
                    robotSeqence[PARAM::YELLOW][robot.id][i]=robot;
            }
        }
    }
    for (int i=0;i<PARAM::ROBOTMAXID-1;i++){
        Robot temp(-32767,-32767,0,-1);
        sortTemp[PARAM::BLUE][i]=temp;
        sortTemp[PARAM::YELLOW][i]=temp;
    }
}

void CDealrobot::MergeRobot(){
    for (int roboId=0; roboId<PARAM::ROBOTMAXID;roboId++){
        bool foundBlue=false,foundYellow=false;
        double blueWeight=0,yellowWeight=0;
        CGeoPoint blueAverage(0,0),yellowAverage(0,0);
        double blueAngle=0,yellowAngle=0;
        for (int camId=0;camId<PARAM::CAMERA;camId++)
        {
            SingleCamera camera=GlobalData::instance()->cameraMatrix[camId];
            double _weight=0;
            if(robotSeqence[PARAM::BLUE][roboId][camId].pos.x() > -30000 && robotSeqence[PARAM::BLUE][roboId][camId].pos.y() > -30000)
            {
                foundBlue=true;
                _weight=calculateWeight(camId,robotSeqence[PARAM::BLUE][roboId][camId].pos);//std::pow(posDist(robotSeqence[PARAM::BLUE][roboId][camId].pos,GlobalData::instance()->cameraMatrix[camera.id].campos)/100.0,-2.0);//
                blueWeight+=_weight;
                blueAverage.setX(blueAverage.x() + robotSeqence[PARAM::BLUE][roboId][camId].pos.x() * _weight);
                blueAverage.setY(blueAverage.y() + robotSeqence[PARAM::BLUE][roboId][camId].pos.y() * _weight);
                blueAngle=robotSeqence[PARAM::BLUE][roboId][camId].angel;
                //std::cout<<"  weight:"<<_weight<<" "<<robotSeqence[PARAM::BLUE][roboId][camId].pos.x <<"\t";
            }
            _weight=0;
            if(robotSeqence[PARAM::YELLOW][roboId][camId].pos.x() >-30000 && robotSeqence[PARAM::YELLOW][roboId][camId].pos.y() >-30000)
            {
                foundYellow=true;
                _weight=calculateWeight(camId,robotSeqence[PARAM::YELLOW][roboId][camId].pos);//std::pow(posDist(robotSeqence[PARAM::YELLOW][roboId][camId].pos,GlobalData::instance()->cameraMatrix[camera.id].campos)/100.0,-2.0);
                yellowWeight+=_weight;
                yellowAverage.setX(yellowAverage.x() + robotSeqence[PARAM::YELLOW][roboId][camId].pos.x() * _weight);
                yellowAverage.setY(yellowAverage.y() + robotSeqence[PARAM::YELLOW][roboId][camId].pos.y() * _weight);
                yellowAngle=robotSeqence[PARAM::YELLOW][roboId][camId].angel;
                //std::cout<<_weight<<" "<<robotSeqence[PARAM::YELLOW][roboId][camId].angel <<"\t";
           }
        }
        //std::cout<<"\n";
        if (foundBlue){
            Robot ave(blueAverage.x() / blueWeight,blueAverage.y() / blueWeight,blueAngle,roboId);
            result.addRobot(PARAM::BLUE,ave);
        }
        if (foundYellow){
            Robot ave(yellowAverage.x() / yellowWeight,yellowAverage.y() / yellowWeight,yellowAngle,roboId);
            result.addRobot(PARAM::YELLOW,ave);
        }
    }
    if (PARAM::DEBUG) std::cout<<"have found "<<result.robotSize[PARAM::BLUE]<<"blue car.\t"<<result.robotSize[PARAM::YELLOW]<<std::endl;
    //GlobalData::instance()->processRobot.push(result);
}



void CDealrobot::sortRobot(int color){
    for (int id=0;id<PARAM::ROBOTMAXID;id++){
            bool found=false;
            for (int i=0;i<result.robotSize[color];i++)
                if (result.robot[color][i].id==id ) {
                    if ((isOnField(result.robot[color][i].pos)) &&
                    (GlobalData::instance()->robotPossible[color][id]<decidePossible ||result.robot[color][i].pos.dist(lastRobot[color][id].pos)<DIFF_VECHILE_MAX_DIFF))
                    {
                        lastRobot[color][id]=result.robot[color][i];
                        found=true;
                    }
                }
//            if (id==6 && color==1) std::cout<<decidePossible<<downPossible<<"id"<<lastRobot[color][id].id<<" found:"<<found<<"before  possible:"<<GlobalData::instance()->robotPossible[color][id]<<std::endl<<
//                                           lastRobot[color][id].pos<<std::endl;
            if (found)  GlobalData::instance()->robotPossible[color][id]+=upPossible;
            else if (GlobalData::instance()->robotPossible[color][id]>=decidePossible) GlobalData::instance()->robotPossible[color][id]-=downPossible;
            else GlobalData::instance()->robotPossible[color][id]-=decidePossible;
            if (GlobalData::instance()->robotPossible[color][id]>1.0) GlobalData::instance()->robotPossible[color][id]=1.0;
            if(GlobalData::instance()->robotPossible[color][id]<0.0)  GlobalData::instance()->robotPossible[color][id]=0.0;
//            if (id==6 && color==1) std::cout<<"After possible:"<<lastRobot[color][id].id<<" "<<GlobalData::instance()->robotPossible[color][id]<<std::endl;
        }
    validNum[color]=0;
    //Check for a robot be identify to 2 robot ,NOT HELPIFUL 6.13
//    for (int i=0;i<PARAM::ROBOTMAXID-1;i++)
//        for (int j=i+1;j<PARAM::ROBOTMAXID;j++)
//        if (GlobalData::instance()->robotPossible[color][i]>0 && GlobalData::instance()->robotPossible[color][j]>0 &&
//            lastRobot[color][i].pos.dist(lastRobot[color][j].pos)<50){          //有概率加入，并且位置重叠。选取概率大的那个
//            std::cout<<"FOUND 2 ROBOT coincide!!"<<i<<" "<<j<<" "<<lastRobot[color][i].pos.dist(lastRobot[color][j].pos)<<std::endl;;
//            if (GlobalData::instance()->robotPossible[color][i]>GlobalData::instance()->robotPossible[color][j])
//                GlobalData::instance()->robotPossible[color][j]=0;
//            else GlobalData::instance()->robotPossible[color][i]=0;
//        }

    for (int id=0;id<PARAM::ROBOTMAXID;id++)
        if(GlobalData::instance()->robotPossible[color][id]>=decidePossible) sortTemp[color][validNum[color]++]=lastRobot[color][id];


    //sort
    for (int i=0;i<validNum[color]-1;i++)
    {
        int maxj=i;
        for (int j=i+1;j<validNum[color];j++)
            if (GlobalData::instance()->robotPossible[color][sortTemp[color][maxj].id]<
                GlobalData::instance()->robotPossible[color][sortTemp[color][j].id]) maxj=j;
        if (maxj!=i){
            Robot temp;
            temp=sortTemp[color][maxj];
            sortTemp[color][maxj]=sortTemp[color][i];
            sortTemp[color][i]=temp;
        }
    }

}
void CDealrobot::run(){
    init();
    MergeRobot();
    sortRobot(PARAM::BLUE);
    sortRobot(PARAM::YELLOW);
    result.init();
    //重新加入概率排序后的车
    for (int i=0;i<validNum[PARAM::BLUE];i++)
            result.addRobot(PARAM::BLUE,sortTemp[PARAM::BLUE][i]);
    for (int i=0;i<validNum[PARAM::YELLOW];i++)
            result.addRobot(PARAM::YELLOW,sortTemp[PARAM::YELLOW][i]);
    //滤波和线性预测
    //cancel in 2018.6.12
    for (int i=0;i<validNum[PARAM::BLUE];i++)
    {
        Robot & robot=result.robot[PARAM::BLUE][i];
        auto & tempMatrix = _kalmanFilter[PARAM::BLUE][robot.id].update(robot.pos.x(),robot.pos.y());
        CGeoPoint filtPoint (tempMatrix(0,0),tempMatrix(1,0));
        robot.pos=filtPoint;
//        result.robot[PARAM::BLUE][i].velocity.setVector(tempMatrix(2, 0)*FRAME_RATE, tempMatrix(3, 0)*FRAME_RATE);
//        CGeoPoint predictPos=filtPoint+robot.velocity;
//        result.robot[PARAM::BLUE][i].fillPredictPos(predictPos);
//        if (filteDir){
//            robot.angel = _dirFilter[PARAM::BLUE][robot.id].update(robot.angel);
//        }
    }
    for (int i=0;i<validNum[PARAM::YELLOW];i++)
    {
        Robot & robot=result.robot[PARAM::YELLOW][i];
        auto & tempMatrix = _kalmanFilter[PARAM::YELLOW][robot.id].update(robot.pos.x(),robot.pos.y());
        CGeoPoint filtPoint (tempMatrix(0,0),tempMatrix(1,0));
        robot.pos=filtPoint;
//        result.robot[PARAM::YELLOW][i].velocity.setVector(tempMatrix(2, 0)*FRAME_RATE, tempMatrix(3, 0)*FRAME_RATE);
//        CGeoPoint predictPos=filtPoint+robot.velocity;
//        result.robot[PARAM::YELLOW][i].fillPredictPos(predictPos);
//        if (filteDir){
//            robot.angel = _dirFilter[PARAM::YELLOW][robot.id].update(robot.angel);
//        }
    }
    // filteRobot(result.robot[PARAM::YELLOW][i]);
    GlobalData::instance()->processRobot.push(result);
}
