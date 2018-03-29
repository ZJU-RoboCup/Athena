#include "dealrobot.h"
#include "globaldata.h"
#include "staticparams.h"
#include <iostream>

CDealrobot::CDealrobot()
{
    for(int i=0;i<PARAM::ROBOTMAXID;i++){
        GlobalData::instance()->robotPossible[PARAM::BLUE][i]=0;
        GlobalData::instance()->robotPossible[PARAM::YELLOW][i]=0;
    }
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
       // std::cout<<"roboID:"<<roboId<<" : ";
        for (int camId=0;camId<PARAM::CAMERA;camId++){
            SingleCamera camera=GlobalData::instance()->cameraMatrix[camId];
            double _weight=0;
            if(robotSeqence[PARAM::BLUE][roboId][camId].pos.x>-30000 && robotSeqence[PARAM::BLUE][roboId][camId].pos.y>-30000)
            {
                foundBlue=true;
                _weight=std::pow(posDist(robotSeqence[PARAM::BLUE][roboId][camId].pos,GlobalData::instance()->cameraMatrix[camera.id].pos)/100.0,-2.0);
                blueWeight+=_weight;
                blueAverage.x+=robotSeqence[PARAM::BLUE][roboId][camId].pos.x * _weight;
                blueAverage.y+=robotSeqence[PARAM::BLUE][roboId][camId].pos.y * _weight;
                blueAngle+=robotSeqence[PARAM::BLUE][roboId][camId].angel * _weight;
                //std::cout<<_weight<<" "<<robotSeqence[PARAM::BLUE][roboId][camId].angel <<"\t";
                //blueAngle=robotSeqence[PARAM::BLUE][roboId][camId].angel ;
            }
            _weight=0;
            if(robotSeqence[PARAM::YELLOW][roboId][camId].pos.x>-30000 && robotSeqence[PARAM::YELLOW][roboId][camId].pos.y>-30000)
            {
                foundYellow=true;
                _weight=std::pow(posDist(robotSeqence[PARAM::YELLOW][roboId][camId].pos,GlobalData::instance()->cameraMatrix[camera.id].pos)/100.0,-2.0);
                yellowWeight+=_weight;
                yellowAverage.x+=robotSeqence[PARAM::YELLOW][roboId][camId].pos.x * _weight;
                yellowAverage.y+=robotSeqence[PARAM::YELLOW][roboId][camId].pos.y * _weight;
                yellowAngle=robotSeqence[PARAM::YELLOW][roboId][camId].angel *_weight;
                //std::cout<<_weight<<" "<<robotSeqence[PARAM::YELLOW][roboId][camId].angel <<"\t";
                //yellowAngle=robotSeqence[PARAM::YELLOW][roboId][camId].angel;
           }
        }
        //std::cout<<"\n";
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
                std::cout<<"Already try to delete Color"<<color<<" ID. "<<robot.id<<std::endl;
                GlobalData::instance()->robotPossible[color][robot.id]=0.0;
                sortTemp[color][robot.id].id=-1;
            }
            else
               sortTemp[color][robot.id]=robot;
        }
    }
    //sort
    for (int i=0;i<PARAM::ROBOTMAXID-1;i++)
        if (sortTemp[color][i].id>=0){
        double possible=GlobalData::instance()->robotPossible[color][sortTemp[color][i].id];
        int maxj=i;
        for (int j=i+1;j<PARAM::ROBOTMAXID;j++)
            if (sortTemp[color][j].id>=0)
            if (possible<GlobalData::instance()->robotPossible[color][sortTemp[color][j].id]) maxj=j;
        if (maxj!=i){
            Robot temp;
            temp=sortTemp[color][maxj];
            sortTemp[color][maxj]=sortTemp[color][i];
            sortTemp[color][i]=temp;
        }
    }
}

void CDealrobot::run(bool sw){
    if (sw){
        init();
        MergeRobot();
        sortRobot(PARAM::BLUE);
        sortRobot(PARAM::YELLOW);
        result.init();
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
        GlobalData::instance()->processRobot.push(result);
    }
    else{
//        for(int i=0;i<PARAM::CAMERA;i++){
//            GlobalData::instance()->processRobot[i].push(GlobalData::instance()->camera[i][0]);
//            ReceiveVisionMessage test=GlobalData::instance()->processRobot[i][0];
//        }
    }
}
