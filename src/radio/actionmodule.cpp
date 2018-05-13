#include "actionmodule.h"
#include "parammanager.h"
#include <QtSerialPort/QSerialPortInfo>
#include "crc.h"
#include <QtDebug>
namespace ZSS {
namespace{
    const int TRANSMIT_PACKET_SIZE = 25;
    const int TRANS_FEEDBACK_SIZE = 25;
    auto zpm = ZSS::ZParamManager::instance();
    void encodeLegacy(const ZSS::Protocol::Robot_Command&,QByteArray&,int);
    quint8 kickStandardization(quint8,bool,quint8);
}
ActionModuleSerialVersion::ActionModuleSerialVersion(QObject *parent) : QObject(parent) {
    updatePortsList();
    serial.setBaudRate(QSerialPort::Baud115200);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    zpm->loadParam(frequency,"Radio/Frequency",8);
    tx.resize(TRANSMIT_PACKET_SIZE);
    rx.resize(TRANS_FEEDBACK_SIZE);
    tx[0] = 0x40;
    connect(&serial, &QSerialPort::readyRead, this, &ActionModuleSerialVersion::readData);
}
ActionModuleSerialVersion::~ActionModuleSerialVersion(){
    closeSerialPort();
}
QStringList& ActionModuleSerialVersion::updatePortsList(){
    this->ports.clear();
    const auto& ports = QSerialPortInfo::availablePorts();
    for(const auto &port : ports){
        this->ports.append(port.portName());
    }
    if(this->ports.size() > 0)
        serial.setPortName(this->ports[this->ports.size() - 1]);
    return this->ports;
}
bool ActionModuleSerialVersion::init(){
    if (serial.isOpen()) {
        serial.close();
    }
    if (serial.open(QIODevice::ReadWrite)) {
        qDebug() << "SerialPort connected... : " << serial.portName();
        sendStartPacket();
        return true;
    }
    qDebug() << "SerialPort connect failed... : " << serial.portName();
    return false;
}
void ActionModuleSerialVersion::sendStartPacket(){
    QByteArray startPacket1(TRANSMIT_PACKET_SIZE,0);
    QByteArray startPacket2(TRANSMIT_PACKET_SIZE,0);
    startPacket1[0] = (char)0xff;
    startPacket1[1] = (char)0xb0;
    startPacket1[2] = (char)0x01;
    startPacket1[3] = (char)0x02;
    startPacket1[4] = (char)0x03;
    startPacket1[TRANSMIT_PACKET_SIZE - 1] = CCrc8::calc((unsigned char*)(startPacket1.data()), TRANSMIT_PACKET_SIZE - 1);

    startPacket2[0] = (char)0xff;
    startPacket2[1] = (char)0xb0;
    startPacket2[2] = (char)0x04;
    startPacket2[3] = (char)0x05;
    startPacket2[4] = (char)0x06;
    startPacket2[5] = 0x10 + frequency;
    startPacket2[TRANSMIT_PACKET_SIZE - 1] = CCrc8::calc((unsigned char*)(startPacket2.data()), TRANSMIT_PACKET_SIZE - 1);
    serial.write(startPacket1);
    serial.flush();
    if(serial.waitForBytesWritten(2000)){
        if(serial.waitForReadyRead(2000)){
            serial.readAll();
            while (serial.waitForReadyRead(10))
                serial.readAll();
        }
    }else{
        qDebug() << "Start packet write timeout!";
    }
    serial.write(startPacket2);
    serial.flush();
}
bool ActionModuleSerialVersion::changePorts(int portNum){
    if(portNum < ports.size() && portNum >= 0){
        serial.setPortName(ports[portNum]);
        return true;
    }
    serial.setPortName("");
    return false;
}
bool ActionModuleSerialVersion::changeFrequency(int frequency){
    if(frequency >= 0 && frequency <= 15){
        this->frequency = frequency;
        zpm->changeParam("Radio/Frequency",frequency);
        return true;
    }
    return false;
}
void ActionModuleSerialVersion::sendLegacy(const ZSS::Protocol::Robots_Command& commands){
    int size = commands.command_size();
    int count = 0;
    for(int i=0;i<size;i++){
        if(count == 4){
            serial.write(this->tx);
            tx[0] = tx[0]&0xf0;
            count = 0;
        }
        auto& command = commands.command(i);
        encodeLegacy(command,this->tx,count++);
    }
    serial.write(this->tx.data(),TRANSMIT_PACKET_SIZE);
    serial.flush();
    //qDebug() << tx.toHex();
}
bool ActionModuleSerialVersion::openSerialPort(){
    if (serial.open(QIODevice::ReadWrite)) {
        qDebug() << "SerialPort connected... : " << serial.portName();
        return true;
    }
    qDebug() << "SerialPort connect failed... : " << serial.portName();
    return false;
}
bool ActionModuleSerialVersion::closeSerialPort(){
    if (serial.isOpen()) {
        serial.close();
        qDebug() << "SerialPort Disconnected... : " << serial.portName();
        return true;
    }
    return false;
}
void ActionModuleSerialVersion::readData(){
    rx = serial.readAll();
    auto& data = rx;
    int id = 0;
    bool infrared = false;
    bool flat = false;
    bool chip = false;
    int battery = 0;
    int capacitance = 0;
    if(data[0] == (char)0xff){
        if(data[1] == (char)0x02){
            id       = (quint8)data[2];
            infrared = (quint8)data[3] & 0x40;
            flat     = (quint8)data[3] & 0x20;
            chip     = (quint8)data[3] & 0x10;
        }
        else if(data[1] == (char)0x01){
            id          = (quint8)data[2];
            battery     = (quint8)data[3];
            capacitance = (quint8)data[4];
        }
    }
    //qDebug() << id << ' ' << infrared << ' ' << flat << ' ' << chip << ' ' << battery << ' ' << capacitance;
}
namespace{
void encodeLegacy(const ZSS::Protocol::Robot_Command& command,QByteArray& tx,int num){
    // num 0 ~ 3
    // id  0 ~ 11
    int id = command.robot_id();
    // velocity  m/s -> cm/s
    qint16 vx = (qint16)(command.velocity_x()*100);
    qint16 vy = (qint16)(command.velocity_y()*100);
    qint16 vr = (qint16)(command.velocity_r()*40);
    qint16 abs_vx = std::abs(vx);
    qint16 abs_vy = std::abs(vy);
    qint16 abs_vr = std::abs(vr);
    // flat&chip m/s -> cm/s
    // kick   1 : chip   0 : flat
    bool kick = command.kick();
    quint8 power = kickStandardization(id,kick,(quint8)(command.power()*100));
    // dribble -1 ~ +1 -> -3 ~ +3
    qint8 dribble = (qint8)(command.dribbler_spin()*3.1);
    // TODO
    // TXBuff[0] = TXBuff[0] | (1 << (3-i));
    tx[0] = (tx[0])|(1<<(3-num));
    tx[num*4 + 1] = (dribble&0x80)|((quint8)kick<<6)|(dribble<<4 & 0x30)|id;
    tx[num*4 + 2] = (vx>>8 & 0x80)|(abs_vx & 0x7f);
    tx[num*4 + 3] = (vy>>8 & 0x80)|(abs_vy & 0x7f);
    tx[num*4 + 4] = (vr>>8 & 0x80)|(abs_vr & 0x7f);
    tx[num  + 17] = (abs_vx>>1 & 0xc0)|(abs_vy>>3 & 0x30)|(abs_vr>>7 & 0x0f);
    tx[num  + 21] = power;
}
quint8 kickStandardization(quint8 id,bool mode,quint8 power){
    // TODO
    return power;
}
} // namespace ZSS::anonymous
}

