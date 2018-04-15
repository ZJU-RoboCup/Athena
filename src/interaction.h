#ifndef __INTERACTION_H__
#define __INTERACTION_H__

#include <QObject>
#include "visionmodule.h"
#include "staticparams.h"
#include "field.h"
class Interaction : public QObject
{
    Q_OBJECT
public:
    explicit Interaction(QObject *parent = 0);
    Q_INVOKABLE void setVision(bool);
    Q_INVOKABLE int  getCameraNumber() { return PARAM::CAMERA; }
    Q_INVOKABLE void controlCamera(int,bool);
    Q_INVOKABLE void controlProcess(int,bool);
    Q_INVOKABLE int getFPS(){ return VisionModule::instance()->getFPS();}
    Q_INVOKABLE void setArea(int a,int b,int c,int d){Field::setArea(a,b,c,d); }
    ~Interaction();
public:
signals:
public slots:
};

#endif // __INTERACTION_H__
