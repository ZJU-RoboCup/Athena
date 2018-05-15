#include "simulator.h"
#include "staticparams.h"
#include <QtDebug>
namespace{
}
CSimulator::CSimulator(QObject *parent) : QObject(parent)
{
    sendSocket.setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
}
void CSimulator::setBall(double x,double y,double vx,double vy){
    grSim_Packet packet;
    auto* replacement = packet.mutable_replacement();
    auto* ball = replacement->mutable_ball();
    ball->set_x(x);
    ball->set_y(y);
    ball->set_vx(vx);
    ball->set_vy(vy);
    send(&packet);
}
void CSimulator::setRobot(double x,double y,int id,bool team,double dir){
    grSim_Packet packet;
    auto* replacement = packet.mutable_replacement();
    auto* robot = replacement->add_robots();
    robot->set_x(x);
    robot->set_y(y);
    robot->set_id(id);
    robot->set_dir(dir);
    robot->set_yellowteam(team);
    robot->set_turnon(true);
    send(&packet);
}
void CSimulator::moveinRobot(int id,bool team){
}
void CSimulator::moveoutRobot(int id,bool team){
}
void CSimulator::send(grSim_Packet* packet){
    int size = packet->ByteSize();
    QByteArray buffer(size,0);
    packet->SerializeToArray(buffer.data(), size);
    sendSocket.writeDatagram(buffer,QHostAddress(ZSS::LOCAL_ADDRESS),ZSS::Athena::SIM_SEND);
}
