#ifndef IMMORTALSVISION_H
#define IMMORTALSVISION_H

#include <QObject>
#include <string>
#include "singleton.hpp"
#include "messages_robocup_ssl_detection.pb.h"
#include "messages_robocup_ssl_wrapper.pb.h"
#include "immortals/Source/Common/MedianFilter.h"
#include "immortals/Source/Common/MATHS_REGRESSION_PARABOLIC.h"
#include "immortals/Source/Reality/Vision/Kalman/FilteredObject.h"
#include "immortals/Source/Reality/Vision/Protobuf/ImmortalsProtocols.pb.h"
#include "immortals/Source/Reality/Vision/WorldState.h"
namespace {
    double POWED_DIS(double a,double b,double c,double d) { return (((a-c)*(a-c))+((b-d)*(b-d))); }
    //change for 2018
    const int CAM_COUNT = 8;

    const double PREDICT_STEPS = 5.0f;

    const int MAX_BALLS = 10;
    const int MAX_BALL_NOT_SEEN = 40;

    const int MAX_ROBOT_NOT_SEEN = 600;
    const int MAX_ROBOT_SUBSITUTE = 60;

    const int MERGE_DISTANCE = 5000;

    const int MAX_INCOMING_PACKET_SIZE = 1000;

    const int BALL_BUFFER_FRAMES = 30;
    const std::string DATA_PATH = ".";
    const bool COLOR_BLUE = false;
    const bool COLOR_YELLOW = true;
}
class CImmortalsVision : public QObject
{
    Q_OBJECT
public:
    explicit CImmortalsVision(QObject *parent = nullptr);
    void ProcessVision ( WorldState * );
    void ProcessBalls (  WorldState * );
    void ProcessRobots (  WorldState * );

    int ExtractBlueRobots ( void );
    int ExtractYellowRobots ( void );
    int MergeRobots ( int num );
    void FilterRobots ( int num , bool own );
    void predictRobotsForward( WorldState * );

    int ExtractBalls ( void );
    int MergeBalls ( int num );
    void FilterBalls ( int num , WorldState * );
    void predictBallForward( WorldState * );
    void calculateBallHeight ( void );

    void SendStates ( WorldState * );
private:
    float ballBufferX[BALL_BUFFER_FRAMES];
    float ballBufferY[BALL_BUFFER_FRAMES];
    int ballBufferIndex;

    Parabolic ballParabolic;

    FilteredObject ball_kalman;
    FilteredObject robot_kalman[2][MAX_ROBOTS];

    MedianFilter<float> AngleFilter[2][MAX_ROBOTS];
    float rawAngles[2][MAX_ROBOTS];

    int ball_not_seen;
    int robot_not_seen[2][MAX_ROBOTS];

    SSL_DetectionBall lastRawBall;

    RobotState robotState[2][MAX_ROBOTS];

    robotDataMsg robotPacket[2][MAX_ROBOTS];

    SSL_WrapperPacket packet;
    SSL_DetectionFrame frame[CAM_COUNT];
    SSL_DetectionBall d_ball[MAX_BALLS*CAM_COUNT];
    SSL_DetectionRobot robot[MAX_ROBOTS*CAM_COUNT];

    friend class CVisionModule;
};
typedef Singleton<CImmortalsVision> ImmortalsVision;
#endif // IMMORTALSVISION_H
