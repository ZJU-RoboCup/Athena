#ifndef ZACTIONMODULE_H
#define ZACTIONMODULE_H
#include <QObject>
#include <QSerialPort>
#include "singleton.hpp"
#include "zss_cmd.pb.h"
namespace ZSS{
class ActionModuleSerialVersion : public QObject
{
    Q_OBJECT
public:
    ActionModuleSerialVersion(QObject *parent = 0);
    ~ActionModuleSerialVersion();
    bool init();
    void sendLegacy(const ZSS::Protocol::Robots_Command&);
    QStringList& updatePortsList();
    int getFrequency(){ return frequency;}
    bool changePorts(int);
    bool changeFrequency(int);
    bool openSerialPort();
    bool closeSerialPort();
private slots:
    void readData();
private:
    void sendStartPacket();
private:
    QSerialPort serial;
    QStringList ports;
    int frequency;
    QByteArray tx;
    QByteArray rx;
};
typedef Singleton<ActionModuleSerialVersion> ZActionModule;
}
#endif // ZACTIONMODULE_H
