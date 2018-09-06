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
    void udpSocketConnect(bool);
    void udpSocketDisconnect();
    void parse(void *,int);
    quint16 getFPS();

    //void tranMatrix(const OriginMessage& ,int);

signals:
    void needDraw();
public slots:
    void storeData();
private:
    CGeoPoint saoConvert(CGeoPoint);
    double saoConvert(double);
    void edgeTest();
    QUdpSocket udpSocket;
    QHostAddress groupAddress;
    quint64 counter;
    int interface;
    int port;
    int saoAction;
    bool collectNewVision();
    bool dealWithData();
};
typedef Singleton<CVisionModule> VisionModule;
#endif // __VISIONMODULE_H__
