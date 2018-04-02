#ifndef COLLISIONDETECT_H
#define COLLISIONDETECT_H
#include "singleton.hpp"
#include "globaldata.h"

#define DETECT_DIST 30 //detect whether the ball in 30cm of vechile
#define LENGTH_THERESHOLD 580
#define WIDTH_THERESHOLD 440
#define MAX_FRAME_NUM 20
#define SPLIT_THRESHOLD 4 //split
class CCollisionDetect
{
public:
    CCollisionDetect();
    //virtual ~CCollisionDetect();
    void visionAlart();
private:
    int LinePoint[MAX_FRAME_NUM];
    int _RecordFlag = 0;
    int _RecordIndex = -1;
    int PointN;
    int LastTouch=0;//0 for NOT sure; 1 for our ;2 for their; 3 for wall;
    int LastTouchNumber = 0;//car number; 0 for not sure or wall
    void analyzeData();
    bool ballCloseEnough2Analyze(int);
    bool ballIsOnEdge(Pos2d);
};
typedef Singleton<CCollisionDetect> CollisionDetect;

#endif // COLLISIONDETECT_H
