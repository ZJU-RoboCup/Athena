#ifndef TRANSFORM_H
#define TRANSFORM_H
#include "singleton.hpp"
class CTransform
{
public:
    CTransform();
    void run(bool);
};
typedef Singleton<CTransform> Transform;
#endif // TRANSFORM_H
