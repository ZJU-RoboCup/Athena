#include "communicator.h"
#include "staticparams.h"
#include <QNetworkInterface>
#include "zss_cmd.pb.h"
#include "actionmodule.h"
//#include <QElapsedTimer>

Communicator::Communicator(QObject *parent) : QObject(parent){
    QObject::connect(&receiveSocket,SIGNAL(readyRead()),this,SLOT(testReceive()),Qt::DirectConnection);
}
bool Communicator::connect(){
    if (
            receiveSocket.bind(QHostAddress::AnyIPv4,ZSS::Athena::CONTROL_BACK_RECEIVE, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint) &&
            receiveSocket.joinMulticastGroup(QHostAddress(ZSS::ZSS_ADDRESS),QNetworkInterface::interfaceFromName(networkInterfaceNames[networkInterfaceIndex]))
            ){
        sendSocket.setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
        return true;
    }
    disconnect();
    return false;
}
bool Communicator::disconnect(){
    receiveSocket.disconnectFromHost();
    sendSocket.disconnectFromHost();
    return true;
}
QStringList& Communicator::updateNetworkInterfaces(){
    const auto& interfaces = QNetworkInterface::allInterfaces();
    networkInterfaceNames.clear();
    networkInterfaceReadableNames.clear();
    for(int i = 0; i < interfaces.length(); ++i){
        networkInterfaceNames.append(interfaces[i].name());
        networkInterfaceReadableNames.append(interfaces[i].humanReadableName());
    }
    return networkInterfaceReadableNames;
}
void Communicator::changeNetworkInterface(int index){
    networkInterfaceIndex = index;
}
// test TODO;
bool Communicator::testSend(const QString& message){
    qDebug() << "try to send : " << message;
    QByteArray data = message.toLatin1();
    sendSocket.writeDatagram(data,QHostAddress(ZSS::ZSS_ADDRESS),ZSS::Athena::CONTROL_SEND);
    return true;
}
void Communicator::testReceive(){
    QByteArray datagram;
    while (receiveSocket.state() == QUdpSocket::BoundState && receiveSocket.hasPendingDatagrams()) {
        datagram.resize(receiveSocket.pendingDatagramSize());
        receiveSocket.readDatagram(datagram.data(), datagram.size());
        qDebug() << "receive data : " << datagram;
        sendCommand(datagram);
    }
}

void Communicator::sendCommand(QByteArray datagram){
    ZSS::Protocol::Robots_Command commands;
    commands.ParseFromArray(datagram, datagram.size());
    qDebug() << "Send Command Now !!!";
    ZSS::ZActionModule::instance()->sendLegacy(commands);
}
