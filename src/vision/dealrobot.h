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
    Robot sortTemp[2][PARAM::ROBOTMAXID];
    void init();
    double posDist(CGeoPoint, CGeoPoint);
    void sortRobot(int);
};
typedef Singleton <CDealrobot> Dealrobot;

#endif // DEALROBOT_H
