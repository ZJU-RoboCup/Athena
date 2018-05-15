#include "parammanager.h"

namespace ZSS {
ParamManager::ParamManager():
    settings("./zss.ini", QSettings::IniFormat){
}
ParamManager::~ParamManager(){
}
bool ParamManager::loadParam(QChar& value, const QString& key, QChar defaultValue){
    value = settings.value(key, defaultValue).toChar();
    if (!settings.contains(key)){
        settings.setValue(key, defaultValue);
        return false;
    }
    return true;
}
bool ParamManager::loadParam(int& value, const QString& key, int defaultValue){
    value = settings.value(key, defaultValue).toInt();
    if (!settings.contains(key)){
        settings.setValue(key, defaultValue);
        return false;
    }
    return true;
}
bool ParamManager::loadParam(double& value, const QString& key, double defaultValue){
    value = settings.value(key, defaultValue).toDouble();
    if (!settings.contains(key)){
        settings.setValue(key, defaultValue);
        return false;
    }
    return true;
}
bool ParamManager::loadParam(QString& value, const QString& key , QString defaultValue){
    value = settings.value(key, defaultValue).toString();
    if (!settings.contains(key)){
        settings.setValue(key, defaultValue);
        return false;
    }
    return true;
}
bool ParamManager::loadParam(bool& value, const QString& key, bool defaultValue){
    value = settings.value(key, defaultValue).toBool();
    if (!settings.contains(key)){
        settings.setValue(key, defaultValue);
        return false;
    }
    return true;
}
bool ParamManager::changeParam(const QString & key, const QVariant & value){
    if (!settings.contains(key)){
        return false;
    }
    settings.setValue(key,value);
    return true;
}

}
