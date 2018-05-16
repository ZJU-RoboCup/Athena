#include "interaction.h"
#include "visionmodule.h"
#include "globaldata.h"
#include "maintain.h"
#include "actionmodule.h"
#include "globalsettings.h"
#include <QProcess>
namespace{
    QProcess *medusaProcess = nullptr;
    QProcess *simProcess = nullptr;
}
Interaction::Interaction(QObject *parent) : QObject(parent) {
}
Interaction::~Interaction() {
//    if (simProcess != nullptr) {
//        if (simProcess->isOpen()) {
//            simProcess->close();
//        }
//        delete simProcess;
//        simProcess = nullptr;
//        QTextStream(stdout) << "\n------------------------------------\nexit Sim\n------------------------------------\n";
//    }
    if (medusaProcess != nullptr) {
        if (medusaProcess->isOpen()) {
            medusaProcess->close();
        }
        delete medusaProcess;
        medusaProcess = nullptr;
        QTextStream(stdout) << "\n------------------------------------\nexit Medusa\n------------------------------------\n";
    }
}
void Interaction::setVision(bool needStart){
    if (needStart) {
        VisionModule::instance()->udpSocketConnect();
        Maintain::instance()->udpSocketConnect();
    }else{
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
bool Interaction::controlMedusa(bool control,bool color,bool side,bool version){
    if(control == false){
        if (medusaProcess != nullptr) {
            if (medusaProcess->isOpen()) {
                medusaProcess->close();
            }
            delete medusaProcess;
            medusaProcess = nullptr;
            QTextStream(stdout) << "\n------------------------------------\nexit Medusa\n------------------------------------\n";
        }
    }else{
        medusaProcess = new QProcess();
        QStringList args;
        args << (color ? "cy" : "cb") << (side ? "sr" : "sl");
        QString name = version ? "./MedusaD.exe" : "./Medusa.exe";
        connect(medusaProcess,SIGNAL(readyReadStandardOutput()),this,SLOT(medusaPrint()));
        medusaProcess->start(name, args);
        QTextStream(stdout) << "\n------------------------------------\n" << "running " << name << "\n------------------------------------\n";
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
        QString name = "./ZSim.exe";//version ? "./MedusaD.exe" : "./Medusa.exe";
        simProcess->start(name);
        QTextStream(stdout) << "\n------------------------------------\n" << "running " << name << "\n------------------------------------\n";
    }
    return true;
}
void Interaction::medusaPrint(){
    QTextStream(stdout) << medusaProcess->readAllStandardOutput();
}
