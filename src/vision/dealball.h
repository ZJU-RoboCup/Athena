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
    void run();
    void choseBall();
    bool getValid(){return validBall;}
    double getBallSpeed() {return result.ball[0].velocity.mod();}
private:
    double posDist(CGeoPoint,CGeoPoint);
    bool ballNearVechile(Ball,double);
    void init();
    void mergeBall();
    double calculateWeight(int camID, CGeoPoint);
    Ball ballSequence[PARAM::BALLNUM][PARAM::CAMERA];
    Ball lastBall,currentBall;
    double lastPossible,currentPossible;
    ReceiveVisionMessage result;
    KalmanFilter _kalmanFilter;
    int actualBallNum=0;
    int minBelieveFrame;
    bool validBall;
    double upPossible,downPossible;
    long long _cycle,lastFrame,currentFrame;
};
typedef Singleton<CDealball> Dealball;
#endif // DEALBALL_H
