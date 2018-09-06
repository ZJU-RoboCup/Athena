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
    double ballPlacementX,ballPlacementY;
    bool inChosenArea(float x, float y);
    bool inChosenArea(CGeoPoint);
    void setArea(int,int,int,int);
    void resetArea();
    void setBallPlacementPos(int,int);
signals:
    void needRepaint();
    void addOutput(const QString&);
    void clearOutput();
};
typedef Singleton<CGlobalSettings> GlobalSettings;
#endif // GLOBALSETTINGS_H
