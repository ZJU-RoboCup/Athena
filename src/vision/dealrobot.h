#ifndef DEALROBOT_H
#define DEALROBOT_H
#include "kalmanfilter.h"
#include <singleton.hpp>
#include <messageformat.h>

class CDealrobot
{
public:
    CDealrobot();
    void run (bool);
    void MergeRobot();//
    void selectRobot();//
//    void filteRobot(Robot &);

private:
    Robot robotSeqence[2][PARAM::ROBOTMAXID][PARAM::CAMERA];
    ReceiveVisionMessage result;
    Robot sortTemp[2][PARAM::ROBOTMAXID];
    void init();
    double posDist(CGeoPoint, CGeoPoint);
    void sortRobot(int);
    KalmanFilter _kalmanFilter[2][PARAM::ROBOTMAXID];
};
typedef Singleton <CDealrobot> Dealrobot;

#endif // DEALROBOT_H
