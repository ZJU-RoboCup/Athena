#ifndef TRANSFORM_H
#define TRANSFORM_H
#include "singleton.hpp"
#include <QObject>
class CTransform : public QObject {
public:
    CTransform();
    void run(bool);

signals:
    void needDraw();
};
typedef Singleton<CTransform> Transform;
#endif // TRANSFORM_H
