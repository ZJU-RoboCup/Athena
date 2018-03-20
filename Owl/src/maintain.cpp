#include "maintain.h"
#include "globaldata.h"
#include "staticparams.h"


CMaintain::CMaintain()
{

}

void CMaintain::init(){
    result.init();
    for (int i=0;i<GlobalData::instance()->processBall[0].ballSize;i++)
        result.addBall(GlobalData::instance()->processBall[0].ball[i]);
    for (int i=0;i<GlobalData::instance()->processRobot[0].robotSize[PARAM::BLUE];i++)
        result.addRobot(PARAM::BLUE,GlobalData::instance()->processRobot[0].robot[PARAM::BLUE][i]);
    for (int i=0;i<GlobalData::instance()->processRobot[0].robotSize[PARAM::YELLOW];i++)
        result.addRobot(PARAM::BLUE,GlobalData::instance()->processRobot[0].robot[PARAM::YELLOW][i]);
    GlobalData::instance()->maintain.push(result);
}

void CMaintain::run(bool sw){
    if (sw){
        init();
    }
    else{

    }
}
