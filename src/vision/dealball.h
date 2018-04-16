#ifndef DEALBALL_H
#define DEALBALL_H

#include <singleton.hpp>
#include "globaldata.h"
#include "messageformat.h"
#include "geometry.h"


class CDealball
{
public:
    CDealball();
    void run(bool);
    void mergeBall();

private:
    int actualBallNum=0;
    Ball ballSequence[PARAM::BALLNUM][PARAM::CAMERA];
    ReceiveVisionMessage result;
    double posDist(CGeoPoint,CGeoPoint);
    void init();
};
typedef Singleton<CDealball> Dealball;
#endif // DEALBALL_H
