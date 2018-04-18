#include "dealball.h"
#include "globaldata.h"
#include "staticparams.h"
#include "matrix2d.h"
#include <iostream>
#define MAX_BALL_PER_FRAME 150
#define MIN_FILT_DIST 15
#define FRAME_RATE 61

CDealball::CDealball()
{
    lastBall.pos.setX(0);
    lastBall.pos.setY(0);
}
double CDealball::posDist(CGeoPoint pos1, CGeoPoint pos2)
{
    return std::sqrt((pos1.x() - pos2.x())*(pos1.x() - pos2.x())+(pos1.y() - pos2.y())*((pos1.y() - pos2.y())));
}

void CDealball::mergeBall(){
    int i,j;
    actualBallNum=0;
    for ( i=0; i<result.ballSize;i++){
        bool found=false;
        for (j=0;j<actualBallNum;j++)
        {
            for (int id=0;id<PARAM::CAMERA;id++)
                if (posDist(result.ball[i].pos,ballSequence[j][id].pos)<PARAM::BALLMERGEDISTANCE){
                    found=true;
                    break;
                }
            if (found) break;
        }
        if (found) ballSequence[j][result.ball[i].cameraID].fill(result.ball[i]);
        else ballSequence[actualBallNum++][result.ball[i].cameraID].fill(result.ball[i]);
    }
    if (PARAM::DEBUG) std::cout<<"Actually have "<<actualBallNum<<" balls.\n";
    result.init();
    for (i=0;i<actualBallNum;i++){
        double weight=0;
        CGeoPoint average(0,0);
        for(j=0;j<PARAM::CAMERA;j++){
            if (ballSequence[i][j].pos.x()>-30000 && ballSequence[i][j].pos.y()>-30000)
            {
                SingleCamera camera=GlobalData::instance()->cameraMatrix[j];
                double _weight;
                _weight=std::pow(posDist(ballSequence[i][j].pos,GlobalData::instance()->cameraMatrix[camera.id].pos)/100.0,-2.0);
                weight+=_weight;
                average.setX(average.x() + ballSequence[i][j].pos.x() * _weight);
                average.setY(average.y() + ballSequence[i][j].pos.y() * _weight);
            }
        }
        result.addBall(average.x()/weight,average.y()/weight);
        if (PARAM::DEBUG) std::cout<<"have merged NO. "<<i<<" ball with"<<average<<" "<<weight<<"\n";
    }

}

void CDealball::init(){
    for (int i=0;i<PARAM::CAMERA;i++){
        for(int j=0;j<GlobalData::instance()->camera[i][0].ballSize;j++)
        {
            result.addBall(GlobalData::instance()->camera[i][0].ball[j].pos.x(),
                    GlobalData::instance()->camera[i][0].ball[j].pos.y(),0,i);
            if (PARAM::DEBUG) std::cout<<" "<<i<<" "
                                      <<GlobalData::instance()->camera[i][0].ball[j].pos<<" ";
        }
    }
    if (PARAM::DEBUG) std::cout<<"Origin vision has "<<result.ballSize<<" balls.\n";
    //GlobalData::instance()->processBall.push(result);
}



void CDealball::filteBall(){
    float dis=32767;
    int id=-1;
    for (int i=0; i<result.ballSize;i++)
    {
        if (result.ball[i].pos.dist(lastBall.pos)<dis){
            dis = result.ball[i].pos.dist(lastBall.pos);
            id = i;
            if (PARAM::DEBUG) std::cout <<" the dis=" <<dis<<std::endl;
        }
    }
    lastBall=result.ball[id];
    Ball curentBall=result.ball[id];
    bool filteSwitch=true;
    for (int i=0;i< result.robotSize[PARAM::BLUE];i++)
       if (result.robot[PARAM::BLUE][i].pos.dist(curentBall.pos) <= MIN_FILT_DIST ) filteSwitch = false;

    for (int i=0;i< result.robotSize[PARAM::YELLOW];i++)
       if (result.robot[PARAM::YELLOW][i].pos.dist(curentBall.pos)<MIN_FILT_DIST) filteSwitch = false;
    auto & tempMatrix= filteSwitch? _kalmanFilter.update(curentBall.pos.x(),curentBall.pos.y()):_kalmanFilter.follow(curentBall.pos.x(),curentBall.pos.y());
    CGeoPoint filtePos(tempMatrix(0,0),tempMatrix(1,0));
    result.ballSize=0;
    result.ball[0].fill(filtePos.x(),filtePos.y());
    result.ball[0].velocity.setVector(tempMatrix(2, 0)*FRAME_RATE, tempMatrix(3, 0)*FRAME_RATE);
    std::cout<<"After filt Pos:"<<filtePos<<"\twith velocity = "<<result.ball[0].velocity.mod()<<std::endl;
    GlobalData::instance()->processBall.push(result);
}

void CDealball::run(bool sw){
    result.init();
    for(int i=0;i<PARAM::BALLNUM;i++)
        for(int j=0;j<PARAM::CAMERA;j++)
            ballSequence[i][j].fill(-32767,-32767);

    if (sw){
        init();
        mergeBall();
        filteBall();
        GlobalData::instance()->processBall.push(result);
    }
    else{
        for(int i=0;i<PARAM::CAMERA;i++){
            GlobalData::instance()->processBall.push(GlobalData::instance()->camera[i][0]);
        }
    }
}
