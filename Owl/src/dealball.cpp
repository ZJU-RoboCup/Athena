#include "dealball.h"
#include "globaldata.h"
#include "staticparams.h"

CDealball::CDealball()
{
}
void CDealball::run(bool sw){
    if (sw){
        for(int i=0;i<PARAM::CAMERA;i++){
            GlobalData::instance()->processBall[i].push(GlobalData::instance()->camera[i][0]);
        }

    }
    else{

    }
}
