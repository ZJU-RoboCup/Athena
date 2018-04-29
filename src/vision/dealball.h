#ifndef DEALBALL_H
#define DEALBALL_H

#include <singleton.hpp>
#include "globaldata.h"
#include "messageformat.h"
#include "geometry.h"
#include "kalmanfilter.h"


class CDealball
{
public:
    CDealball();
    void run(bool);
    void filteBall();
    double getBallSpeed() {return result.ball[0].velocity.mod();}
private:
    int actualBallNum=0;
    Ball ballSequence[PARAM::BALLNUM][PARAM::CAMERA];
    Ball lastBall;
    ReceiveVisionMessage result;
    double posDist(CGeoPoint,CGeoPoint);
    void init();
    void mergeBall();
    KalmanFilter _kalmanFilter;
    bool validBall;

};
typedef Singleton<CDealball> Dealball;
#endif // DEALBALL_H
