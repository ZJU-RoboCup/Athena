#ifndef PARAMINTERFACE_H
#define PARAMINTERFACE_H
#include <QAbstractItemModel>
class ParamInterface : public QAbstractListModel{
    Q_OBJECT
public:
    explicit ParamInterface(QObject *parent = Q_NULLPTR);
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
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    // interface for qml call the origin setdata();
    Q_INVOKABLE bool setData(const int row,const int column,const QVariant& value);
    Q_INVOKABLE QString getType(const int row);
    Q_INVOKABLE void reload();
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QModelIndex parent(const QModelIndex &index) const override{
        return QModelIndex();
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override{
        return 3;
    }
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QHash<int,QByteArray> roleNames() const Q_DECL_OVERRIDE;
private:
    QString judgeType(const QVariant& value) const;
    QStringList keys;
};

#endif // PARAMINTERFACE_H
