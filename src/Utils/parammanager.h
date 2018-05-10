#ifndef PARAMMANAGER_H
#define PARAMMANAGER_H
#include <QSettings>
#include <QDebug>
#include "singleton.hpp"
namespace ZSS {
class ParamManager{
public:
    ParamManager();
    ~ParamManager();
    bool loadParam(QChar&, const QString&, QChar d = 0);
    bool loadParam(int&, const QString&, int d = 0);
    bool loadParam(double&, const QString&, double d = 0);
    bool loadParam(QString&, const QString&, QString d = "");
    bool loadParam(bool&, const QString&, bool d = false);
    bool changeParam(const QString&,const QVariant&);
    QStringList allKeys(){
        return settings.allKeys();
    }
    QVariant value(const QString& key){
        return settings.value(key);
    }
private:
    QSettings settings;
};
typedef Singleton<ParamManager> ZParamManager;
}

#endif // PARAMMANAGER_H

// void QSettings::setPath(Format format, Scope scope, const QString &path)
  // QSettings settings("./myapp.ini", QSettings::IniFormat);
