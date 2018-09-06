#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <QObject>
#include <QUdpSocket>
#include "singleton.hpp"
// temp before merge medusa TODO
class Debugger : public QObject
{
    Q_OBJECT
public:
    explicit Debugger(QObject *parent = nullptr);
public slots:
    void receive();
private:
    QUdpSocket receiver;
};
typedef Singleton<Debugger> ZDebugger;
#endif // DEBUGGER_H
