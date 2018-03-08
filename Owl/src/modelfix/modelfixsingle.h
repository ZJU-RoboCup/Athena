#ifndef MODELFIXSINGLE_H
#define MODELFIXSINGLE_H
#include "staticparams.h"
struct RobotFollow{
    int lostFrame;
    float confidence;
    RobotFollow():lostFrame(9999),confidence(0){}
};

class ModelFixSingle
{
public:
    ModelFixSingle(int);
    void run();
private:
    RobotFollow robotFollow[2][PARAM::ROBOTMAXID];
    int camera;
};

#endif // MODELFIXSINGLE_H
