#include "interaction4field.h"
#include "globalsettings.h"
#include "globaldata.h"
Interaction4Field::Interaction4Field(QObject *parent) : QObject(parent) {
}
Interaction4Field::~Interaction4Field() {
}
void Interaction4Field::setArea(int a,int b,int c,int d){
    GlobalSettings::instance()->setArea(a,b,c,d);
}
void Interaction4Field::resetArea(){
    GlobalSettings::instance()->resetArea();
}
void Interaction4Field::moveField(int a,int b){
    Field::moveField(a,b);
    emit GlobalSettings::instance()->needRepaint();
}
void Interaction4Field::setSize(int width,int height){
    Field::setSize(width,height);
}
void Interaction4Field::setPlacementPoint(int x,int y){
    GlobalSettings::instance()->setBallPlacementPos(x,y);
}
void Interaction4Field::setCtrlC(){
    GlobalData::instance()->ctrlCMutex.lock();
    GlobalData::instance()->ctrlC = !GlobalData::instance()->ctrlC;
    GlobalData::instance()->ctrlCMutex.unlock();
}
int Interaction4Field::getRealX(int x){// cm
    return (int)Field::fieldXFromCoordinate(x)/10;
}
int Interaction4Field::getRealY(int y){// cm
    return (int)Field::fieldYFromCoordinate(y)/10;
}
