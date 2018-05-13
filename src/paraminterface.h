#ifndef PARAMINTERFACE_H
#define PARAMINTERFACE_H
#include <QAbstractItemModel>
#include "parammanager.h"
#include <regex>
class ParamInterface : public QAbstractListModel{
    Q_OBJECT
public:
    explicit ParamInterface(QObject *parent = Q_NULLPTR)
        : QAbstractListModel(parent) {
        keys = ZSS::ZParamManager::instance()->allKeys();
    }
    ~ParamInterface(){

    }

    enum Roles  {
        KeyRole = Qt::UserRole + 1,
        TypeRole = Qt::UserRole + 2,
        ValueRole = Qt::UserRole + 3
    };
    Q_ENUM(Roles)
    virtual int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE {
        Q_UNUSED(parent);
        return keys.length();
    }
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE {
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
    // interface for qml call the origin setdata();
    Q_INVOKABLE bool setData(const int row,const int column,const QVariant& value){
        if(column == 2){
            return setData(this->index(row,column),value,ParamInterface::ValueRole);
        }
        return false;
    }
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override{
        qDebug() << keys[index.row()] << value.toString();
        ZSS::ZParamManager::instance()->changeParam(keys[index.row()],value);
        emit dataChanged(index, index);
        return true;
    }
    QModelIndex parent(const QModelIndex &index) const override{
        return QModelIndex();
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override{
        return 3;
    }
    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        return Qt::ItemIsEditable | QAbstractListModel::flags(index);
    }
    QHash<int,QByteArray> roleNames() const Q_DECL_OVERRIDE
    {
         QHash<int, QByteArray> result = QAbstractItemModel::roleNames();
         result.insert(KeyRole, QByteArrayLiteral("settingName"));
         result.insert(TypeRole, QByteArrayLiteral("settingType"));
         result.insert(ValueRole, QByteArrayLiteral("settingValue"));
         return result;
    }
private:
    QString judgeType(const QVariant& value) const{
        const static std::regex boolExp("true|false",std::regex_constants::icase);
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
    }
    QStringList keys;
};

#endif // PARAMINTERFACE_H
