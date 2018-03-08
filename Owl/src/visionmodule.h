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
    //void tranMatrix(const OriginMessage& ,int);

signals:
    void needDraw();
public slots:
    void storeData();
private:
    QUdpSocket udpSocket;
    QHostAddress groupAddress;
    int interface;
    int port;
    bool collectNewVision();
    bool dealWithData();
    bool immortalsVision();
};
typedef Singleton<CVisionModule> VisionModule;
#endif // __VISIONMODULE_H__
