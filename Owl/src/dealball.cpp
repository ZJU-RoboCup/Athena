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
        else ballSequence[j][actualBallNum++].fill(result.ball[i]);
    }
    if (PARAM::DEBUG) std::cout<<"Actually have "<<actualBallNum<<" balls.\n";


}

void CDealball::run(bool sw){
    result.init();
    if (sw){
        //init
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

        mergeBall();

    }
    else{
        for(int i=0;i<PARAM::CAMERA;i++){
            GlobalData::instance()->processBall.push(GlobalData::instance()->camera[i][0]);
        }
    }
}
