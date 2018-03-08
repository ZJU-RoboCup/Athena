#include "interaction.h"
#include "visionmodule.h"
#include "globaldata.h"
Interaction::Interaction(QObject *parent) : QObject(parent) {
}
Interaction::~Interaction() {
}
void Interaction::setVision(bool needStart){
    if (needStart) {
        VisionModule::instance()->udpSocketConnect();
    }else{
        VisionModule::instance()->udpSocketDisconnect();
    }
}
void Interaction::controlCamera(int index, bool state){
    GlobalData::instance()->cameraControl[index] = state;
}
void Interaction::controlProcess(int index,bool state){
    GlobalData::instance()->processControl[index] = state;
}
