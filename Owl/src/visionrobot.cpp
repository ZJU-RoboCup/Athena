#include "visionrobot.h"

CVisionrobot::CVisionrobot(){

}
void CVisionrobot::run(bool sw){
    if (sw){

    }
    else{
        for(int i=0;i<PARAM::CAMERA;i++){
            GlobalData::instance()->transformed[i].push(GlobalData::instance()->camera[i][0]);
        }
    }
}
