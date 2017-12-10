#include "DefPos2015.h"

#include "BallSpeedModel.h"
#include "BestPlayer.h"
#include "ChipBallJudge.h"
#include "DefendUtils.h"
#include "GDebugEngine.h"
#include "TaskMediator.h"
#include "WorldModel.h"

#include "param.h"
#include "utils.h"

#include <cmath>

using namespace DefendUtils;
using namespace Param::AvoidDist;
using namespace Param::Field;
using namespace Param::Math;
using namespace Param::Vehicle::V2;

namespace {

const double GoalBuffer = 2;
const double PLAYERSIZE = PLAYER_SIZE - 1.0; 
const double RExtremeLeftDir = -PI / 2 - atan(1.0);
const double RExtremeRightDir = PI / 2 + atan(1.0);
const double PENALTYBUFFER = 12;

double RgudgeRightDir;
double RgudgeLeftDir;
double gudgeBuffer = PI * 5 / 180;

CGeoPoint RLEFTGPOINT;
CGeoPoint RRIGHTGPOINT;
CGeoPoint RGOAL_LEFT_POS;
CGeoPoint RGOAL_RIGHT_POS;
CGeoPoint RGOAL_CENTRE_POS;

CGeoPoint RleftGudgePoint;
CGeoPoint RrightGudgePoint;

};

CDefPos2015::CDefPos2015() {    
    _defendPoints.leftD    = CGeoPoint(-(PITCH_LENGTH/2 - PENALTY_AREA_DEPTH - MAX_PLAYER_SIZE), -TEAMMATE_AVOID_DIST);
    _defendPoints.rightD   = CGeoPoint(-(PITCH_LENGTH/2 - PENALTY_AREA_DEPTH - MAX_PLAYER_SIZE), TEAMMATE_AVOID_DIST);
    _defendPoints.middleD  = CGeoPoint(0,0);
    _defendPoints.singleD  = CGeoPoint(-(PITCH_LENGTH/2 - PENALTY_AREA_DEPTH - MAX_PLAYER_SIZE), 0);
    _defendPoints.goalieD  = CGeoPoint(-(PITCH_LENGTH/2 - PLAYER_SIZE), 0);
    _laststoredgoaliePoint = CGeoPoint(-(PITCH_LENGTH/2 - PLAYER_SIZE), 0);
    _RdefendTarget         = CGeoPoint(0, 0);
    _RleftmostDir          = 0;
    _RrightmostDir         = 0;
    _RgoalieLeftDir        = 0;
    _RgoalieRightDir       = 0;
    _RleftgoalDir          = 0;
    _RrightgoalDir         = 0;
    _RmiddlegoalDir        = 0;
    _RdefendDir            = 0;

    RLEFTGPOINT            = CGeoPoint(PITCH_LENGTH / 2 - PLAYER_SIZE - 2, GOAL_WIDTH / 2 + 2);
    RRIGHTGPOINT           = CGeoPoint(PITCH_LENGTH / 2 - PLAYER_SIZE - 2, -GOAL_WIDTH / 2 - 2);
    RGOAL_LEFT_POS         = CGeoPoint(PITCH_LENGTH / 2, GOAL_WIDTH / 2 + GoalBuffer);
    RGOAL_RIGHT_POS        = CGeoPoint(PITCH_LENGTH / 2, -GOAL_WIDTH / 2 - GoalBuffer);
    RGOAL_CENTRE_POS       = CGeoPoint(PITCH_LENGTH / 2,0);

    // TODO 这里只是暂时这么改，x坐标应该是禁区线处，y坐标要看完再算
    double tempY           = PITCH_WIDTH/2 - MAX_PLAYER_SIZE*2;
    double tempX           = PITCH_LENGTH/2 - ((MAX_PLAYER_SIZE*2 + PLAYER_SIZE) * tempY/(PENALTY_AREA_WIDTH/2));
    RleftGudgePoint        = CGeoPoint(tempX, tempY);
    RrightGudgePoint       = CGeoPoint(tempX, -tempY);
    RgudgeLeftDir          = PI/2 + atan((PITCH_LENGTH/2 - tempX)/tempY);
    RgudgeRightDir         = -PI/2 - atan((PITCH_LENGTH/2 - tempX)/tempY);

    _lastCycle             = 0;
}

void CDefPos2015::generatePos(const CVisionModule* pVision) {
    CGeoPoint RleftPoint;
    CGeoPoint RrightPoint;
    CGeoPoint RmiddlePoint;
    CGeoPoint RsinglePoint;
    CGeoPoint RgoaliePoint;
    CGeoPoint RsidePoint;

    static bool goalieFirst = false;
    static bool leftFirst = false;
    static bool defendSide = false;
    static bool defendSide2 = false;
    static posSide RtargetSide = posSide::POS_SIDE_MIDDLE;

    const CGeoLine RtargetLine = getDefenceTargetAndLine(_RdefendTarget,_RdefendDir);
    const CVector RleftgoalVector = RGOAL_LEFT_POS - _RdefendTarget;
    _RleftgoalDir = RleftgoalVector.dir();
    const CVector RrightgoalVector = RGOAL_RIGHT_POS - _RdefendTarget;
    _RrightgoalDir = RrightgoalVector.dir();
    const CVector RmiddlegoalVector = RGOAL_CENTRE_POS - _RdefendTarget;
    _RmiddlegoalDir = RmiddlegoalVector.dir();
    double RgudgeDir = Utils::Normalize(_RmiddlegoalDir + PI);

    CGeoPoint RballPos = reversePoint(pVision->Ball().Pos());
    double RBallPosX = (-1)*pVision->Ball().Pos().x();
    double RBallPosY = (-1)*pVision->Ball().Pos().y();
    const MobileVisionT& ball = pVision->Ball();

    if (posSide::POS_SIDE_MIDDLE == RtargetSide) {
        if (RgudgeDir < 0 && RgudgeDir > RgudgeRightDir + gudgeBuffer)
            RtargetSide = posSide::POS_SIDE_RIGHT;
        else if (RgudgeDir > 0 && RgudgeDir < RgudgeLeftDir - gudgeBuffer)
            RtargetSide = posSide::POS_SIDE_LEFT;
    } else if (posSide::POS_SIDE_RIGHT == RtargetSide) {
        if (RgudgeDir < RgudgeRightDir || RgudgeDir > 0)
            RtargetSide = posSide::POS_SIDE_MIDDLE;
    } else if (posSide::POS_SIDE_LEFT == RtargetSide) {
        if (RgudgeDir > RgudgeLeftDir || RgudgeDir < 0)
            RtargetSide = posSide::POS_SIDE_MIDDLE;
    }

    // 由于匹配要慢一帧，需要在单后卫时计算left和right，反之亦然
    int leftBack = TaskMediator::Instance()->leftBack();
    int rightBack = TaskMediator::Instance()->rightBack();
    int singleBack = TaskMediator::Instance()->singleBack();
    if (leftBack != 0 && rightBack != 0) {
        if (Utils::InOurPenaltyArea(pVision->Ball().RawPos(), Param::Vehicle::V2::PLAYER_SIZE/2)) {
            // left,right防破防的车
            if (RBallPosX > PITCH_LENGTH/2 - PENALTY_AREA_DEPTH/2 - PLAYER_SIZE) {
                switch (RtargetSide) {
                    case posSide::POS_SIDE_MIDDLE:
                        // 守门员封球门连线
                        RgoaliePoint = calcGoaliePointV3(_RdefendTarget,_RdefendDir ,posSide::POS_SIDE_MIDDLE,_laststoredgoaliePoint,0);
                        RleftPoint = calcDefenderPointV3(_RdefendTarget,_RdefendDir-0.1,posSide::POS_SIDE_LEFT,0);
                        RrightPoint = calcDefenderPointV3(_RdefendTarget,_RdefendDir+0.1,posSide::POS_SIDE_RIGHT,0);
                        RsinglePoint = calcDefenderPointV3(_RdefendTarget,_RdefendDir,posSide::POS_SIDE_MIDDLE,0);
                        break;
                    case posSide::POS_SIDE_LEFT:
                        RgoaliePoint = calcGoaliePointV3(_RdefendTarget,_RmiddlegoalDir ,posSide::POS_SIDE_LEFT,_laststoredgoaliePoint,0);
                        RleftPoint = calcDefenderPointV3(RleftGudgePoint,(RGOAL_CENTRE_POS-RleftGudgePoint).dir()+0.1,posSide::POS_SIDE_LEFT,0);
                        RrightPoint = calcDefenderPointV3(RleftGudgePoint,(RGOAL_CENTRE_POS-RleftGudgePoint).dir()-0.1,posSide::POS_SIDE_RIGHT,0);
                        RsinglePoint = calcDefenderPointV3(RleftGudgePoint,CVector(RGOAL_CENTRE_POS - RleftGudgePoint).dir(),posSide::POS_SIDE_LEFT,0);
                        break;
                    case posSide::POS_SIDE_RIGHT:
                        RgoaliePoint = calcGoaliePointV3(_RdefendTarget,_RmiddlegoalDir ,posSide::POS_SIDE_RIGHT,_laststoredgoaliePoint,0);
                        RleftPoint = calcDefenderPointV3(RrightGudgePoint,(RGOAL_CENTRE_POS-RrightGudgePoint).dir()+0.1,posSide::POS_SIDE_LEFT,0);
                        RrightPoint = calcDefenderPointV3(RrightGudgePoint,(RGOAL_CENTRE_POS-RrightGudgePoint).dir()-0.1,posSide::POS_SIDE_RIGHT,0);
                        RsinglePoint = calcDefenderPointV3(RrightGudgePoint,CVector(RGOAL_CENTRE_POS - RrightGudgePoint).dir(),posSide::POS_SIDE_RIGHT,0);
                        break;
                }
            } else {
                RgoaliePoint = calcGoaliePointV3(_RdefendTarget,_RmiddlegoalDir ,posSide::POS_SIDE_MIDDLE,_laststoredgoaliePoint,0);
                RleftPoint = calcDefenderPointV3(_RdefendTarget,_RdefendDir-0.05,posSide::POS_SIDE_LEFT,0);
                RrightPoint = calcDefenderPointV3(_RdefendTarget,_RdefendDir+0.05,posSide::POS_SIDE_RIGHT,0);
                RsinglePoint = calcDefenderPointV3(_RdefendTarget,_RdefendDir,posSide::POS_SIDE_MIDDLE,0);
            }
        } else {
            // 这里defendbasedir可能处理的略微有点问题
            double defendbasedir = 0;
            if (isBallShotToTheGoal() == true)
                defendbasedir = _RdefendDir;
            else
                defendbasedir = _RmiddlegoalDir;
            if (posSide::POS_SIDE_MIDDLE == RtargetSide) {
                RgoaliePoint = calcGoaliePointV3(_RdefendTarget,_RmiddlegoalDir ,posSide::POS_SIDE_MIDDLE,_laststoredgoaliePoint,0);
                double Rleftdir = defendbasedir + calcBlockAngle(_RdefendTarget, RgoaliePoint);
                double Rrightdir = defendbasedir - calcBlockAngle(_RdefendTarget, RgoaliePoint);
                RleftPoint = calcDefenderPointV3(_RdefendTarget, Rleftdir, posSide::POS_SIDE_LEFT, 0);
                RrightPoint = calcDefenderPointV3(_RdefendTarget, Rrightdir, posSide::POS_SIDE_RIGHT, 0);

                double ball2leftDir = (RleftPoint - RballPos).dir();
                double ball2rightDir = (RrightPoint - RballPos).dir();
                double preFrame = 0.15;  // 提前0.15秒截球，否则视为截不到球
                bool isAbleToAttach;
                if (RballPos.dist(RGOAL_CENTRE_POS) < preFrame * ball.Vel().mod())
                    isAbleToAttach = false;
                else
                    isAbleToAttach = true;
                if (isBallShotToTheGoal()
                    && ball.Vel().mod() > 50
                    && Utils::AngleBetween(_RdefendDir, ball2leftDir, ball2rightDir, 0)
                    && isAbleToAttach == true) {
                  RleftPoint = calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_LEFT, 0);
                  RrightPoint = calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_RIGHT, 0);
                }
                RsinglePoint = calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_MIDDLE, 0);
            } else if (posSide::POS_SIDE_LEFT == RtargetSide) {
                RgoaliePoint = calcGoaliePointV3(_RdefendTarget,_RmiddlegoalDir ,posSide::POS_SIDE_LEFT,_laststoredgoaliePoint,0);
                RleftPoint = calcDefenderPointV3(RleftGudgePoint,(RGOAL_CENTRE_POS-RleftGudgePoint).dir(),posSide::POS_SIDE_LEFT,0);
                RrightPoint = calcDefenderPointV3(RleftGudgePoint,(RGOAL_CENTRE_POS-RleftGudgePoint).dir(),posSide::POS_SIDE_RIGHT,0);
                RsinglePoint = calcDefenderPointV3(RleftGudgePoint,CVector(RGOAL_CENTRE_POS - RleftGudgePoint).dir(),posSide::POS_SIDE_RIGHT,0);
            } else if (posSide::POS_SIDE_RIGHT == RtargetSide) {
                RgoaliePoint = calcGoaliePointV3(_RdefendTarget,_RmiddlegoalDir ,posSide::POS_SIDE_RIGHT,_laststoredgoaliePoint,0);
                RleftPoint = calcDefenderPointV3(RrightGudgePoint,(RGOAL_CENTRE_POS-RrightGudgePoint).dir(),posSide::POS_SIDE_LEFT,0);
                RrightPoint = calcDefenderPointV3(RrightGudgePoint,(RGOAL_CENTRE_POS-RrightGudgePoint).dir(),posSide::POS_SIDE_RIGHT,0);
                RsinglePoint = calcDefenderPointV3(RrightGudgePoint,CVector(RGOAL_CENTRE_POS - RrightGudgePoint).dir(),posSide::POS_SIDE_LEFT,0);
            }
        }

        // 若左右后卫距离较远，守门员防主朝向
        if ((pVision->OurPlayer(leftBack).Valid() == false
          || pVision->OurPlayer(rightBack).Valid() == false
          || pVision->OurPlayer(leftBack).Pos().dist(reversePoint(RleftPoint)) > 25
          || pVision->OurPlayer(rightBack).Pos().dist(reversePoint(RrightPoint)) > 25)) {
            switch (RtargetSide) {
                case posSide::POS_SIDE_MIDDLE:
                    RgoaliePoint = calcGoaliePointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_MIDDLE, _laststoredgoaliePoint, 0);
                    break;
                case posSide::POS_SIDE_LEFT:
                    RgoaliePoint = calcGoaliePointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_LEFT, _laststoredgoaliePoint, 0);
                    break;
                case posSide::POS_SIDE_RIGHT:
                default:
                    RgoaliePoint = calcGoaliePointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_RIGHT, _laststoredgoaliePoint, 0);
                    break;
            }
        }

        // 如果球已经踢出，门将从防球门连线状态转向防球速线
        CVector RballVel = Utils::Polar2Vector(ball.Vel().mod(), Utils::Normalize(ball.Vel().dir() + PI));
        double RballVelDir = RballVel.dir(); 
        CVector Rball2LeftGoal = RGOAL_LEFT_POS - RballPos;
        CVector Rball2RightGoal = RGOAL_RIGHT_POS - RballPos;
        CVector Rball2CenterGoal = RGOAL_CENTRE_POS - RballPos;
        double Rball2LeftDir = Rball2LeftGoal.dir();
        double Rball2RightDir = Rball2RightGoal.dir();
        bool ballShooted = getBallShooted() && Utils::InBetween(RballVelDir, Rball2LeftDir, Rball2RightDir);
        if (ballShooted == true) {
            if (RBallPosX < 0)
                ballShooted = (ball.Vel().mod() > 250)? true : false;
            else if (RBallPosX < PITCH_LENGTH/4)
                ballShooted = (ball.Vel().mod() > 150)? true : false;
            else
                ballShooted = (ball.Vel().mod() > 100)? true : false;
        }

        if (ballShooted == true
          && RtargetSide == posSide::POS_SIDE_MIDDLE
          && Utils::AngleBetween(_RdefendDir, Rball2LeftDir, Rball2RightDir, 0) == true) {
            RgoaliePoint = calcGoaliePointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_MIDDLE, _laststoredgoaliePoint,0);
            //GDebugEngine::Instance()->gui_debug_msg(reversePoint(RgoaliePoint), "GGGGGGG", COLOR_WHITE);
        }
    } else if (singleBack != 0) {
        if (posSide::POS_SIDE_RIGHT == RtargetSide) {
            RsinglePoint = calcDefenderPointV3(RrightGudgePoint,CVector(RGOAL_CENTRE_POS - RrightGudgePoint).dir(),posSide::POS_SIDE_RIGHT,0);
            RgoaliePoint = calcGoaliePointV3(_RdefendTarget,_RdefendDir,posSide::POS_SIDE_RIGHT,_laststoredgoaliePoint,0);
            RleftPoint = calcDefenderPointV3(RrightGudgePoint,(RGOAL_CENTRE_POS-RrightGudgePoint).dir(),posSide::POS_SIDE_LEFT,0);
            RrightPoint = calcDefenderPointV3(RrightGudgePoint,(RGOAL_CENTRE_POS-RrightGudgePoint).dir(),posSide::POS_SIDE_RIGHT,0);
        } else if (posSide::POS_SIDE_LEFT == RtargetSide) {
            RsinglePoint = calcDefenderPointV3(RleftGudgePoint,CVector(RGOAL_CENTRE_POS - RleftGudgePoint).dir(),posSide::POS_SIDE_LEFT,0);
            RgoaliePoint = calcGoaliePointV3(_RdefendTarget,_RdefendDir,posSide::POS_SIDE_LEFT,_laststoredgoaliePoint,0);
            RleftPoint = calcDefenderPointV3(RleftGudgePoint,(RGOAL_CENTRE_POS-RleftGudgePoint).dir(),posSide::POS_SIDE_LEFT,0);
            RrightPoint = calcDefenderPointV3(RleftGudgePoint,(RGOAL_CENTRE_POS-RleftGudgePoint).dir(),posSide::POS_SIDE_RIGHT,0);
        } else {
            RsinglePoint = calcDefenderPointV3(_RdefendTarget,_RdefendDir,posSide::POS_SIDE_MIDDLE,0);
            RgoaliePoint = calcGoaliePointV3(_RdefendTarget,_RdefendDir,posSide::POS_SIDE_MIDDLE,_laststoredgoaliePoint,0);
            RleftPoint = calcDefenderPointV3(_RdefendTarget,_RmiddlegoalDir,posSide::POS_SIDE_LEFT,0);
            RrightPoint = calcDefenderPointV3(_RdefendTarget,_RmiddlegoalDir,posSide::POS_SIDE_RIGHT,0);

            // 判断守门员和后卫谁先到各自的点
            double goalieOKdist = pVision->OurPlayer(PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie())).Pos().dist(reversePoint(RgoaliePoint));
            double defOKdist = pVision->OurPlayer(TaskMediator::Instance()->singleBack()).Pos().dist(reversePoint(RsinglePoint));
            if (defOKdist > goalieOKdist + 2)
                goalieFirst = true;
            else if (goalieOKdist > defOKdist + 5)
                goalieFirst = false;
            if (goalieFirst == true) {
                // 门将封主朝向
                double Rtarget2goalieDir = CVector(RgoaliePoint - _RdefendTarget).dir();
                _RgoalieLeftDir = Rtarget2goalieDir + calcBlockAngle(_RdefendTarget, RgoaliePoint);
                _RgoalieRightDir = Rtarget2goalieDir - calcBlockAngle(_RdefendTarget, RgoaliePoint);
                double leftRange = _RleftgoalDir - _RgoalieLeftDir;
                double rightRange = _RgoalieRightDir - _RrightgoalDir;                
                if (leftRange > rightRange + 0.01)
                    defendSide = true;
                else if (rightRange > leftRange + 0.01)
                    defendSide = false;
                // 辅助防守次级判断
                static posSide RExtremeSide = posSide::POS_SIDE_MIDDLE;
                if (posSide::POS_SIDE_MIDDLE == RExtremeSide){
                    if (RgudgeDir < 0 && RgudgeDir > RExtremeLeftDir + gudgeBuffer){
                        RExtremeSide = posSide::POS_SIDE_RIGHT;
                    } else if (RgudgeDir > 0 && RgudgeDir < RExtremeRightDir - gudgeBuffer){
                        RExtremeSide = posSide::POS_SIDE_LEFT;
                    }
                } else if (posSide::POS_SIDE_RIGHT == RExtremeSide){
                    if (RgudgeDir < RExtremeLeftDir || RgudgeDir > 0){
                        RExtremeSide = posSide::POS_SIDE_MIDDLE;
                    }
                } else if (posSide::POS_SIDE_LEFT == RExtremeSide){
                    if (RgudgeDir > RExtremeRightDir || RgudgeDir < 0){
                        RExtremeSide = posSide::POS_SIDE_MIDDLE;
                    }
                }
                if (posSide::POS_SIDE_RIGHT == RExtremeSide){
                    defendSide = true;
                } else if (posSide::POS_SIDE_LEFT == RExtremeSide){
                    defendSide = false;
                }
                posSide defSide = defendSide ? posSide::POS_SIDE_RIGHT : posSide::POS_SIDE_LEFT;
                double anotherDir = defendSide ? _RgoalieLeftDir : _RgoalieRightDir;
                //后卫站位点，当球速过快，不考虑封最大朝向，对球速线进行偏置
                RsinglePoint = calcDefenderPointV3(_RdefendTarget,anotherDir,defSide,0);
                bool ballShooted = getBallShooted();
                if (ballShooted){
                    RsinglePoint = calcDefenderPointV3(_RdefendTarget,_RdefendDir,defSide,0);
                }
                bool enemyPass = getEnemyPass();
                if (enemyPass){
                    if (pVision->Ball().VelY() > 100){
                        RsinglePoint = calcDefenderPointV3(_RdefendTarget,_RdefendDir,posSide::POS_SIDE_RIGHT,0);
                    }else if (pVision->Ball().VelY()<-100){
                        RsinglePoint = calcDefenderPointV3(_RdefendTarget,_RdefendDir,posSide::POS_SIDE_LEFT,0);
                    }
                }
            } else {
                double Rtarget2defDir = CVector(RsinglePoint- _RdefendTarget).dir();
                double _RdefLeftDir = Rtarget2defDir + calcBlockAngle(_RdefendTarget,RsinglePoint);
                double _RdefRightDir = Rtarget2defDir - calcBlockAngle(_RdefendTarget,RsinglePoint);
                double leftRange = _RleftgoalDir -_RdefLeftDir ;
                double rightRange = _RdefRightDir-_RrightgoalDir ;
                if (leftRange > rightRange + 0.01){
                    defendSide2 = true;
                } else if (rightRange > leftRange + 0.01){
                    defendSide2 = false;
                }
                posSide defSide = defendSide2 ? posSide::POS_SIDE_LEFT : posSide::POS_SIDE_RIGHT;
                double goalieDir = defendSide2 ? _RdefLeftDir:_RdefRightDir;
                int sideFactor;
                if (posSide::POS_SIDE_LEFT == defSide){
                    sideFactor = 1;
                } else{
                    sideFactor = -1;
                }
                if (RBallPosX > Param::Field::PITCH_LENGTH/4){
                    goalieDir = Rtarget2defDir + sideFactor*0.1;
                }
                CVector transVector = Utils::Polar2Vector(PLAYERSIZE,Utils::Normalize(goalieDir + sideFactor*PI / 2));
                CGeoPoint transPoint = _RdefendTarget + transVector;
                RgoaliePoint = calcGoaliePointV3(transPoint, goalieDir, defSide, _laststoredgoaliePoint, 1);
            }
        }
    } else if (leftBack == 0 && rightBack == 0 && singleBack == 0) {
        //单守门员
        switch (RtargetSide) {
            case posSide::POS_SIDE_MIDDLE:
                RgoaliePoint = calcGoaliePointV3(_RdefendTarget,_RdefendDir,posSide::POS_SIDE_MIDDLE, _laststoredgoaliePoint,0);
                break;
            case posSide::POS_SIDE_LEFT:
                RgoaliePoint = calcGoaliePointV3(_RdefendTarget,_RdefendDir,posSide::POS_SIDE_LEFT, _laststoredgoaliePoint,0);
                break;
            case posSide::POS_SIDE_RIGHT:
                RgoaliePoint = calcGoaliePointV3(_RdefendTarget,_RdefendDir,posSide::POS_SIDE_RIGHT, _laststoredgoaliePoint,0);
                break;
        }
        RleftPoint = calcDefenderPointV3(_RdefendTarget,_RmiddlegoalDir,posSide::POS_SIDE_LEFT,0);
        RrightPoint = calcDefenderPointV3(_RdefendTarget,_RmiddlegoalDir,posSide::POS_SIDE_RIGHT,0);
        RsinglePoint = calcDefenderPointV3(_RdefendTarget,_RdefendDir,posSide::POS_SIDE_MIDDLE,0);
    } else {
        //其他情况如调用leftback和goalie，车看不见等
        switch (RtargetSide) {
            case posSide::POS_SIDE_LEFT: {
                RsinglePoint = DefendUtils::calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_LEFT, 0);
                RgoaliePoint = DefendUtils::calcGoaliePointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_LEFT, _laststoredgoaliePoint, 0);
                if (leftBack != 0) {
                    RleftPoint  = DefendUtils::calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_RIGHT, 0);
                    RrightPoint = DefendUtils::calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_LEFT, 0);
                } else {
                    RrightPoint = DefendUtils::calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_RIGHT, 0);
                    RleftPoint  = DefendUtils::calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_LEFT, 0);
                }
            } break;
            case posSide::POS_SIDE_RIGHT: {
                RsinglePoint = DefendUtils::calcDefenderPointV3(_RdefendTarget,_RdefendDir,posSide::POS_SIDE_RIGHT,0);
                RgoaliePoint = DefendUtils::calcGoaliePointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_RIGHT, _laststoredgoaliePoint, 0);
                if (leftBack != 0) {
                    RleftPoint  = DefendUtils::calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_LEFT, 0);
                    RrightPoint = DefendUtils::calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_RIGHT, 0);
                } else {
                    RrightPoint = DefendUtils::calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_LEFT, 0);
                    RleftPoint  = DefendUtils::calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_RIGHT, 0);
                }
            } break;
            case posSide::POS_SIDE_MIDDLE: {
                RsinglePoint = DefendUtils::calcDefenderPointV3(_RdefendTarget,_RdefendDir,posSide::POS_SIDE_MIDDLE,0);
                RgoaliePoint = DefendUtils::calcGoaliePointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_MIDDLE, _laststoredgoaliePoint, 0);
                if (_RdefendDir >= 0) {
                    if (leftBack != 0) {
                        RleftPoint  = DefendUtils::calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_LEFT, 0);
                        RrightPoint = DefendUtils::calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_RIGHT, 0);
                    } else {
                        RrightPoint = DefendUtils::calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_LEFT, 0);
                        RleftPoint  = DefendUtils::calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_RIGHT, 0);
                    }
                } else {
                    if (leftBack != 0) {
                        RleftPoint  = DefendUtils::calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_RIGHT, 0);
                        RrightPoint = DefendUtils::calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_LEFT, 0);
                    } else {
                        RrightPoint = DefendUtils::calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_RIGHT, 0);
                        RleftPoint  = DefendUtils::calcDefenderPointV3(_RdefendTarget, _RdefendDir, posSide::POS_SIDE_LEFT, 0);
                    }
                }
            } break;
        }
    }

    // 当后卫站位点附近有我方车且对方车距离较远时强行将车移开一车身
    int oppnum = BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num; 
    if (TaskMediator::Instance()->leftBack() != 0
              && TaskMediator::Instance()->rightBack() != 0
              && pVision->TheirPlayer(oppnum).Pos().dist(reversePoint(RleftPoint))> 100
              && pVision->TheirPlayer(oppnum).Pos().dist(reversePoint(RrightPoint))> 100) {
          double dist1 = checkCollision(TaskMediator::Instance()->leftBack(),reversePoint(RleftPoint),pVision);
          double dist2 = checkCollision(TaskMediator::Instance()->rightBack(),reversePoint(RrightPoint),pVision);
        if (dist1 > 0 && dist2 > 0){
            if (dist1 < dist2) {
                CVector transVector = Utils::Polar2Vector(PLAYERSIZE*3,Utils::Normalize(_RdefendDir + (-1 )* PI / 2));
                CGeoPoint transPoint = _RdefendTarget + transVector;
                //GDebugEngine::Instance()->gui_debug_line(reversePoint(transPoint),reversePoint(transPoint+Utils::Polar2Vector(200,_RdefendDir)),COLOR_WHITE);
                RleftPoint = calcDefenderPointV3(transPoint,_RdefendDir,posSide::POS_SIDE_LEFT, 0);
                RrightPoint = calcDefenderPointV3(transPoint,_RdefendDir,posSide::POS_SIDE_RIGHT, 0);
            } else {
                CVector transVector = Utils::Polar2Vector(PLAYERSIZE*3,Utils::Normalize(_RdefendDir + PI / 2));
                CGeoPoint transPoint = _RdefendTarget + transVector;
                //GDebugEngine::Instance()->gui_debug_line(reversePoint(transPoint),reversePoint(transPoint+Utils::Polar2Vector(200,_RdefendDir)),COLOR_WHITE);
                RleftPoint = calcDefenderPointV3(transPoint,_RdefendDir,posSide::POS_SIDE_LEFT, 0);
                RrightPoint = calcDefenderPointV3(transPoint,_RdefendDir,posSide::POS_SIDE_RIGHT, 0);
            }
        } else if (dist1 > 0 && dist2 == -1) {
            CVector transVector = Utils::Polar2Vector(PLAYERSIZE*3,Utils::Normalize(_RdefendDir + (-1 )* PI / 2));
            CGeoPoint transPoint = _RdefendTarget + transVector;
            //GDebugEngine::Instance()->gui_debug_line(reversePoint(transPoint),reversePoint(transPoint+Utils::Polar2Vector(200,_RdefendDir)),COLOR_WHITE);
            RleftPoint = calcDefenderPointV3(transPoint,_RdefendDir,posSide::POS_SIDE_LEFT, 0);
            RrightPoint = calcDefenderPointV3(transPoint,_RdefendDir,posSide::POS_SIDE_RIGHT, 0);
        } else if (dist1 == -1 && dist2 > 0) {
            CVector transVector = Utils::Polar2Vector(PLAYERSIZE*3,Utils::Normalize(_RdefendDir + PI / 2));
            CGeoPoint transPoint = _RdefendTarget + transVector;
            //GDebugEngine::Instance()->gui_debug_line(reversePoint(transPoint),reversePoint(transPoint+Utils::Polar2Vector(200,_RdefendDir)),COLOR_WHITE);
            RleftPoint = calcDefenderPointV3(transPoint,_RdefendDir,posSide::POS_SIDE_LEFT, 0);
            RrightPoint = calcDefenderPointV3(transPoint,_RdefendDir,posSide::POS_SIDE_RIGHT, 0);
        }
    }

    const CGeoPoint LEFT_POST(-PITCH_LENGTH/2, -PITCH_WIDTH/2);
    const CGeoPoint RIGHT_POST(-PITCH_LENGTH/2, PITCH_WIDTH/2);
    if (ball.Vel().dir() < (LEFT_POST - ball.Pos()).dir() || ball.Vel().dir() > (RIGHT_POST - ball.Pos()).dir()) {
      if (TaskMediator::Instance()->leftBack() != 0 && pVision->OurPlayer(TaskMediator::Instance()->leftBack()).X() > -300
          || TaskMediator::Instance()->rightBack() != 0 && pVision->OurPlayer(TaskMediator::Instance()->rightBack()).X() > -300) {
        const CGeoPoint& leftBackPos = pVision->OurPlayer(TaskMediator::Instance()->leftBack()).Pos();
        const CGeoPoint& rightBackPos = pVision->OurPlayer(TaskMediator::Instance()->rightBack()).Pos();
        const CGeoPoint& defendTarget = reversePoint(_RdefendTarget);
        const CGeoPoint* realLeftBackPos = &leftBackPos;
        const CGeoPoint* realRightBackPos = &rightBackPos;
        if (realLeftBackPos->y() > realRightBackPos->y()) {
          realLeftBackPos = &rightBackPos;
          realRightBackPos = &leftBackPos;
        }
        const double ballVelDir = ball.Vel().dir();
        const double targetToLeftBackDir = (*realLeftBackPos - defendTarget).dir();
        const double targetToRightBackDir = (*realRightBackPos - defendTarget).dir();
        const double diffDirLeft = Utils::Normalize(targetToLeftBackDir - ballVelDir);
        const double diffDirRight = Utils::Normalize(targetToRightBackDir - ballVelDir);
        if (diffDirLeft > PI / 2 && diffDirLeft <= PI) {  // 左后卫在球的左前方
          CVector transVector = Utils::Polar2Vector(50, Utils::Normalize(_RdefendDir + PI / 2));
          CGeoPoint transPoint = _RdefendTarget + transVector;
          RleftPoint = calcDefenderPointV3(transPoint, _RdefendDir, posSide::POS_SIDE_LEFT, 0);
        } else if (diffDirLeft < -PI / 2 && diffDirLeft >= -PI) {  // 左后卫在球的右前方 交叉
          CVector transVector = Utils::Polar2Vector(70, Utils::Normalize(_RdefendDir - PI / 2));
          CGeoPoint transPoint = _RdefendTarget + transVector;
          RleftPoint = calcDefenderPointV3(transPoint, _RdefendDir, posSide::POS_SIDE_RIGHT, 0);
        }
        if (diffDirRight > PI / 2 && diffDirRight <= PI) {  // 右后卫在球的左前方 交叉
          CVector transVector = Utils::Polar2Vector(70, Utils::Normalize(_RdefendDir + PI / 2));
          CGeoPoint transPoint = _RdefendTarget + transVector;
          RrightPoint = calcDefenderPointV3(transPoint, _RdefendDir, posSide::POS_SIDE_LEFT, 0);
        } else if (diffDirRight < -PI / 2 && diffDirRight >= -PI) {  // 右后卫在球的右前方
          CVector transVector = Utils::Polar2Vector(50, Utils::Normalize(_RdefendDir - PI / 2));
          CGeoPoint transPoint = _RdefendTarget + transVector;
          RrightPoint = calcDefenderPointV3(transPoint, _RdefendDir, posSide::POS_SIDE_RIGHT, 0);
        }
      }
    }

    // 当后卫站位点后方有对方头球车时强行挤进禁区
    const CGeoPoint OUR_GOAL_CENTER(-PITCH_LENGTH/2, 0);
    const double leftBlockDir = calcBlockAngle(reversePoint(RleftPoint), OUR_GOAL_CENTER);
    const double rightBlockDir = calcBlockAngle(reversePoint(RrightPoint), OUR_GOAL_CENTER);
    const double leftToGoalDist = RleftPoint.dist(RGOAL_CENTRE_POS);
    const double rightToGoalDist = RrightPoint.dist(RGOAL_CENTRE_POS);
    for (int i = 1; i <= MAX_PLAYER; ++i) {
        const PlayerVisionT& enemy = pVision->TheirPlayer(i);
        const CGeoPoint& enemyPos = enemy.Pos();
        double enemyToGoalDist = enemyPos.dist(OUR_GOAL_CENTER);
        double goalToLeftDir = (reversePoint(RleftPoint) - OUR_GOAL_CENTER).dir();
        double goalToRightDir = (reversePoint(RrightPoint) - OUR_GOAL_CENTER).dir();
        double goalToEnemyDir = (enemyPos - OUR_GOAL_CENTER).dir();
        if (goalToEnemyDir > goalToLeftDir - leftBlockDir*1.5 && goalToEnemyDir < goalToRightDir + rightBlockDir*1.5
                || goalToEnemyDir > goalToRightDir - rightBlockDir*1.5 && goalToEnemyDir < goalToLeftDir + leftBlockDir*1.5) {
            /* 对方车在我方后卫后面 */
            if (Utils::InOurPenaltyArea(enemyPos, 0) == false) {
                if (enemyToGoalDist < leftToGoalDist && enemyToGoalDist < rightToGoalDist) {
                    RleftPoint = reversePoint(enemyPos);
                    RrightPoint = reversePoint(enemyPos);
                    RsinglePoint = reversePoint(enemyPos);
                    break;
                }
            }
        }
    }

    // 计算中后卫
    switch (RtargetSide) {
        case posSide::POS_SIDE_MIDDLE:
            RmiddlePoint = calcDefenderPointV3(_RdefendTarget,_RdefendDir,posSide::POS_SIDE_MIDDLE, 3);
            break;
        case posSide::POS_SIDE_LEFT:
            RmiddlePoint = calcDefenderPointV3(RleftGudgePoint,(RGOAL_CENTRE_POS - RleftGudgePoint).dir(),posSide::POS_SIDE_MIDDLE, 3);
            break;
        case posSide::POS_SIDE_RIGHT:
            RmiddlePoint = calcDefenderPointV3(RrightGudgePoint,(RGOAL_CENTRE_POS - RrightGudgePoint).dir(),posSide::POS_SIDE_MIDDLE, 3);
            break;
    }
	
    // 计算边后卫
    double RSideDir;
    CGeoPoint RSideTarget;
    const CGeoLine line = getSideBackTargetAndLine(RSideTarget, RSideDir);
    RsidePoint = calcDefenderPointV3(RSideTarget, RSideDir, POS_SIDE_MIDDLE, 1);

    _defendPoints.sideD = reversePoint(RsidePoint);
    _defendPoints.leftD = reversePoint(RleftPoint);
    _defendPoints.rightD = reversePoint(RrightPoint);
    _defendPoints.middleD = reversePoint(RmiddlePoint);
    _defendPoints.singleD = reversePoint(RsinglePoint);
	_defendPoints.goalieD = reversePoint(RgoaliePoint);
	_laststoredgoaliePoint = _defendPoints.goalieD;
    

    // Debug信息
    if (false) {
        GDebugEngine::Instance()->gui_debug_arc(reversePoint(RleftGudgePoint), 2, 0, 360, COLOR_WHITE);
        GDebugEngine::Instance()->gui_debug_arc(reversePoint(RrightGudgePoint), 2, 0, 360, COLOR_WHITE);
        GDebugEngine::Instance()->gui_debug_arc(reversePoint(_RdefendTarget), 2, 0, 360, COLOR_WHITE);

        GDebugEngine::Instance()->gui_debug_line(reversePoint(RGOAL_CENTRE_POS), reversePoint(_RdefendTarget), COLOR_WHITE);
        GDebugEngine::Instance()->gui_debug_line(reversePoint(RGOAL_LEFT_POS), reversePoint(_RdefendTarget), COLOR_WHITE);
        GDebugEngine::Instance()->gui_debug_line(reversePoint(RGOAL_RIGHT_POS), reversePoint(_RdefendTarget), COLOR_WHITE);

        GDebugEngine::Instance()->gui_debug_line(reversePoint(RGOAL_CENTRE_POS),
                                                reversePoint(RGOAL_CENTRE_POS + Utils::Polar2Vector(1000, RgudgeRightDir)),
                                                COLOR_WHITE);
        GDebugEngine::Instance()->gui_debug_line(reversePoint(RGOAL_CENTRE_POS),
                                                reversePoint(RGOAL_CENTRE_POS + Utils::Polar2Vector(1000, RgudgeLeftDir)),
                                                COLOR_WHITE);

        GDebugEngine::Instance()->gui_debug_line(reversePoint(RGOAL_CENTRE_POS),
                                                reversePoint(RGOAL_CENTRE_POS + Utils::Polar2Vector(1000, RExtremeLeftDir)),
                                                COLOR_RED);
        GDebugEngine::Instance()->gui_debug_line(reversePoint(RGOAL_CENTRE_POS),
                                                reversePoint(RGOAL_CENTRE_POS + Utils::Polar2Vector(1000, RExtremeRightDir)),
                                                COLOR_RED);
    }
}

Defend2015 CDefPos2015::getDefPos2015(const CVisionModule* pVision)
{
    if (pVision->Cycle() == _lastCycle) {
        return _defendPoints;
    } else {
        _lastCycle = pVision->Cycle();
    }
    generatePos(pVision);
    return _defendPoints;
}

double CDefPos2015::checkCollision(int myself, CGeoPoint targetPoint, const CVisionModule* pVision)
{
    bool result = false;
    double dist = 100;
    for (int i = 1; i<= MAX_PLAYER; i++){
        if (i!= myself
          && i!=TaskMediator::Instance()->defendMiddle() 
          && i!= TaskMediator::Instance()->leftBack()
          && i!= TaskMediator::Instance()->rightBack()
          && i!=TaskMediator::Instance()->sideBack()
          && vision->OurPlayer(i).Pos().dist(targetPoint)< 20){
            result = true;
            //cout<<"i is "<<i<<endl;
            double temp = vision->OurPlayer(i).Pos().dist(targetPoint);
            if (temp<dist ){
                dist = temp;
            }
        }
    }
    if (result == true){
        return dist;
    }else{
        return -1;
    }
}
