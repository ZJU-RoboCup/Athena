#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H
#include <QObject>
#include "geometry.h"
#include <singleton.hpp>
class CGlobalSettings : public QObject{
    Q_OBJECT
public:
    CGlobalSettings(QObject *parent = 0);
    double minimumX,maximumX,minimumY,maximumY;
    bool inChosenArea(float x, float y);
    bool inChosenArea(CGeoPoint);
    void setArea(int,int,int,int);
signals:
    void needRepaint();
};
typedef Singleton<CGlobalSettings> GlobalSettings;
#endif // GLOBALSETTINGS_H
