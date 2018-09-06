#include "dealball.h"
#include "globaldata.h"
#include "staticparams.h"
#include "parammanager.h"
#include "matrix2d.h"
#include <iostream>
#define MAX_BALL_PER_FRAME 200
#define MIN_FILT_DIST 150
#define FRAME_RATE 61
#define MAX_BALL_LOST_FRAME 15
namespace{
    auto zpm =ZSS::ZParamManager::instance();
    long long min(long long a,long long b ){
        if (a<b) return a; else return b;
    }
}
CDealball::CDealball()
{
    zpm->loadParam(minBelieveFrame,"AlertFusion/ballMinBelieveFrame",10);
//    zpm->loadParam(maxLostFrame,"AlertFusion/carMaxLostFrame",15);
    //minBelieveFrame=3;
    //maxLostFrame=15;
    upPossible=1.0/minBelieveFrame;
    downPossible =0.05;

    lastBall.pos.setX(0);
    lastBall.pos.setY(0);
}

double CDealball::posDist(CGeoPoint pos1, CGeoPoint pos2)
{
    return std::sqrt((pos1.x() - pos2.x())*(pos1.x() - pos2.x())+(pos1.y() - pos2.y())*((pos1.y() - pos2.y())));
}

bool CDealball::ballNearVechile(Ball curentBall,double dist){
    bool answer=false;
    ReceiveVisionMessage result=GlobalData::instance()->maintain[-1];
    for (int i=0;i< result.robotSize[PARAM::BLUE];i++)
    {
       if (result.robot[PARAM::BLUE][i].pos.dist(curentBall.pos) <= dist ) answer = true;
    }
    for (int i=0;i< result.robotSize[PARAM::YELLOW];i++)
       if (result.robot[PARAM::YELLOW][i].pos.dist(curentBall.pos)<dist) answer = true;
    return answer;
}

double CDealball::calculateWeight(int camID,CGeoPoint ballPos){
    SingleCamera camera=GlobalData::instance()->cameraMatrix[camID];
    if (ballPos.x()>camera.leftedge.max && ballPos.x()<camera.rightedge.max &&
            ballPos.y()>camera.downedge.max && ballPos.y()<camera.upedge.max)
        return 1;
    else if (ballPos.x()<camera.leftedge.max && ballPos.x()>camera.leftedge.min)
        return abs(ballPos.x()-camera.leftedge.min)/abs(camera.leftedge.max-camera.leftedge.min);
    else if (ballPos.x()>camera.rightedge.max && ballPos.x()<camera.rightedge.min)
        return abs(ballPos.x()-camera.rightedge.min)/abs(camera.rightedge.max-camera.rightedge.min);
    else if (ballPos.y()<camera.downedge.max && ballPos.y()>camera.downedge.min)
        return abs(ballPos.y()-camera.downedge.min)/abs(camera.downedge.max-camera.downedge.min);
    else if (ballPos.y()>camera.upedge.max && ballPos.y()<camera.upedge.min)
        return abs(ballPos.y()-camera.upedge.min)/abs(camera.upedge.max-camera.upedge.min);
    else return 0.01;//to deal with can see out of border situation
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
    if (!validBall && GlobalData::instance()->ballStateMachine==struggle) result.addBall(GlobalData::instance()->maintain[-1].ball[0]);
    else{
        for (i=0;i<actualBallNum;i++){
            double weight=0;
            CGeoPoint average(0,0);//!!!
            for(j=0;j<PARAM::CAMERA;j++){
                if (ballSequence[i][j].pos.x()>-30000 && ballSequence[i][j].pos.y()>-30000)
                {
                    SingleCamera camera=GlobalData::instance()->cameraMatrix[j];
                    double _weight;
                    _weight= calculateWeight(j,ballSequence[i][j].pos);
                    _weight=std::pow(posDist(ballSequence[i][j].pos,GlobalData::instance()->cameraMatrix[camera.id].campos)/1000.0,-2.0);
                    if (PARAM::DEBUG)std::cout<<"camera: "<< j <<ballSequence[i][j].pos<<GlobalData::instance()->cameraMatrix[camera.id].campos<<"weight:"<<posDist(ballSequence[i][j].pos,GlobalData::instance()->cameraMatrix[camera.id].campos)<<std::endl;
                    weight+=_weight;
                    average.setX(average.x() + ballSequence[i][j].pos.x() * _weight);
                    average.setY(average.y() + ballSequence[i][j].pos.y() * _weight);
                }
            }
            if (weight!=0)result.addBall(average.x()/weight,average.y()/weight);
            if (PARAM::DEBUG) std::cout<<"have merged NO. "<<i<<" ball with"<<average<<" "<<weight<<"\n";
        }
    }
}

void CDealball::init(){
    _cycle=GlobalData::instance()->processBall.cycle()+1;
    for (int i=0;i<PARAM::CAMERA;i++){
        //if (GlobalData::instance()->cameraControl[i]==true){
            SingleCamera& currentCamera = GlobalData::instance()->cameraMatrix[i];
            for(int j=0;j<GlobalData::instance()->camera[i][0].ballSize;j++)
            {
                Ball currentball=GlobalData::instance()->camera[i][0].ball[j];
                result.addBall(GlobalData::instance()->camera[i][0].ball[j].pos.x(),
                        GlobalData::instance()->camera[i][0].ball[j].pos.y(),0,i);
            }
       // }
    }

    if (result.ballSize==0) {
        validBall=false;
        //std::cout<<"its invalid!!!!"<<std::endl;
    }
    else validBall=true;

    if (PARAM::DEBUG) std::cout<<"Origin vision has "<<result.ballSize<<" balls.\n";
    //GlobalData::instance()->processBall.push(result);
}


void CDealball::choseBall(){
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

        //possiblity
        if (lastBall.pos.dist(result.ball[id].pos)<min(2000,MAX_BALL_PER_FRAME*(_cycle-lastFrame))){
            lastBall=result.ball[id];
            lastFrame=_cycle;
            lastPossible = 1;
            currentPossible = 0;
        }
        else if(currentBall.pos.dist(result.ball[id].pos)<min(2000,MAX_BALL_PER_FRAME*(_cycle-currentFrame))){
            //std::cout<<"fuck!! same other ball!!!"<<std::endl;
            currentBall=result.ball[id];
            currentFrame=_cycle;
            currentPossible+=upPossible;
            lastPossible -=downPossible;
        }
        else{
            //std::cout<<"hello!! NEW other ball!!!"<<std::endl;
            currentBall=result.ball[id];
            currentFrame=_cycle;
            currentPossible =upPossible;
            lastPossible -=downPossible;
        }
        result.init();
        if (lastPossible>=currentPossible)
            result.addBall(lastBall);
        else{
            result.addBall(currentBall);
            lastBall=currentBall;
            lastFrame=currentFrame;
            lastPossible=currentPossible;
            currentPossible=0;
        }
}

void CDealball::run(){
    result.init();
    // TODO fill_n
    for(int i=0;i<PARAM::BALLNUM;i++)
        for(int j=0;j<PARAM::CAMERA;j++)
            ballSequence[i][j].fill(-32767,-32767);

    init();
    if (validBall){
        mergeBall();
        choseBall();
    }
    else //if(ballNearVechile(GlobalData::instance()->maintain[-1].ball[0],300))
    {
        //std::cout<<"cansee BUT yes!!!"<<std::endl;
        result.addBall(GlobalData::instance()->maintain[0].ball[0]);
        lastBall=GlobalData::instance()->maintain[0].ball[0];
        lastPossible=1;
    }

    GlobalData::instance()->processBall.push(result);
}
