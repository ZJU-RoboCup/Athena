#ifndef DEALROBOT_H
#define DEALROBOT_H
#include <singleton.hpp>
#include <messageformat.h>

class CDealrobot
{
public:
    CDealrobot();
    void run (bool);
    void MergeRobot();//
    void selectRobot();//

private:
    Robot robotSeqence[2][PARAM::ROBOTMAXID][PARAM::CAMERA];
    ReceiveVisionMessage result;
    void init();
    double posDist(Pos2d, Pos2d);
};
typedef Singleton <CDealrobot> Dealrobot;

#endif // DEALROBOT_H
