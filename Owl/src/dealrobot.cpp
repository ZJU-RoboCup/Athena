#include "dealrobot.h"
#include "globaldata.h"
#include "staticparams.h"
#include <iostream>

CDealrobot::CDealrobot()
{
}

ReceiveVisionMessage CDealrobot::mergeRobot(ReceiveVisionMessage camera1, ReceiveVisionMessage camera2){
    ReceiveVisionMessage result;//
    return result;
}
void CDealrobot::run(bool sw){
    if (sw){


    }
    else{
        for(int i=0;i<PARAM::CAMERA;i++){
            GlobalData::instance()->processRobot[i].push(GlobalData::instance()->camera[i][0]);
            ReceiveVisionMessage test=GlobalData::instance()->processRobot[i][0];
        }
    }
}
