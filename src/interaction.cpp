#include "interaction.h"
#include "visionmodule.h"
#include "globaldata.h"
#include "maintain.h"
#include "actionmodule.h"
#include "globalsettings.h"
#include "simulator.h"
#include "parammanager.h"
#include <QProcess>
namespace{
    QProcess *medusaProcess = nullptr;
    QProcess *medusaProcess2 = nullptr;
    QProcess *simProcess = nullptr;
    QProcess *crazyProcess = nullptr;
    QProcess *monitorProcess = nullptr;
}
Interaction::Interaction(QObject *parent) : QObject(parent) {
}
Interaction::~Interaction() {
    if (simProcess != nullptr) {
        if (simProcess->isOpen()) {
            simProcess->close();
        }
        delete simProcess;
        simProcess = nullptr;
        QTextStream(stdout) << "\n------------------------------------\nexit Sim\n------------------------------------\n";
    }
    if (medusaProcess != nullptr) {
        if (medusaProcess->isOpen()) {
            medusaProcess->close();
        }
        delete medusaProcess;
        medusaProcess = nullptr;
        QTextStream(stdout) << "\n------------------------------------\nexit Medusa\n------------------------------------\n";
    }
    if (medusaProcess2 != nullptr) {
        if (medusaProcess2->isOpen()) {
            medusaProcess2->close();
        }
        delete medusaProcess2;
        medusaProcess2 = nullptr;
        QTextStream(stdout) << "\n------------------------------------\nexit 2 Medusa\n------------------------------------\n";
    }
    if (crazyProcess != nullptr) {
        if (crazyProcess->isOpen()) {
            crazyProcess->close();
        }
        delete crazyProcess;
        crazyProcess = nullptr;
        QTextStream(stdout) << "\n------------------------------------\nexit Crazy\n------------------------------------\n";
    }
    if (monitorProcess != nullptr) {
        if (monitorProcess->isOpen()) {
            monitorProcess->close();
        }
        delete monitorProcess;
        monitorProcess = nullptr;
        QTextStream(stdout) << "\n------------------------------------\nexit Monitor\n------------------------------------\n";
    }
}
void Interaction::setVision(bool needStart,bool real){
    if (needStart) {
        ZSS::ZParamManager::instance()->changeParam("Alert/IsSimulation",!real);
        VisionModule::instance()->udpSocketConnect(real);
        Maintain::instance()->udpSocketConnect();
    }else{
        ZSS::ZParamManager::instance()->changeParam("Alert/IsSimulation",!real);
        VisionModule::instance()->udpSocketDisconnect();
        Maintain::instance()->udpSocketDisconnect();
    }
}
void Interaction::controlCamera(int index, bool state){
    GlobalData::instance()->cameraControl[index] = state;
}
void Interaction::controlProcess(int index,bool state){
    GlobalData::instance()->processControl[index] = state;
}

bool Interaction::connectSerialPort(bool sw){
    if(sw){
        return ZSS::ZActionModule::instance()->init();
    }
    return ZSS::ZActionModule::instance()->closeSerialPort();
}

bool Interaction::changeSerialFrequency(int frequency){
    return ZSS::ZActionModule::instance()->changeFrequency(frequency);
}

bool Interaction::changeSerialPort(int index){
    return ZSS::ZActionModule::instance()->changePorts(index);
}

QStringList Interaction::getSerialPortsList(){
    return ZSS::ZActionModule::instance()->updatePortsList();
}
int Interaction::getFrequency(){
    return ZSS::ZActionModule::instance()->getFrequency();
}
bool Interaction::controlMonitor(bool control){
    if(control == false){
        if (monitorProcess != nullptr) {
            if (monitorProcess->isOpen()) {
                monitorProcess->close();
            }
            delete monitorProcess;
            monitorProcess = nullptr;
            QTextStream(stdout) << "\n------------------------------------\nexit Monitor\n------------------------------------\n";
        }
    }else{
        monitorProcess = new QProcess();
        QString name = "./ProcessAlive.exe";
        monitorProcess->start(name);
        QTextStream(stdout) << "\n------------------------------------\n" << "running " << name << "\n------------------------------------\n";
    }
    return true;
}
bool Interaction::controlMedusa(bool control,bool color,bool side){
    if(control == false){
        if (medusaProcess != nullptr) {
            if (medusaProcess->isOpen()) {
                medusaProcess->close();
            }
            delete medusaProcess;
            medusaProcess = nullptr;
            QTextStream(stdout) << "\n------------------------------------\nexit Medusa\n------------------------------------\n";
        }
        emit GlobalSettings::instance()->clearOutput();
    }else{
        ZSS::ZParamManager::instance()->changeParam("ZAlert/IsYellow",color);
        ZSS::ZParamManager::instance()->changeParam("ZAlert/IsRight",side);
        medusaProcess = new QProcess();
        QStringList args;
        args << (color ? "cy" : "cb") << (side ? "sr" : "sl");
        QString name = "./Medusa.exe";
        connect(medusaProcess,SIGNAL(readyReadStandardOutput()),this,SLOT(medusaPrint()));
        medusaProcess->start(name, args);
        QTextStream(stdout) << "\n------------------------------------\n" << "running " << name << "\n------------------------------------\n";
    }
    return true;
}
bool Interaction::controlMedusa2(bool control,bool color,bool side){
    if(control == false){
        if (medusaProcess2 != nullptr) {
            if (medusaProcess2->isOpen()) {
                medusaProcess2->close();
            }
            delete medusaProcess2;
            medusaProcess2 = nullptr;
            QTextStream(stdout) << "\n------------------------------------\nexit 2 Medusa\n------------------------------------\n";
        }
    }else{
        ZSS::ZParamManager::instance()->changeParam("ZAlert/IsYellow",color);
        ZSS::ZParamManager::instance()->changeParam("ZAlert/IsRight",side);
        medusaProcess2 = new QProcess();
        QStringList args;
        args << (color ? "cy" : "cb") << (side ? "sr" : "sl");
        QString name = "./Medusa.exe";
        medusaProcess2->start(name, args);
        QTextStream(stdout) << "\n------------------------------------\n" << "running 2 " << name << "\n------------------------------------\n";
    }
    return true;
}
bool Interaction::controlSim(bool control){
    if(control == false){
        if (simProcess != nullptr) {
            if (simProcess->isOpen()) {
                simProcess->close();
            }
            delete simProcess;
            simProcess = nullptr;
            QTextStream(stdout) << "\n------------------------------------\nexit Sim\n------------------------------------\n";
        }
    }else{
        simProcess = new QProcess();
        QString name = "./grsim";
        simProcess->start(name);
        QTextStream(stdout) << "\n------------------------------------\n" << "running " << name << "\n------------------------------------\n";
    }
    return true;
}
bool Interaction::controlCrazy(bool control){
    if(control == false){
        if (crazyProcess != nullptr) {
            if (crazyProcess->isOpen()) {
                crazyProcess->close();
            }
            delete crazyProcess;
            crazyProcess = nullptr;
            QTextStream(stdout) << "\n------------------------------------\nexit Crazy\n------------------------------------\n";
        }
    }else{
        crazyProcess = new QProcess();
        QString name = "./Crazy2018.exe";
        crazyProcess->start(name);
        QTextStream(stdout) << "\n------------------------------------\n" << "running " << name << "\n------------------------------------\n";
    }
    return true;
}
bool Interaction::changeMedusaSettings(bool color,bool side){
    ZSS::ZParamManager::instance()->changeParam("ZAlert/IsYellow",color);
    ZSS::ZParamManager::instance()->changeParam("ZAlert/IsRight",side);
    return true;
}
void Interaction::medusaPrint(){
     emit GlobalSettings::instance()->addOutput(medusaProcess->readAllStandardOutput());
}
void Interaction::robotControl(int id,int team){
    Simulator::instance()->controlRobot(id,team==1);
}
