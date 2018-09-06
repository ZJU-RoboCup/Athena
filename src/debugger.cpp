#include "debugger.h"
#include "globaldata.h"
Debugger::Debugger(QObject *parent) : QObject(parent)
{
    QObject::connect(&receiver,SIGNAL(readyRead()),this,SLOT(receive()),Qt::DirectConnection);
    receiver.bind(QHostAddress::AnyIPv4,ZSS::Athena::DEBUG_MSG_RECEIVE, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
}
void Debugger::receive(){
    auto& datagram = GlobalData::instance()->debugMessages;
    while (receiver.state() == QUdpSocket::BoundState && receiver.hasPendingDatagrams()) {
        GlobalData::instance()->debugMutex.lock();
        datagram.resize(receiver.pendingDatagramSize());
        receiver.readDatagram(datagram.data(),datagram.size());
        GlobalData::instance()->debugMutex.unlock();
    }
}
