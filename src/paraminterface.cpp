#include "paraminterface.h"
#include "parammanager.h"
#include <regex>
ParamInterface::ParamInterface(QObject *parent)
    : QAbstractListModel(parent) {
    keys = ZSS::ZParamManager::instance()->allKeys();
}
QString ParamInterface::judgeType(const QVariant& value) const{
    const static std::regex boolExp("true|false|t|f",std::regex_constants::icase);
    const static std::regex doubleExp("^(-?)(0|([1-9][0-9]*))(\\.[0-9]+)?$");
    const static std::regex integerExp("^(0|[1-9][0-9]*)$");
    if(std::regex_match(value.toString().toUtf8().constData(),boolExp))
        return "Bool";
    else if(std::regex_match(value.toString().toUtf8().constData(),integerExp))
        return "Int";
    else if(std::regex_match(value.toString().toUtf8().constData(),doubleExp))
        return "Double";
    else
        return "String";
    return "Unknown";
}
QHash<int,QByteArray> ParamInterface::roleNames() const {
     QHash<int, QByteArray> result = QAbstractItemModel::roleNames();
     result.insert(KeyRole, QByteArrayLiteral("settingName"));
     result.insert(TypeRole, QByteArrayLiteral("settingType"));
     result.insert(ValueRole, QByteArrayLiteral("settingValue"));
     return result;
}
Qt::ItemFlags ParamInterface::flags(const QModelIndex &index) const {
    return Qt::ItemIsEditable | QAbstractListModel::flags(index);
}
bool ParamInterface::setData(const QModelIndex &index, const QVariant &value,
             int role){
    qDebug() << keys[index.row()] << value.toString();
    ZSS::ZParamManager::instance()->changeParam(keys[index.row()],value);
    emit dataChanged(index, index);
    return true;
}
bool ParamInterface::setData(const int row,const int column,const QVariant& value){
    if(column == 2){
        return setData(this->index(row,column),value,ParamInterface::ValueRole);
    }
    return false;
}
QVariant ParamInterface::data(const QModelIndex &index, int role) const {
    if (index.isValid() && role >= KeyRole) {
        QVariant value = ZSS::ZParamManager::instance()->value(keys[index.row()]);
        switch (role) {
        case KeyRole:
            return QVariant(keys[index.row()]);
        case TypeRole:
            return QVariant(judgeType(value));
        case ValueRole:
            return QVariant(value);
        default:
            break;
        }
    }
    return QVariant("Error");
}
QString ParamInterface::getType(const int row){
    if(row >= 0 && row < keys.size())
        return judgeType(ZSS::ZParamManager::instance()->value(keys[row]));
    return "Invalid";
}
void ParamInterface::reload(){
    beginResetModel();
    ZSS::ZParamManager::instance()->sync();
    keys = ZSS::ZParamManager::instance()->allKeys();
    endResetModel();
    this->resetInternalData();
}
