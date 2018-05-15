#include "interaction4field.h"
#include "globalsettings.h"
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
