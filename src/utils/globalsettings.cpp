#include "globalsettings.h"
#include "field.h"
#include "parammanager.h"
CGlobalSettings::CGlobalSettings(QObject *parent)
    : QObject(parent),minimumX(-999999),minimumY(-999999),maximumX(999999),maximumY(999999){

}
bool CGlobalSettings::inChosenArea(float x, float y){
    return (x >= minimumX && x <= maximumX && y >= minimumY && y <= maximumY);
}
bool CGlobalSettings::inChosenArea(CGeoPoint point){
    return (point.x() >= minimumX && point.x() <= maximumX && point.y() >= minimumY && point.y() <= maximumY);
}
void CGlobalSettings::setArea(int a,int b,int c,int d){
    minimumX = Field::fieldXFromCoordinate(a);
    minimumY = Field::fieldYFromCoordinate(c);
    maximumX = Field::fieldXFromCoordinate(b);
    maximumY = Field::fieldYFromCoordinate(d);
    // field axis direction is different from the canvas so the max and the min should exchange
    emit needRepaint();
}
void CGlobalSettings::resetArea(){
    int width,height;
    ZSS::ZParamManager::instance()->loadParam(width,"field/canvasWidth",13200);
    ZSS::ZParamManager::instance()->loadParam(height,"field/canvasHeight",9900);
    minimumX = -width/2;
    minimumY = -height/2;
    maximumX = width/2;
    maximumY = height/2;
    emit needRepaint();
}
