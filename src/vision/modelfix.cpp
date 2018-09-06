#include "modelfix.h"
#include "globaldata.h"
#include "staticparams.h"
#include "modelfix/modelfixsingle.h"
CModelFix::CModelFix(){
    for(int i=0;i<PARAM::CAMERA;i++){
        mf[i] = new ModelFixSingle(i);
    }
}
CModelFix::~CModelFix(){
    for(int i=0;i<PARAM::CAMERA;i++){
        if(mf[i]!=nullptr)
            delete mf[i];
    }
}
void CModelFix::run(bool sw){
    if(sw){
        for(int i=0;i<PARAM::CAMERA;i++){
            mf[i]->run();
        }
    }else{
        for(int i=0;i<PARAM::CAMERA;i++){
            GlobalData::instance()->modelFixed[i].push(GlobalData::instance()->transformed[i][0]);
        }
    }
}
