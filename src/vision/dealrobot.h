#ifndef DEALROBOT_H
#define DEALROBOT_H
#include "kalmanfilter.h"
#include "kalmanfilterdir.h"
#include <singleton.hpp>
#include <messageformat.h>

class CDealrobot
{
public:
    CDealrobot();
    void run();
    void MergeRobot();//
    void selectRobot();//
//    void filteRobot(Robot &);

private:
    Robot robotSeqence[2][PARAM::ROBOTMAXID][PARAM::CAMERA];
    ReceiveVisionMessage result;
    Robot sortTemp[2][PARAM::ROBOTMAXID];
    Robot lastRobot[2][PARAM::ROBOTMAXID];
    Robot currentRobot[2][PARAM::ROBOTMAXID];
    void init();
    double posDist(CGeoPoint, CGeoPoint);
    double calculateWeight(int camID,CGeoPoint roboPos);
    void sortRobot(int);
    bool isOnField(CGeoPoint);
    KalmanFilter _kalmanFilter[2][PARAM::ROBOTMAXID];
    KalmanFilterDir _dirFilter[2][PARAM::ROBOTMAXID];
    bool filteDir;
    int validNum[2];
    int minBelieveFrame,maxLostFrame,fieldWidth,fieldHeight;
    double upPossible,decidePossible,downPossible;
};
typedef Singleton <CDealrobot> Dealrobot;

#endif // DEALROBOT_H
