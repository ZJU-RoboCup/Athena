#ifndef __INTERACTION4FIELD_H__
#define __INTERACTION4FIELD_H__

#include <QObject>
#include "visionmodule.h"
#include "field.h"
class Interaction4Field : public QObject
{
    Q_OBJECT
public:
    explicit Interaction4Field(QObject *parent = 0);
    Q_INVOKABLE int getFPS(){ return VisionModule::instance()->getFPS();}
    Q_INVOKABLE void setArea(int a,int b,int c,int d);
    ~Interaction4Field();
public:
signals:
public slots:
};

#endif // __INTERACTION4FIELD_H__
