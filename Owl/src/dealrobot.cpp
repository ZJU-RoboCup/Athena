#include "dealrobot.h"
#include "globaldata.h"
#include "staticparams.h"

CDealrobot::CDealrobot()
{
}
void CDealrobot::run(bool sw){
    if (sw){
        for(int i=0;i<PARAM::CAMERA;i++){
            GlobalData::instance()->processRobot[i].push(GlobalData::instance()->camera[i][0]);
        }
    }
    else{

    }
}
