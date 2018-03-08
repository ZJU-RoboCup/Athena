#ifndef __MESSAGEFORMAT_H__
#define __MESSAGEFORMAT_H__
#include "staticparams.h"
#include <iostream>
namespace {
    struct Pos2d{
        double x;
        double y;
        Pos2d():x(-9999),y(-9999){}
    };
    struct Ball{
        Pos2d pos;
        double height;
        Ball():pos(),height(0){}
        bool fill(double x,double y,double height = 0){
            pos.x = x; pos.y = y; this->height = height;
            return true;
        }
        bool fill(const Ball& ball){
            return fill(ball.pos.x,ball.pos.y,ball.height);
        }
    };
    struct Robot{
        unsigned short id;
        Pos2d pos;
        double angel;
        Robot():id(-1){}
        bool fill(unsigned short id,double x,double y,double angel){
            this->id = id; pos.x = x; pos.y = y; this->angel = angel;
            return true;
        }
        bool fill(const Robot& robot){
            return fill(robot.id,robot.pos.x,robot.pos.y,robot.angel);
        }
    };
}
const int BLUE = 0;
const int YELLOW = 1;
//* Robot store the data, Index and Size exist for the index search for robots;
//* Index : -1 means not exist, or it means the index in Robot Array;
//* VisionMessage use for the final processed vision data.
class OriginMessage{
public:
    unsigned short robotSize[2];
    unsigned short ballSize;
    Robot robot[2][PARAM::ROBOTNUM];
    Ball ball[PARAM::BALLNUM];
    int robotIndex[2][PARAM::ROBOTMAXID];
    OriginMessage():ballSize(0){
        robotSize[BLUE] = robotSize[YELLOW] = 0;
        std::fill_n(&robotIndex[0][0],2*PARAM::ROBOTMAXID,-1);
    }
    void init(){
        robotSize[BLUE] = robotSize[YELLOW] = ballSize = 0;
        std::fill_n(&robotIndex[0][0],2*PARAM::ROBOTMAXID,-1);
    }
    bool addBall(double x,double y,double height = 0){
        return ballSize >= PARAM::BALLNUM ? false : ball[ballSize++].fill(x,y,height);
    }
    bool addRobot(int color,unsigned short id,double x,double y,double angel){
        if(robotSize[color] >= PARAM::ROBOTNUM)
            return false;
        robotIndex[color][id] = robotSize[color];
        return robot[color][robotSize[color]++].fill(id,x,y,angel);
    }
    bool addRobot(int color,const Robot& r){
        if(robotSize[color] >= PARAM::ROBOTNUM)
            return false;
        robotIndex[color][r.id] = robotSize[color];
        return robot[color][robotSize[color]++].fill(r);
    }
    bool addBall(const Ball& b){
        return ballSize >= PARAM::BALLNUM ? false : ball[ballSize++].fill(b);
    }
};

class ReceiveVisionMessage:public OriginMessage{
public:
    unsigned short camID;
    unsigned long long frame;
    double captureTime; // ![1] TODO -> gjy

    ReceiveVisionMessage():camID(-1),frame(-1),captureTime(-1){}
    void clear(){
        captureTime = frame = camID = -1;
        ballSize = robotSize[BLUE] = robotSize[YELLOW] = 0;
    }
};

#endif // __MESSAGEFORMAT_H__
