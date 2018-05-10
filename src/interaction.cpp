#include "interaction.h"
#include "visionmodule.h"
#include "globaldata.h"
#include "maintain.h"
#include "actionmodule.h"
Interaction::Interaction(QObject *parent) : QObject(parent) {
}
Interaction::~Interaction() {
}
void Interaction::setVision(bool needStart){
    if (needStart) {
        VisionModule::instance()->udpSocketConnect();
        Maintain::instance()->udpSocketConnect();
    }else{
        VisionModule::instance()->udpSocketDisconnect();
        Maintain::instance()->udpSocketDisconnect();
    }
}
void Interaction::controlCamera(int index, bool state){
    GlobalData::instance()->cameraControl[index] = state;
}
void Interaction::controlProcess(int index,bool state){
    GlobalData::instance()->processControl[index] = state;
}

bool Interaction::connectSerialPort(bool sw){
    if(sw){
        return ZSS::ZActionModule::instance()->init();
    }
    return ZSS::ZActionModule::instance()->closeSerialPort();
}

bool Interaction::changeSerialFrequency(int frequency){
    return ZSS::ZActionModule::instance()->changeFrequency(frequency);
}

bool Interaction::changeSerialPort(int index){
    return ZSS::ZActionModule::instance()->changePorts(index);
}

QStringList Interaction::getSerialPortsList(){
    return ZSS::ZActionModule::instance()->updatePortsList();
}
int Interaction::getFrequency(){
    return ZSS::ZActionModule::instance()->getFrequency();
}
