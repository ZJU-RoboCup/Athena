#include "dealrobot.h"
#include "globaldata.h"
#include "staticparams.h"
#include "matrix2d.h"
#include <iostream>
#include <qdebug.h>
#define FRAME_RATE 60

CDealrobot::CDealrobot()
{
    for(int i=0;i<PARAM::ROBOTMAXID;i++){
        GlobalData::instance()->robotPossible[PARAM::BLUE][i]=0;
        GlobalData::instance()->robotPossible[PARAM::YELLOW][i]=0;
    }
}

double CDealrobot::posDist(CGeoPoint pos1, CGeoPoint pos2)
{
    return std::sqrt((pos1.x() - pos2.x())*(pos1.x() -pos2.x())+(pos1.y() -pos2.y())*((pos1.y() -pos2.y())));
}

void CDealrobot::MergeRobot(){
    for (int roboId=0; roboId<PARAM::ROBOTMAXID;roboId++){
        bool foundBlue=false,foundYellow=false;
        double blueWeight=0,yellowWeight=0;
        CGeoPoint blueAverage(0,0),yellowAverage(0,0);
        double blueAngle=0,yellowAngle=0;
        //std::cout<<"roboID:"<<roboId<<" : ";
        for (int camId=0;camId<PARAM::CAMERA;camId++)
        {
            SingleCamera camera=GlobalData::instance()->cameraMatrix[camId];
            double _weight=0;
            if(robotSeqence[PARAM::BLUE][roboId][camId].pos.x() > -30000 && robotSeqence[PARAM::BLUE][roboId][camId].pos.y() > -30000 && (!foundBlue ||
              (foundBlue && robotSeqence[PARAM::BLUE][roboId][camId].pos.dist(CGeoPoint(blueAverage.x() /blueWeight,blueAverage.y() /blueWeight))<PARAM::ROBOTMERGEDOSTANCE)))
            {
                //std::cout<<"blueFound:"<<foundBlue;
                foundBlue=true;
                _weight=std::pow(posDist(robotSeqence[PARAM::BLUE][roboId][camId].pos,GlobalData::instance()->cameraMatrix[camera.id].pos)/100.0,-2.0);
                blueWeight+=_weight;
                blueAverage.setX(blueAverage.x() + robotSeqence[PARAM::BLUE][roboId][camId].pos.x() * _weight);
                blueAverage.setY(blueAverage.y() + robotSeqence[PARAM::BLUE][roboId][camId].pos.y() * _weight);
                blueAngle=robotSeqence[PARAM::BLUE][roboId][camId].angel;
                //std::cout<<"  weight:"<<_weight<<" "<<robotSeqence[PARAM::BLUE][roboId][camId].pos.x <<"\t";
            }
            _weight=0;
            if(robotSeqence[PARAM::YELLOW][roboId][camId].pos.x() >-30000 && robotSeqence[PARAM::YELLOW][roboId][camId].pos.y() >-30000 &&(!foundYellow ||
            (foundYellow && robotSeqence[PARAM::YELLOW][roboId][camId].pos.dist(CGeoPoint(yellowAverage.x()/yellowWeight,yellowAverage.y()/yellowWeight))<PARAM::ROBOTMERGEDOSTANCE)))
            {
//                std::cout<<"yellowFound:"<<foundYellow;
                foundYellow=true;
                _weight=std::pow(posDist(robotSeqence[PARAM::YELLOW][roboId][camId].pos,GlobalData::instance()->cameraMatrix[camera.id].pos)/100.0,-2.0);
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
    GlobalData::instance()->processRobot.push(result);
}

void CDealrobot::init(){
    result.init();
    for (int roboId=0; roboId<PARAM::ROBOTMAXID;roboId++)
        for (int camId=0;camId<PARAM::CAMERA;camId++){
           robotSeqence[PARAM::BLUE][roboId][camId].fill(-1,-32767,-32767,0);
           robotSeqence[PARAM::YELLOW][roboId][camId].fill(-1,-32767,-32767,0);
        }

    for (int i=0;i<PARAM::CAMERA;i++){
        for (int j=0;j<GlobalData::instance()->camera[i][0].robotSize[PARAM::BLUE];j++){
            Robot robot=GlobalData::instance()->camera[i][0].robot[PARAM::BLUE][j];
            robotSeqence[PARAM::BLUE][robot.id][i]=robot;
        }
        for (int j=0;j<GlobalData::instance()->camera[i][0].robotSize[PARAM::YELLOW];j++){
            Robot robot=GlobalData::instance()->camera[i][0].robot[PARAM::YELLOW][j];
            robotSeqence[PARAM::YELLOW][robot.id][i]=robot;
        }
    }
    for (int i=0;i<PARAM::ROBOTMAXID-1;i++){
        Robot temp(-32767,-32767,0,-1);
        sortTemp[PARAM::BLUE][i]=temp;
        sortTemp[PARAM::YELLOW][i]=temp;
    }
}

void CDealrobot::sortRobot(int color){
    //Possible Increase
    for (int i=0;i<result.robotSize[color];i++)
    {
        Robot robot=result.robot[color][i];
        GlobalData::instance()->robotPossible[color][robot.id]+=0.3;
        if (GlobalData::instance()->robotPossible[color][robot.id]>1.0)
            GlobalData::instance()->robotPossible[color][robot.id]=1.0;
        sortTemp[color][robot.id]=robot;
    }
    //Possible Decrease
    for(int j=0; j< GlobalData::instance()->processRobot[-1].robotSize[color]; j++)
    {
        bool found=false;
        Robot robot=GlobalData::instance()->processRobot[-1].robot[color][j];
        for (int i=0;i<result.robotSize[color];i++)
        if (result.robot[color][i].id==robot.id)
        {
            found=true;
            break;
        }
        if (!found) {
            GlobalData::instance()->robotPossible[color][robot.id]-=0.05;
            if (GlobalData::instance()->robotPossible[color][robot.id]<0.0)
            {
                //std::cout<<"Already try to delete Color"<<color<<" ID. "<<robot.id<<std::endl;
                GlobalData::instance()->robotPossible[color][robot.id]=0.0;
                sortTemp[color][robot.id].id=-1;
            }
            else
               sortTemp[color][robot.id]=robot;
        }
    }
    //sort
    for (int i=0;i<PARAM::ROBOTMAXID-1;i++)
        if (sortTemp[color][i].id>=0 && sortTemp[color][i].id<=PARAM::ROBOTMAXID){
        double possible=GlobalData::instance()->robotPossible[color][sortTemp[color][i].id];
        int maxj=i;
        for (int j=i+1;j<PARAM::ROBOTMAXID;j++){
            if (sortTemp[color][j].id>=0 && sortTemp[color][j].id<=PARAM::ROBOTMAXID &&
                sortTemp[color][maxj].id>=0 && sortTemp[color][maxj].id<PARAM::ROBOTMAXID &&
                    GlobalData::instance()->robotPossible[color][sortTemp[color][maxj].id]<
                    GlobalData::instance()->robotPossible[color][sortTemp[color][j].id]) maxj=j;
        }
        if (maxj!=i){
            Robot temp;
            temp=sortTemp[color][maxj];
            sortTemp[color][maxj]=sortTemp[color][i];
            sortTemp[color][i]=temp;
        }
    }
}

//void CDealrobot::filteRobot(Robot& robot){
//    auto & tempMatrix = _kalmanFilter.update(robot.pos.x(),robot.pos.y());
//    CGeoPoint filtPoint (tempMatrix(0,0),tempMatrix(1,0));
//    robot.pos=filtPoint;
//    robot.velocity.setVector(tempMatrix(2, 0)*FRAME_RATE, tempMatrix(3, 0)*FRAME_RATE);
//}

void CDealrobot::run(bool sw){
    if (sw){
        init();
        MergeRobot();
        sortRobot(PARAM::BLUE);
        sortRobot(PARAM::YELLOW);
        result.init();
        //重新加入概率排序后的车
        for (int i=0;i<PARAM::ROBOTMAXID;i++)
            if (sortTemp[PARAM::BLUE][i].id>=0 && sortTemp[PARAM::BLUE][i].id<=PARAM::ROBOTMAXID)
            if (GlobalData::instance()->robotPossible[sortTemp[PARAM::BLUE][i].id]>0)
            {
                result.addRobot(PARAM::BLUE,sortTemp[PARAM::BLUE][i]);
            }
        for (int i=0;i<PARAM::ROBOTMAXID;i++)
            if (sortTemp[PARAM::YELLOW][i].id>=0 && sortTemp[PARAM::YELLOW][i].id<=PARAM::ROBOTMAXID)
            if (GlobalData::instance()->robotPossible[sortTemp[PARAM::YELLOW][i].id]>0)
            {
                result.addRobot(PARAM::YELLOW,sortTemp[PARAM::YELLOW][i]);
            }
        //滤波和线性预测
        for (int i=0;i<result.robotSize[PARAM::BLUE];i++)
        {
            Robot robot=result.robot[PARAM::BLUE][i];
            auto & tempMatrix = _kalmanFilter[PARAM::BLUE][i].update(robot.pos.x(),robot.pos.y());
            CGeoPoint filtPoint (tempMatrix(0,0),tempMatrix(1,0));
            robot.pos=filtPoint;
            robot.velocity.setVector(tempMatrix(2, 0)*FRAME_RATE, tempMatrix(3, 0)*FRAME_RATE);
            CGeoPoint predictPos=filtPoint+robot.velocity;
        }
        for (int i=0;i<result.robotSize[PARAM::YELLOW];i++)
        {
            Robot robot=result.robot[PARAM::YELLOW][i];
            auto & tempMatrix = _kalmanFilter[PARAM::YELLOW][i].update(robot.pos.x(),robot.pos.y());
            CGeoPoint filtPoint (tempMatrix(0,0),tempMatrix(1,0));
            robot.pos=filtPoint;
            robot.velocity.setVector(tempMatrix(2, 0)*FRAME_RATE, tempMatrix(3, 0)*FRAME_RATE);
            CGeoPoint predictPos=filtPoint+robot.velocity;
        }
           // filteRobot(result.robot[PARAM::YELLOW][i]);
        GlobalData::instance()->processRobot.push(result);
    }
    else{
//        for(int i=0;i<PARAM::CAMERA;i++){
//            GlobalData::instance()->processRobot[i].push(GlobalData::instance()->camera[i][0]);
//            ReceiveVisionMessage test=GlobalData::instance()->processRobot[i][0];
//        }
    }
}
