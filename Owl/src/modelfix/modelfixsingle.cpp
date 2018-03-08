#include "modelfixsingle.h"
#include "globaldata.h"
ModelFixSingle::ModelFixSingle(int id):camera(id){
}
void ModelFixSingle::run(){
    auto& input = GlobalData::instance()->transformed[camera];
    auto& output = GlobalData::instance()->modelFixed[camera];
    ReceiveVisionMessage msg;
    for(int color=BLUE;color<=YELLOW;color++){
        for(int i=0;i<PARAM::ROBOTMAXID;i++){
            auto& f = robotFollow[color][i];
            if(input[0].robotIndex[color][i] != -1){
                f.confidence = 1;
                f.lostFrame = 0;
            }else if(f.lostFrame < GlobalData::instance()->maxLostFrame){
                f.lostFrame++;
            }else{
                continue;
            }
            msg.addRobot(color,input[0-f.lostFrame].robot[color][input[0-f.lostFrame].robotIndex[color][i]]);
        }
    }
    output.push(msg);
}
