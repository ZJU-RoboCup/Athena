#include <QtDebug>
#include <QElapsedTimer>
#include "refereebox.h"
#include "parammanager.h"
#include "staticparams.h"
#include "proto/cpp/referee.pb.h"
#include <QNetworkInterface>
void multicastCommand(int state);

RefereeBox::RefereeBox(QObject *parent) : QObject(parent),currentCommand(GameState::HALTED){
    ZSS::ZParamManager::instance()->loadParam(port,"Referee/port",10003);
    ZSS::ZParamManager::instance()->loadParam(groupAddress,"Referee/address","224.5.23.1");
    sendSocket.setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
    commandCounter = 1;
}
RefereeBox::~RefereeBox(){
}
void RefereeBox::changeCommand(int command){
    commandMutex.lock();
    currentCommand = command;
    commandCounter++;
    commandMutex.unlock();
}
void RefereeBox::multicastCommand(){
    SSL_Referee ssl_referee;
    ssl_referee.set_packet_timestamp(0);//todo
    ssl_referee.set_stage(SSL_Referee_Stage_NORMAL_FIRST_HALF);//todo
    ssl_referee.set_stage_time_left(0);//todo

    commandMutex.lock();
    ssl_referee.set_command(SSL_Referee_Command(currentCommand));
    ssl_referee.set_command_counter(commandCounter);//todo
    commandMutex.unlock();

    ssl_referee.set_command_timestamp(0);//todo
    SSL_Referee_TeamInfo *yellow = ssl_referee.mutable_yellow();
    yellow->set_name("ZJUNlict");//todo
    yellow->set_score(0);//todo
    yellow->set_red_cards(0);//todo
    //yellow->set_yellow_card_times(0,0); //todo
    yellow->set_yellow_cards(0);//todo
    yellow->set_timeouts(0.0);//todo
    yellow->set_timeout_time(0.0);//todo
    yellow->set_goalie(0); //todo
    SSL_Referee_TeamInfo *blue = ssl_referee.mutable_blue();
    blue->set_name("ZJUNlict");
    blue->set_score(0);//todo
    blue->set_red_cards(0);//todo
    //blue->set_yellow_card_times(0,0); //todo
    blue->set_yellow_cards(0);//todo
    blue->set_timeouts(0.0);//todo
    blue->set_timeout_time(0.0);//todo
    blue->set_goalie(0); //todo
    int size = ssl_referee.ByteSize();
    QByteArray buffer(size,0);
    ssl_referee.SerializeToArray(buffer.data(), buffer.size());
    sendSocket.writeDatagram(buffer.data(), buffer.size(),
                                 QHostAddress(ZSS::REF_ADDRESS), port);
}
