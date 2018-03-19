#include "dealball.h"
#include "globaldata.h"
#include "staticparams.h"
#include <iostream>

CDealball::CDealball()
{
}
double CDealball::posDist(Pos2d pos1, Pos2d pos2)
{
    return std::sqrt((pos1.x-pos2.x)*(pos1.x-pos2.x)+(pos1.y-pos2.y)*((pos1.y-pos2.y)));
}

void CDealball::mergeBall(){
    int i,j;
    actualBallNum=0;
    for ( i=0; i<result.ballSize;i++){
        bool found=false;
        for (j=0;j<actualBallNum;j++)
        {
            for (int id=0;id<PARAM::CAMERA;id++)
                if (posDist(result.ball[i].pos,ballSequence[j][id].pos)<PARAM::MERGEDISTANCE){
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
        Pos2d count(0,0);
        double weight=0;
        Pos2d average(0,0);
        for(j=0;j<PARAM::CAMERA;j++){
            if (ballSequence[i][j].pos.x>-9000 && ballSequence[i][j].pos.y>-9000)
            {
                SingleCamera camera=GlobalData::instance()->cameraMatrix[j];
                double _weight;
                _weight=std::pow(posDist(ballSequence[i][j].pos,GlobalData::instance()->cameraMatrix[camera.id].pos)/100.0,-2.0);
                weight+=_weight;
                average.x+=ballSequence[i][j].pos.x * _weight;
                average.y+=ballSequence[i][j].pos.y * _weight;
            }
        }
        result.addBall(average.x/weight,average.y/weight);
        if (PARAM::DEBUG) std::cout<<"have merged NO. "<<i<<" ball with"<<average.x<<","<<average.y<<" "<<weight<<"\n";
    }
    GlobalData::instance()->processBall.push(result);
}

void CDealball::init(){
    for (int i=0;i<PARAM::CAMERA;i++){
        for(int j=0;j<GlobalData::instance()->camera[i][0].ballSize;j++)
        {
            std::cout<<i<<" "<<j<<" "<<GlobalData::instance()->camera[i][0].ballSize<<"\t";
            result.addBall(GlobalData::instance()->camera[i][0].ball[j].pos.x,
                    GlobalData::instance()->camera[i][0].ball[j].pos.y,0,i);
        }
    }
    if (PARAM::DEBUG) std::cout<<"Origin vision has "<<result.ballSize<<" balls.\n";
    GlobalData::instance()->processBall.push(result);
}

void CDealball::run(bool sw){
    result.init();
    for(int i=0;i<PARAM::BALLNUM;i++)
        for(int j=0;j<PARAM::CAMERA;j++)
            ballSequence[i][j].fill(-9999,-9999);

    if (sw){
        init();
        mergeBall();

    }
    else{
        for(int i=0;i<PARAM::CAMERA;i++){
            GlobalData::instance()->processBall.push(GlobalData::instance()->camera[i][0]);
        }
    }
}
