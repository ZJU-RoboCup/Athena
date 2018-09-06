#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include <QObject>
#include <QUdpSocket>
#include <singleton.hpp>
class Communicator : public QObject
{
    Q_OBJECT
public:
    explicit Communicator(QObject *parent = nullptr);
    QStringList& updateNetworkInterfaces();
    void changeNetworkInterface(int);
    bool connect();
    bool disconnect();
    // test TODO;
    bool testSend(const QString&);
    void sendCommand(QByteArray datagram);
private slots:
    void testReceive();
private:
    QUdpSocket sendSocket,receiveSocket;
    QStringList networkInterfaceNames;
    QStringList networkInterfaceReadableNames;
    int networkInterfaceIndex;
};
typedef Singleton<Communicator> ZCommunicator;
#endif // COMMUNICATOR_H
