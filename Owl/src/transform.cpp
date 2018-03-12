#include "transform.h"
#include "staticparams.h"
#include "globaldata.h"
#include "visionmodule.h"


CTransform::CTransform(){

}
void CTransform::run(bool sw){
    if(sw){

    }else{
        for(int i=0;i<PARAM::CAMERA;i++){
            GlobalData::instance()->transformed[i].push(GlobalData::instance()->camera[i][0]);
        }
    }
}
