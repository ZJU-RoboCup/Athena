#include "dealrobot.h"
#include "globaldata.h"
#include "staticparams.h"
#include <iostream>

CDealrobot::CDealrobot()
{
}

double CDealrobot::posDist(Pos2d pos1, Pos2d pos2)
{
    return std::sqrt((pos1.x-pos2.x)*(pos1.x-pos2.x)+(pos1.y-pos2.y)*((pos1.y-pos2.y)));
}

void CDealrobot::MergeRobot(){
    for (int roboId=0; roboId<PARAM::ROBOTMAXID;roboId++){
        bool foundBlue=false,foundYellow=false;
        double blueWeight=0,yellowWeight=0;
        Pos2d blueAverage(0,0),yellowAverage(0,0);
        double blueAngle=0,yellowAngle=0;
        for (int camId=0;camId<PARAM::CAMERA;camId++){
            SingleCamera camera=GlobalData::instance()->cameraMatrix[camId];
            double _weight;
            if(robotSeqence[PARAM::BLUE][roboId][camId].pos.x>-30000 && robotSeqence[PARAM::BLUE][roboId][camId].pos.y>-30000)
            {
                foundBlue=true;
                _weight=std::pow(posDist(robotSeqence[PARAM::BLUE][roboId][camId].pos,GlobalData::instance()->cameraMatrix[camera.id].pos)/100.0,-2.0);
                blueWeight+=_weight;
                blueAverage.x+=robotSeqence[PARAM::BLUE][roboId][camId].pos.x * _weight;
                blueAverage.y+=robotSeqence[PARAM::BLUE][roboId][camId].pos.y * _weight;
                blueAngle+=robotSeqence[PARAM::BLUE][roboId][camId].angel * _weight;
            }
            if(robotSeqence[PARAM::YELLOW][roboId][camId].pos.x>-30000 && robotSeqence[PARAM::YELLOW][roboId][camId].pos.y>-30000)
            {
                foundYellow=true;
                _weight=std::pow(posDist(robotSeqence[PARAM::YELLOW][roboId][camId].pos,GlobalData::instance()->cameraMatrix[camera.id].pos)/100.0,-2.0);
                yellowWeight+=_weight;
                yellowAverage.x+=robotSeqence[PARAM::YELLOW][roboId][camId].pos.x * _weight;
                yellowAverage.y+=robotSeqence[PARAM::YELLOW][roboId][camId].pos.y * _weight;
                yellowAngle=robotSeqence[PARAM::YELLOW][roboId][camId].angel *_weight;
           }
        }
        if (foundBlue){
            Robot ave(blueAverage.x/blueWeight,blueAverage.y/blueWeight,blueAngle/blueWeight,roboId);
            result.addRobot(PARAM::BLUE,ave);
        }
        if (foundYellow){
            Robot ave(yellowAverage.x/yellowWeight,yellowAverage.y/yellowWeight,yellowAngle/yellowWeight,roboId);
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
}

void CDealrobot::run(bool sw){
    if (sw){
        init();
        MergeRobot();
    }
    else{
//        for(int i=0;i<PARAM::CAMERA;i++){
//            GlobalData::instance()->processRobot[i].push(GlobalData::instance()->camera[i][0]);
//            ReceiveVisionMessage test=GlobalData::instance()->processRobot[i][0];
//        }
    }
}
