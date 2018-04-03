#ifndef __VISIONMODULE_H__
#define __VISIONMODULE_H__

#include <QObject>
#include <QUdpSocket>
#include "singleton.hpp"
#include "staticparams.h"
#include "messageformat.h"
class CVisionModule : public QObject {
    Q_OBJECT
public:
    CVisionModule(QObject *parent = 0);
    void udpSocketConnect();
    void udpSocketDisconnect();
    void parse(void *,int);
    Q_INVOKABLE quint16 getFPS();
    bool inChoseArea(float,float);
    //void tranMatrix(const OriginMessage& ,int);

signals:
    void needDraw();
public slots:
    void storeData();
private:
    QUdpSocket udpSocket;
    QHostAddress groupAddress;
    quint64 counter;
    int interface;
    int port;
    int MINX=0,MAXX=6500,MINY=-4800,MAXY=4800;
    bool collectNewVision();
    bool dealWithData();
    bool immortalsVision();
};
typedef Singleton<CVisionModule> VisionModule;
#endif // __VISIONMODULE_H__
