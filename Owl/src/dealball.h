#ifndef DEALBALL_H
#define DEALBALL_H

#include <singleton.hpp>
#include "globaldata.h"
#include "messageformat.h"


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
    double posDist( Pos2d,Pos2d);
};
typedef Singleton<CDealball> Dealball;
#endif // DEALBALL_H
