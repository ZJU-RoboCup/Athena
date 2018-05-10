#ifndef __INTERACTION_H__
#define __INTERACTION_H__

#include <QObject>
#include "visionmodule.h"
#include "staticparams.h"
#include "field.h"
#include "dealball.h"
#include "communicator.h"
class Interaction : public QObject
{
    Q_OBJECT
public:
    explicit Interaction(QObject *parent = 0);

    Q_INVOKABLE bool connectSerialPort(bool);
    Q_INVOKABLE bool changeSerialFrequency(int);
    Q_INVOKABLE bool changeSerialPort(int);
    Q_INVOKABLE QStringList getSerialPortsList();
    Q_INVOKABLE int getFrequency();
    
    Q_INVOKABLE void setVision(bool);
    Q_INVOKABLE int  getCameraNumber() { return PARAM::CAMERA; }
    Q_INVOKABLE void controlCamera(int,bool);
    Q_INVOKABLE void controlProcess(int,bool);
    Q_INVOKABLE int getFPS(){ return VisionModule::instance()->getFPS();}
    Q_INVOKABLE double getBallVelocity(){return Dealball::instance()->getBallSpeed();}
    Q_INVOKABLE void setArea(int a,int b,int c,int d){Field::setArea(a,b,c,d); }
    Q_INVOKABLE bool send(const QString& str){
        return ZCommunicator::instance()->testSend(str);
    }
    Q_INVOKABLE QStringList getNetworkInterfaces(){
        return ZCommunicator::instance()->updateNetworkInterfaces();
    }
    Q_INVOKABLE bool changeConnection(bool sw){
        if(sw)
            return ZCommunicator::instance()->connect();
        else
            return ZCommunicator::instance()->disconnect();
    }
    Q_INVOKABLE void changeNetworkInterface(int index){
        ZCommunicator::instance()->changeNetworkInterface(index);
    }
    ~Interaction();
public:
signals:
public slots:
};

#endif // __INTERACTION_H__
