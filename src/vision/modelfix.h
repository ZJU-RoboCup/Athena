#ifndef CMODELFIX_H
#define CMODELFIX_H
#include <singleton.hpp>
#include "staticparams.h"
class ModelFixSingle;
class CModelFix
{
public:
    CModelFix();
    ~CModelFix();
    void run(bool);
private:
    ModelFixSingle *mf[PARAM::CAMERA];
};
typedef Singleton<CModelFix> ModelFix;
#endif // CMODELFIX_H
