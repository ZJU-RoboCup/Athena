#ifndef VISIONROBOT_H
#define VISIONROBOT_H

#include <singleton.hpp>
#include <QObject>

class CVisionrobot : public QObject
{
    Q_OBJECT
public:
    CVisionrobot();
    void run(bool);

signals:

public slots:
};
typedef Singleton<CVisionrobot> Visionrobot;



#endif // VISIONROBOT_H
