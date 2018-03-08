#include "immortalsvision.h"

CImmortalsVision::CImmortalsVision(QObject *parent) : QObject(parent) {
    ballBufferIndex = 0;

    for ( int i = 0 ; i < BALL_BUFFER_FRAMES ; i ++ )
    {
        ballBufferX[i] = 0.0f;
        ballBufferY[i] = 0.0f;
    }

    lastRawBall.set_x ( 0.0f );
    lastRawBall.set_y ( 0.0f );

    string fast_filter_path(DATA_PATH); fast_filter_path.append("/ballFilterFast.txt");
    string slow_filter_path(DATA_PATH); slow_filter_path.append("/ballFilterSlow.txt");

    ball_kalman.initialize(fast_filter_path.c_str(), slow_filter_path.c_str());

    for ( int i = 0 ; i < MAX_ROBOTS; i++ )
    {
        robot_kalman[0][i].initialize (fast_filter_path.c_str(), slow_filter_path.c_str());
        robot_kalman[1][i].initialize (fast_filter_path.c_str(), slow_filter_path.c_str());
        rawAngles[0][i] = 0.0f;
        rawAngles[1][i] = 0.0f;
    }

    //InitANN();

    ball_not_seen = MAX_BALL_NOT_SEEN + 1;
    for ( int i = 0 ; i < MAX_ROBOTS ; i ++ ){
        robot_not_seen[0][i] = MAX_ROBOT_NOT_SEEN + 1;
        robot_not_seen[1][i] = MAX_ROBOT_NOT_SEEN + 1;
    }
}
void CImmortalsVision::ProcessVision(WorldState * state){
    ProcessBalls ( state );
    ProcessRobots ( state );
}
