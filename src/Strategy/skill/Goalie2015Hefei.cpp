#include "BallSpeedModel.h"
#include "DefendUtils.h"
#include "DefPos2015.h"
#include "DribbleStatus.h"
#include "GDebugEngine.h"
#include "Goalie2015Hefei.h"
#include "KickStatus.h"
#include "PlayInterface.h"
#include "RobotSensor.h"
#include "TaskMediator.h"
#include "VisionModule.h"
#include "WorldModel.h"
#include "param.h"
#include "skill/Factory.h"
#include "utils.h"
#include <atlstr.h>
#include "tinyxml/ParamReader.h"

#include <cmath>

using namespace Param;
using namespace Utils;
using namespace std;

namespace {
  const bool DEBUG_INFO_FLAG = false;

  const double DANGER_FACTOR         = 2.5;
  const double PENALTY_BUFFER        = 15.0;
  const double DEFEND_PENALTY_BUFFER = 25.0;

  const CGeoPoint LEFT_GOAL_POST          = CGeoPoint(-Field::PITCH_LENGTH/2, -Field::GOAL_WIDTH/2);
  const CGeoPoint RIGHT_GOAL_POST         = CGeoPoint(-Field::PITCH_LENGTH/2, Field::GOAL_WIDTH/2);
  const CGeoPoint GOAL_CENTER             = CGeoPoint(-Field::PITCH_LENGTH/2, 0);
  const CGeoPoint LEFT_GOAL_CENTER_FRONT  = CGeoPoint(-Field::PITCH_LENGTH/2 + 50, -10);
  const CGeoPoint RIGHT_GOAL_CENTER_FRONT = CGeoPoint(-Field::PITCH_LENGTH/2 + 50, 10);
  const CGeoPoint LEFT_CORNER             = CGeoPoint(-Field::PITCH_LENGTH/2, -Field::PITCH_WIDTH/2);
  const CGeoPoint RIGHT_CORNER            = CGeoPoint(-Field::PITCH_LENGTH/2, Field::PITCH_WIDTH/2);
  const CGeoPoint LEFT_PENALTY_END        = CGeoPoint(-Field::PITCH_LENGTH/2, -Field::PENALTY_AREA_WIDTH/2);
  const CGeoPoint RIGHT_PENALTY_END       = CGeoPoint(-Field::PITCH_LENGTH/2, Field::PENALTY_AREA_WIDTH/2);

  int GOALIE_FILL_IN_MODE;
  int GOALIE_DEFEND_ONE_PASS_SHOOT;
}

CGoalie2015Hefei::CGoalie2015Hefei() : lastGoaliePoint(-Field::PITCH_LENGTH/2 + Vehicle::V2::PLAYER_SIZE, 0) {
  clearBall = false;
  fillIn = NONE;

  /* ����Ա��ǰ��λģʽ 0Ϊ����ǰ */
  DECLARE_PARAM_READER_BEGIN(Defence)
    READ_PARAM(GOALIE_FILL_IN_MODE)
	READ_PARAM(GOALIE_DEFEND_ONE_PASS_SHOOT)
  DECLARE_PARAM_READER_END
}

void CGoalie2015Hefei::plan(const CVisionModule* pVision) {
  int goalieNum = task().executor;
  const PlayerVisionT& enemy = pVision->TheirPlayer(BestPlayer::Instance()->getTheirBestPlayer());
  const PlayerVisionT& me    = pVision->OurPlayer(goalieNum);
  const MobileVisionT& ball  = pVision->Ball();
  const CGeoPoint& normalGoaliePoint = DefPos2015::Instance()->getDefPos2015(pVision).getGoaliePos();
  TaskT myTask(task());
  myTask.player.pos = normalGoaliePoint;
  myTask.player.flag |= PlayerStatus::QUICKLY;
  if (ball.Valid())
    myTask.player.angle = (ball.Pos() - me.Pos()).dir();
  else
    myTask.player.angle = (me.Pos() - GOAL_CENTER).dir();

  // add 2017/7/18 TODO
  if (GOALIE_DEFEND_ONE_PASS_SHOOT) {
	  auto ballVel = ball.Vel().mod();
	  auto ballDir = ball.Vel().dir();
	  auto rawBallPos = ball.RawPos();
	  CGeoLine ballLine(ball.Pos(),ball.Vel().mod());
	  CGeoSegment ballMovingSeg = CGeoSegment(rawBallPos, rawBallPos + Utils::Polar2Vector(800, Utils::Normalize(ballDir)));
	  CGeoPoint projEnemy = ballMovingSeg.projection(enemy.Pos());
	  bool enemyOnBallMovingSeg = ballMovingSeg.IsPointOnLineOnSegment(projEnemy);
	  auto enemyMoveDist = (projEnemy - enemy.Pos()).mod();
	  CGeoPoint defendPos = myTask.player.pos;
	  auto enemy2Ball = ball.Pos() - enemy.Pos();
	  auto dirEnemy2Ball = enemy2Ball.dir();
	  auto enemyMoveAngel = fabs(dirEnemy2Ball + ballDir);
	  if ((InOurPenaltyArea(enemy.Pos(),300) && // ���ҷ�����ǰ�� TODO ��Ҫ��ӵз�������ж�
		  ballVel > 50 && (!DefendUtils::isBallShotToTheGoal()) && // �򲻳����Ź���
		  (enemyMoveDist < 40 || enemyMoveAngel < Param::Math::PI/180.0* 5) &&
		   enemyOnBallMovingSeg) // ����ǰ������׼������
		  || (InOurPenaltyArea(enemy.Pos(), 120) && ballVel <= 50 && enemy2Ball.mod() < 40) //���Ѿ��ڵз���ǰ
		  ) {
		  double leftPostToBallDir = (enemy.Pos() - LEFT_GOAL_POST).dir();
		  double rightPostToBallDir = (enemy.Pos() - RIGHT_GOAL_POST).dir();
		  double defendDir = Normalize((leftPostToBallDir + rightPostToBallDir) / 2 + Math::PI);
		  double ROriginX = -(Param::Field::PITCH_LENGTH / 2 - Param::Vehicle::V2::PLAYER_SIZE / 3);
		  double m1 = 100;
		  double n1 = 100;//Param::Field::GOAL_WIDTH / 2 + Param::Vehicle::V2::PLAYER_SIZE;
		  double m2 = 90;
		  double n2 = 100;
		  CGeoPoint defendPos1(-9999, -9999);
		  CGeoPoint defendPos2(-9999, -9999);
		  CGeoEllipse defendEllipse = CGeoEllipse(CGeoPoint(ROriginX, 0), m1, n1);
		  CGeoEllipse clearBallEllipse = CGeoEllipse(CGeoPoint(ROriginX, 0), m2, n2);
		  CGeoLine defenceLine = CGeoLine(enemy.Pos(), defendDir);
		  CGeoLineEllipseIntersection intersect(defenceLine, defendEllipse);
		  CGeoLineEllipseIntersection ballLineintersect(ballMovingSeg, clearBallEllipse);
		  if (ballLineintersect.intersectant() == true && intersect.intersectant() == true) {
			  CGeoLineLineIntersection linelineinter(ballMovingSeg, defenceLine);
			  defendPos1 = linelineinter.IntersectPoint();
		  }
		  if (intersect.intersectant() == true) {
			  if (intersect.point1().dist(enemy.Pos()) < intersect.point2().dist(enemy.Pos()))
				  defendPos2 = intersect.point1();
			  else
				  defendPos2 = intersect.point2();
		  }
		  else {
			  defendPos2 = defendPos;
		  }
		  if (defendPos.dist(defendPos1) > defendPos.dist(defendPos2))
			  myTask.player.pos = defendPos2;
		  else
			  myTask.player.pos = defendPos1;
	  }
	  if (DEBUG_INFO_FLAG) {
		  int enemyNum = BestPlayer::Instance()->getTheirBestPlayer();
		  CString msg;
		  msg.Format("%.0lf\t%.0lf",enemyMoveDist, enemyMoveAngel*180/ Param::Math::PI);
		  GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0, -200),msg,COLOR_GREEN);
		  GDebugEngine::Instance()->gui_debug_x(defendPos, COLOR_BLACK);
	  }
  } // GOALIE_DEFEND_ONE_PASS_SHOOT end

  CGoalie2015Hefei::FillInTarget fillInTarget = GOALIE_FILL_IN_MODE ? needToFillIn(pVision) : NONE;

  /**
   * OUTOFBOUND
   * ��������ʱ��ͼ��ɵ������bug
   */
  if (pVision->RawBall().Valid() == false && ball.Pos().x() < -Field::PITCH_LENGTH/2 + 2) {
    if (DEBUG_INFO_FLAG) {
      GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(200, 0), "in out of bound");
    }
    if (ball.Y() < LEFT_GOAL_POST.y() || ball.Y() > RIGHT_GOAL_POST.y()) {
      myTask.player.pos = normalGoaliePoint;
    } else {
      myTask.player.pos = me.Pos();
      myTask.player.angle = 0;
    }
    setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
  }

  /**
   * CLEAR BALL
   * ����״̬ ��һ֡��������״̬������һ֡��������״̬
   */
  else if (needToClearBall(pVision) || clearBall == true) {
    fillIn = NONE;
    clearBall = true;
    if (DEBUG_INFO_FLAG)
      GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(200, 0), "in clear ball");

    /* ��������Ƕ� */
    double clearBallDir = (ball.Pos() - GOAL_CENTER).dir();
    if (TaskMediator::Instance()->singleBack() == 0 && TaskMediator::Instance()->leftBack() != 0) {
      CGeoPoint leftpos = DefPos2015::Instance()->getDefPos2015(pVision).getLeftPos();
      CGeoPoint rightpos = DefPos2015::Instance()->getDefPos2015(pVision).getRightPos();
      double goal2leftdir  = (DefPos2015::Instance()->getDefPos2015(pVision).getLeftPos() - ball.Pos()).dir() - 0.3;
      double goal2rightdir = (DefPos2015::Instance()->getDefPos2015(pVision).getRightPos() - ball.Pos()).dir() + 0.3;
      if (AngleBetween(clearBallDir, goal2leftdir, goal2rightdir)) {
        if (goal2rightdir >= 0 && goal2leftdir >= 0) {
          clearBallDir = Normalize(goal2leftdir - Math::PI / 6);
        } else if (goal2leftdir <= 0 && goal2rightdir <= 0) {
          clearBallDir = Normalize(goal2rightdir + Math::PI / 6);
        } else {
          double toLeft  = Normalize(clearBallDir - goal2leftdir);
          double toRight = Normalize(goal2rightdir - clearBallDir);
          if (fabs(toLeft) > fabs(toRight))
            clearBallDir = Normalize(goal2rightdir + Math::PI / 6);
          else
            clearBallDir = Normalize(goal2leftdir - Math::PI / 6);
        }
      }
    } else if (TaskMediator::Instance()->singleBack() != 0) {
      double goal2singledir = (DefPos2015::Instance()->getDefPos2015(pVision).getSinglePos() - ball.Pos()).dir();
      if (fabs(goal2singledir - clearBallDir) < Math::PI / 18) {
        if (fabs(clearBallDir) > Math::PI / 10) {
          clearBallDir = -clearBallDir;
          if (fabs(clearBallDir) > Math::PI * 70 / 180) {
            if (clearBallDir < 0)
              clearBallDir = clearBallDir + Math::PI / 9;
            else
              clearBallDir = clearBallDir - Math::PI / 9;
          }
        } else {
          clearBallDir = clearBallDir + Math::PI * 7 / 18;
        }
      }
    } else {
      clearBallDir = (ball.Pos() - GOAL_CENTER).dir();
    }

    /* ���ǶԷ����ų�����ȽϽ������ */
    if (enemy.Pos().dist(ball.Pos()) < 100)
      clearBallDir =  CVector(ball.Pos() - GOAL_CENTER).dir();

    /* �����Ƕȷ�Χ */
    if (clearBallDir >= Math::PI * 80 / 180) {
      clearBallDir = Math::PI * 80 / 180;
    } else if (clearBallDir <= -Math::PI * 80 / 180) {
      clearBallDir = -Math::PI * 80 / 180;
    }

    /* Stop״̬�²����� */
    if (WorldModel::Instance()->CurrentRefereeMsg() == "gameStop") {
      clearBall = false;
      myTask.player.pos = normalGoaliePoint;
      if (ball.Valid())
        myTask.player.angle = CVector(ball.Pos() - me.Pos()).dir();
      else
        myTask.player.angle = CVector(me.Pos() - GOAL_CENTER).dir();
      setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
    }

    if (clearBall == true) {
      if (DEBUG_INFO_FLAG)
        cout << pVision->Cycle() << " ";
      if (ball.Pos().dist(LEFT_GOAL_POST) < Vehicle::V2::PLAYER_SIZE*DANGER_FACTOR && ball.Pos().y() >= LEFT_GOAL_POST.y()
          && CVector(ball.Pos() - me.Pos()).dir() > 0) {
        setSubTask(PlayerRole::makeItGoto(goalieNum, LEFT_GOAL_CENTER_FRONT, (ball.Pos() - me.Pos()).dir(), CVector(0, 0), 0, 0));
        if (DEBUG_INFO_FLAG)
          cout << "left goal center front " << endl;
      } else if (ball.Pos().dist(RIGHT_GOAL_POST) < Vehicle::V2::PLAYER_SIZE*DANGER_FACTOR && ball.Pos().y() <= RIGHT_GOAL_POST.y()
          && CVector(ball.Pos() - me.Pos()).dir() < 0) {
        setSubTask(PlayerRole::makeItGoto(goalieNum, RIGHT_GOAL_CENTER_FRONT, (ball.Pos() - me.Pos()).dir(), CVector(0, 0), 0, 0));
        if (DEBUG_INFO_FLAG)
          cout << "right goal center front " << endl;
      } else if (isBallShotToGoal(pVision) && ball.Pos().x() > me.Pos().x() + Vehicle::V2::PLAYER_FRONT_TO_CENTER) {
        //const double patrolDir = Normalize(ball.Vel().dir() + Math::PI/2);
        const double patrolDir = Math::PI / 2;
        const CGeoLine patrolLine(me.Pos(), patrolDir);
        const CGeoLine ballVelLine(ball.Pos(), ball.Vel().dir());
        const CGeoLineLineIntersection intersection(patrolLine, ballVelLine);
        DribbleStatus::Instance()->setDribbleCommand(goalieNum, 2);
        if (DEBUG_INFO_FLAG)
          cout << "ball shot ";
        if (intersection.Intersectant() == true) {
          const CGeoPoint& point = intersection.IntersectPoint();
          double interceptDir = Normalize(ball.Vel().dir() + Math::PI);
          setSubTask(PlayerRole::makeItGoto(goalieNum, point, interceptDir, CVector(0, 0), 0, 0));
          if (DEBUG_INFO_FLAG) {
            GDebugEngine::Instance()->gui_debug_line(me.Pos(), me.Pos() + Polar2Vector(200, interceptDir), COLOR_WHITE);
            cout << "intercept " << endl;
          }
        } else {
          setSubTask(PlayerRole::makeItNoneTrajGetBall(goalieNum, clearBallDir, CVector(0, 0), PlayerStatus::QUICKLY | PlayerStatus::DRIBBLING));
          if (DEBUG_INFO_FLAG) {
            GDebugEngine::Instance()->gui_debug_line(me.Pos(), me.Pos() + Polar2Vector(200, clearBallDir), COLOR_WHITE);
            cout << "get ball 1 " << endl;
          }
        }
      } else {
        DribbleStatus::Instance()->setDribbleCommand(goalieNum, 2);
        setSubTask(PlayerRole::makeItNoneTrajGetBall(goalieNum, clearBallDir, CVector(0, 0), PlayerStatus::QUICKLY | PlayerStatus::DRIBBLING));
        if (DEBUG_INFO_FLAG)
          cout << "get ball 2 " << endl;
      }
    }
  }

  /**
   * FILL IN
   * ��һ֡�ж���Ҫ��λ ������һ֡�ǲ�λ״̬
   * ͬʱ��������״̬
   */
  else if (GOALIE_FILL_IN_MODE && (fillInTarget != NONE || fillIn != NONE) && clearBall == false) {

    /* fillIn����һ֡�Ĳ�λ״̬ */
    if (fillInTarget == NONE)
      fillInTarget = fillIn;
    fillIn = fillInTarget;

    switch (fillInTarget) {
      case (BALL) : {
        CGeoPoint defendPoint = getFillInBallPosition(pVision);
        myTask.player.pos.setX(defendPoint.x());
        myTask.player.pos.setY(defendPoint.y());
        myTask.player.angle = (ball.Pos() - GOAL_CENTER).dir();
        if (DEBUG_INFO_FLAG)
          GDebugEngine::Instance()->gui_debug_arc(defendPoint, 3, 0, 360, COLOR_RED);
      }
      if (DEBUG_INFO_FLAG)
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(100, 100), "BALL");
      break;

      case (OURBACKS) : {
        CGeoPoint defendPoint = getFillInBacksPosition(pVision);
        myTask.player.pos.setX(defendPoint.x());
        myTask.player.pos.setY(defendPoint.y());
        myTask.player.angle = (ball.Pos() - GOAL_CENTER).dir();
      }
      if (DEBUG_INFO_FLAG)
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(100, 100), "OURBACKS");
      break;

      case (NONE) :
      if (DEBUG_INFO_FLAG)
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(100, 100), "NONE");
      break;
    }

    /* ������������Ա��ʱ����λ */
    if (ball.Pos().dist(LEFT_GOAL_POST) < Vehicle::V2::PLAYER_SIZE*DANGER_FACTOR
        && ball.Pos().y() >= LEFT_GOAL_POST.y()
        && CVector(ball.Pos() - me.Pos()).dir() > 0) {
      fillIn = NONE;
      clearBall = true;
      setSubTask(PlayerRole::makeItGoto(goalieNum, LEFT_GOAL_CENTER_FRONT, (ball.Pos() - me.Pos()).dir(), CVector(0, 0), 0, 0));
    } else if (ball.Pos().dist(RIGHT_GOAL_POST) < Vehicle::V2::PLAYER_SIZE*DANGER_FACTOR
        && ball.Pos().y() <= RIGHT_GOAL_POST.y()
        && CVector(ball.Pos() - me.Pos()).dir() < 0) {
      fillIn = NONE;
      clearBall = true;
      setSubTask(PlayerRole::makeItGoto(goalieNum, RIGHT_GOAL_CENTER_FRONT, (ball.Pos() - me.Pos()).dir(), CVector(0, 0), 0, 0));
    } else if (ball.Pos().dist(GOAL_CENTER) < me.Pos().dist(GOAL_CENTER)) {
      fillIn = NONE;
      clearBall = true;
      DribbleStatus::Instance()->setDribbleCommand(goalieNum, 2);
      setSubTask(PlayerRole::makeItNoneTrajGetBall(goalieNum, myTask.player.angle, CVector(0, 0), PlayerStatus::QUICKLY | PlayerStatus::DRIBBLING));
    } else {
      setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
    }
    if (DEBUG_INFO_FLAG)
      GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(200, 0), "in fill in");
  }

  /**
   * NORMAL
   * �����ܵ�״̬ ������ȼ� ��DefPos2015������ĵ�
   */
  else {
    fillIn = NONE;
    if (DEBUG_INFO_FLAG)
      GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(200, 0), "in normal goalie");
    /* ����Ա����������ز��� */
    if (me.Y() > RIGHT_GOAL_POST.y() || me.Y() < LEFT_GOAL_POST.y()) {
      if (myTask.player.pos.x() < -Field::PITCH_LENGTH/2 + Vehicle::V2::PLAYER_SIZE)
        myTask.player.pos = CGeoPoint(-Field::PITCH_LENGTH/2 + Vehicle::V2::PLAYER_SIZE, me.Y());
    }
    setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(myTask));
  }

  /**
   * ����״̬����
   */
  if (ball.Valid() && InOurPenaltyArea(ball.Pos(), PENALTY_BUFFER) == false)
    clearBall = false;

  /**
   * ��λ״̬����
   */
  if (clearBall == true) {
    fillIn = NONE;
  } else if (fillIn != NONE) {
    static int cycles = 0;
    if (InOurPenaltyArea(ball.RawPos(), 0))
      cycles = 0;
    else
      ++cycles;
    if (cycles > 5) {
      fillIn = NONE;
      cycles = 0;
    }
    if (isBallShotToGoal(pVision))
      fillIn = NONE;
  }
  lastGoaliePoint.setX(me.Pos().x());
  lastGoaliePoint.setY(me.Pos().y());
  CStatedTask::plan(pVision);
}

CPlayerCommand* CGoalie2015Hefei::execute(const CVisionModule* pVision) {
  if (subTask())
    return subTask()->execute(pVision);
  return 0;
}

CGeoPoint CGoalie2015Hefei::getFillInBallPosition(const CVisionModule* pVision) {
  const MobileVisionT& ball = pVision->Ball();

  /**
   * ������ٱȽϴ� ȥ�����߽����
   */
  if (ball.Vel().mod() > 180) {
    /* ���������ߺͽ����Ľ��� */
    const CGeoLine ballVelLine(ball.Pos(), ball.Vel().dir());
    const CGeoPoint leftCircleCenter(-Field::PITCH_LENGTH/2, -Field::PENALTY_AREA_L/2);
    const CGeoPoint rightCircleCenter(-Field::PITCH_LENGTH/2, Field::PENALTY_AREA_L/2);
    const CGeoCirlce leftPenaltyCircle(leftCircleCenter, Field::PENALTY_AREA_R);
    const CGeoCirlce rightPenaltyCircle(rightCircleCenter, Field::PENALTY_AREA_R);
    const CGeoPoint leftPenaltySegmentEnd(-Field::PITCH_LENGTH/2 + Field::PENALTY_AREA_DEPTH, -Field::PENALTY_AREA_L/2);
    const CGeoPoint rightPenaltySegmentEnd(-Field::PITCH_LENGTH/2 + Field::PENALTY_AREA_DEPTH, Field::PENALTY_AREA_L/2);
    const CGeoLine middlePenaltySegment(leftPenaltySegmentEnd, rightPenaltySegmentEnd);

    const CGeoLineLineIntersection penaltyLineIntersection(ballVelLine, middlePenaltySegment);
    const CGeoLineCircleIntersection leftPenaltyIntersection(ballVelLine, leftPenaltyCircle);
    const CGeoLineCircleIntersection rightPenaltyIntersection(ballVelLine, rightPenaltyCircle);

    /* �����ߺͽ����߽������������ */
    const CGeoPoint* points[3] = {0};

    /* �������м�ֱ���� */
    if (penaltyLineIntersection.Intersectant()) {
      const CGeoPoint& point = penaltyLineIntersection.IntersectPoint();
      if (point.y() <= rightPenaltySegmentEnd.y() && point.y() >= leftPenaltySegmentEnd.y()) {
        points[0] = &point;
        if (DEBUG_INFO_FLAG)
          cout << "seg" << endl;
      }
    }

    /* ����������Բ���� */
    if (leftPenaltyIntersection.intersectant() || rightPenaltyIntersection.intersectant()) {
      const CGeoPoint* pointL[2] = {0};
      const CGeoPoint* pointR[2] = {0};
      if (leftPenaltyIntersection.intersectant()) {
        pointL[0] = &leftPenaltyIntersection.point1();
        pointL[1] = &leftPenaltyIntersection.point2();
        if (pointL[0]->x() < leftCircleCenter.x() || pointL[0]->y() > leftCircleCenter.y()) pointL[0] = 0;
        if (pointL[1]->x() < leftCircleCenter.x() || pointL[1]->y() > leftCircleCenter.y()) pointL[1] = 0;
        if (DEBUG_INFO_FLAG)
          cout << "left circle" << endl;
      }
      if (rightPenaltyIntersection.intersectant()) {
        pointR[0] = &rightPenaltyIntersection.point1();
        pointR[1] = &rightPenaltyIntersection.point2();
        if (pointR[0]->x() < rightCircleCenter.x() || pointR[0]->y() < rightCircleCenter.y()) pointR[0] = 0;
        if (pointR[1]->x() < rightCircleCenter.x() || pointR[1]->y() < rightCircleCenter.y()) pointR[1] = 0;
        if (DEBUG_INFO_FLAG)
          cout << "right circle" << endl;
      }
      if (pointL[0] != 0) {
        if      (points[0] == 0) points[0] = pointL[0];
        else                     points[1] = pointL[0];
      }
      if (pointL[1] != 0) {
        if      (points[0] == 0) points[0] = pointL[1];
        else if (points[1] == 0) points[1] = pointL[1];
        else                     points[2] = pointL[1];
      }
      if (pointR[0] != 0) {
        if      (points[0] == 0) points[0] = pointR[0];
        else if (points[1] == 0) points[1] = pointR[0];
        else if (points[2] == 0) points[2] = pointR[0];
      }
      if (pointR[1] != 0) {
        if      (points[0] == 0) points[0] = pointR[1];
        else if (points[1] == 0) points[1] = pointR[1];
        else if (points[2] == 0) points[2] = pointR[1];
      }
    }

    if (DEBUG_INFO_FLAG) {
      if (points[0] != 0) {GDebugEngine::Instance()->gui_debug_arc(*points[0], 5, 0, 360); cout << 1111 << endl;}
      if (points[1] != 0) {GDebugEngine::Instance()->gui_debug_arc(*points[1], 5, 0, 360); cout << 2222 << endl;}
      if (points[2] != 0) {GDebugEngine::Instance()->gui_debug_arc(*points[2], 5, 0, 360); cout << 3333 << endl;}
    }

    /* �ۺϿ��ǽ��� �ҳ�����ʵ� */
    const CGeoPoint* defendTarget = 0;
    if (ball.VelY() > 0) {
      int count = 0;
      for (int i = 0; i < 3; ++i) {
        if (points[i] != 0)
          ++count;
      }
      if (count > 0) {
        const CGeoPoint* tmpRightest = points[0];
        for (int i = 1; i < count; ++i) {
          if (points[i]->y() > tmpRightest->y())
            tmpRightest = points[i];
        }
        if (tmpRightest->y() > ball.RawPos().y())
          defendTarget = tmpRightest;
      }
      /*
      if (points[0] != 0) {
        if (points[0]->y() > ball.RawPos().y()) {
          defendTarget = points[0];
          if (points[1] != 0) {
            if (points[1]->y() > defendTarget->y()) {
              defendTarget = points[1];
              if (points[2] != 0) {
                if (points[2]->y() > defendTarget->y())
                  defendTarget = points[2];
              }
            }
          }
        }
      }
      */
    } else {
      int count = 0;
      for (int i = 0; i < 3; ++i) {
        if (points[i] != 0)
          ++count;
      }
      if (count > 0) {
        const CGeoPoint* tmpLeftest = points[0];
        for (int i = 1; i < count; ++i) {
          if (points[i]->y() < tmpLeftest->y())
            tmpLeftest = points[i];
        }
        if (tmpLeftest->y() < ball.RawPos().y())
          defendTarget = tmpLeftest;
      }
      /*
      if (points[0] != 0) {
        if (points[0]->y() < ball.RawPos().y()) {
          defendTarget = points[0];
          if (points[1] != 0) {
            if (points[1]->y() < defendTarget->y()) {
              defendTarget = points[1];
              if (points[2] != 0) {
                if (points[2]->y() < defendTarget->y())
                  defendTarget = points[2];
              }
            }
          }
        }
      }
      */
    }
    if (defendTarget != 0) {
      /* ����ƽ���� */
      double leftPostToBallDir = (*defendTarget - LEFT_GOAL_POST).dir();
      double rightPostToBallDir = (*defendTarget - RIGHT_GOAL_POST).dir();
      double defendDir = Normalize((leftPostToBallDir + rightPostToBallDir) / 2 + Math::PI);

      /* �Ƕ����� */
      if (defendDir > 30 / 180 * Math::PI)
        defendDir -= 10 / 180 * Math::PI;
      else if (defendDir < -30 / 180 * Math::PI)
        defendDir += 10 / 180 * Math::PI;

      if (DEBUG_INFO_FLAG) {
        const CGeoPoint tmp = *defendTarget + Polar2Vector(100, defendDir + Math::PI);
        GDebugEngine::Instance()->gui_debug_line(*defendTarget, tmp, COLOR_YELLOW);
      }
      return *defendTarget + Polar2Vector(Vehicle::V2::PLAYER_SIZE*2.5, defendDir);
    }
  }

  /**
   * ����ٶȱȽ�С
   * ���������ߺͽ����߽�û�����õĽ���
   * ����ƽ����
   */
  double leftPostToBallDir = (ball.Pos() - LEFT_GOAL_POST).dir();
  double rightPostToBallDir = (ball.Pos() - RIGHT_GOAL_POST).dir();
  double defendDir = Normalize((leftPostToBallDir + rightPostToBallDir) / 2 + Math::PI);

  /* �Ƕ����� */
  if (defendDir > 30 / 180 * Math::PI)
    defendDir -= 10 / 180 * Math::PI;
  else if (defendDir < -30 / 180 * Math::PI)
    defendDir += 10 / 180 * Math::PI;

  /* ���¼���һ�������ܵ� �����������Ĳ��� */
  const CGeoPoint RBallPos = DefendUtils::reversePoint(pVision->Ball().Pos());
  const double RDedefendDir = Normalize(defendDir + Math::PI);
  const CGeoPoint RDefendPoint = DefendUtils::calcGoaliePointV3(RBallPos, RDedefendDir, posSide::POS_SIDE_MIDDLE, lastGoaliePoint, 1);
  CGeoPoint defendPoint = DefendUtils::reversePoint(RDefendPoint);

  /* ��ֹ����Աײ���� */
  const CGeoPoint* POST = 0;
  if (defendPoint.dist(LEFT_GOAL_POST) < Vehicle::V2::PLAYER_SIZE)
    POST = &LEFT_GOAL_POST;
  else if (defendPoint.dist(RIGHT_GOAL_POST) < Vehicle::V2::PLAYER_SIZE)
    POST = &RIGHT_GOAL_POST;
  if (POST != 0) {
    CGeoLine meToPostLine(defendPoint, *POST);
    CGeoCirlce postCircle(*POST, Vehicle::V2::PLAYER_SIZE);
    CGeoLineCircleIntersection intersection(meToPostLine, postCircle);
    const CGeoPoint& point1 = intersection.point1();
    const CGeoPoint& point2 = intersection.point2();
    if (point1.y() > -Field::PITCH_LENGTH / 2) {
      defendPoint.setX(point1.x());
      defendPoint.setY(point1.y());
    } else if (point2.y() > -Field::PITCH_LENGTH / 2) {
      defendPoint.setX(point2.x());
      defendPoint.setY(point2.y());
    }
  }
  return defendPoint;
}

CGeoPoint CGoalie2015Hefei::getFillInBacksPosition(const CVisionModule* pVision) {
  const MobileVisionT& ball = pVision->Ball();
  const CGeoLine ballToGoalLine(ball.RawPos(), GOAL_CENTER);
  const CGeoPoint leftPenaltySegmentEnd(-Field::PITCH_LENGTH / 2 + Field::PENALTY_AREA_DEPTH, -Field::PENALTY_AREA_L / 2);
  const CGeoPoint rightPenaltySegmentEnd(-Field::PITCH_LENGTH / 2 + Field::PENALTY_AREA_DEPTH, Field::PENALTY_AREA_L / 2);
  const CGeoLine middlePenaltySegment(leftPenaltySegmentEnd, rightPenaltySegmentEnd);
  const CGeoLineLineIntersection penaltyLineIntersection(ballToGoalLine, middlePenaltySegment);
  if (penaltyLineIntersection.Intersectant() == true
      && penaltyLineIntersection.IntersectPoint().y() > leftPenaltySegmentEnd.y()
      && penaltyLineIntersection.IntersectPoint().y() < rightPenaltySegmentEnd.y()) {
    const CGeoPoint& point = penaltyLineIntersection.IntersectPoint();
    double defendDir = (GOAL_CENTER - point).dir();
    return point + Polar2Vector(Vehicle::V2::PLAYER_SIZE*2, defendDir);
  } else {
    const CGeoPoint& point = penaltyLineIntersection.Intersectant() ? penaltyLineIntersection.IntersectPoint() : ball.RawPos();
    if (point.y() < 0) {  // �����Բ���ཻ
      const CGeoPoint leftCircleCenter(-Field::PITCH_LENGTH/2, -Field::PENALTY_AREA_L/2);
      const CGeoCirlce leftPenaltyCircle(leftCircleCenter, Field::PENALTY_AREA_R);
      const CGeoLineCircleIntersection leftPenaltyIntersection(ballToGoalLine, leftPenaltyCircle);
      const CGeoPoint& point1 = leftPenaltyIntersection.point1();
      const CGeoPoint& point2 = leftPenaltyIntersection.point2();
      if (point1.y() < leftCircleCenter.y()) {
        double defendDir = (GOAL_CENTER - point1).dir();
        return point1 + Polar2Vector(Vehicle::V2::PLAYER_FRONT_TO_CENTER, defendDir);
      } else {
        double defendDir = (GOAL_CENTER - point2).dir();
        return point2 + Polar2Vector(Vehicle::V2::PLAYER_FRONT_TO_CENTER, defendDir);
      }
    } else {  // ���ұ�Բ���ཻ
      const CGeoPoint rightCircleCenter(-Field::PITCH_LENGTH/2, Field::PENALTY_AREA_L/2);
      const CGeoCirlce rightPenaltyCircle(rightCircleCenter, Field::PENALTY_AREA_R);
      const CGeoLineCircleIntersection rightPenaltyIntersection(ballToGoalLine, rightPenaltyCircle);
      const CGeoPoint& point1 = rightPenaltyIntersection.point1();
      const CGeoPoint& point2 = rightPenaltyIntersection.point2();
      if (point1.y() > rightCircleCenter.y()) {
        double defendDir = (GOAL_CENTER - point1).dir();
        return point1 + Polar2Vector(Vehicle::V2::PLAYER_FRONT_TO_CENTER, defendDir);
      } else {
        double defendDir = (GOAL_CENTER - point2).dir();
        return point2 + Polar2Vector(Vehicle::V2::PLAYER_FRONT_TO_CENTER, defendDir);
      }
    }
  }
}

CGoalie2015Hefei::FillInTarget CGoalie2015Hefei::needToFillIn(const CVisionModule* pVision) {
  int goalie = PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie());
  const MobileVisionT& ball = pVision->Ball();
  const CGeoPoint& rawBallPos = ball.RawPos();

  /**
   * 1. ���Ǽ��ֲ���λ�������
   *   - Stop״̬����û����������λ
   *   - �����˲���λ
   *   - ���ٹ��󲻲�λ
   *   - �������Զ����λ
   *   - ���ڽ��������λ
   *   - ���������ȥ�˲���λ
   */
  if (pVision->GetCurrentRefereeMsg() == "gameStop") {
    if (DEBUG_INFO_FLAG)
      GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(300, 0), "fillin: stop");
    return NONE;
  }
  if (ball.Valid() == true) {
    bool isBallShotToTarget = isBallShotToGoal(pVision);
    if (isBallShotToTarget == true && ball.Vel().mod() > 100) {
      if (DEBUG_INFO_FLAG)
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(300, 0), "fillin: ball shot");
      return NONE;
    }
  }
  if (ball.Vel().mod() > 600) {
    if (DEBUG_INFO_FLAG)
      GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(300, 0), "fillin: ball too fast");
    return NONE;
  }
  if (InOurPenaltyArea(rawBallPos, 0) == false) {
    if (DEBUG_INFO_FLAG)
      GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(300, 0), "fillin: ball too far");
    return NONE;
  }
  //cout << rawBallPos.x() << " " << rawBallPos.y() << endl;
  if (InOurPenaltyArea(rawBallPos, -Field::PENALTY_AREA_DEPTH/3*2)
      || rawBallPos.x() < -Field::PITCH_LENGTH/2 + Field::PENALTY_AREA_DEPTH/3*2) {
    if (DEBUG_INFO_FLAG)
      GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(300, 0), "fillin: ball too near");
    return NONE;
  }
  if (ball.VelX() < 0) {
    if (DEBUG_INFO_FLAG)
      GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(300, 0), "fillin: ball goto baseline");
    return NONE;
  }

  /**
   * 2. �����������к������漷��һ�ŵ����
   *    ������ڽ�������
   */
  bool isBallBehindBacks = true;
  double ballToGoalDist = rawBallPos.dist(GOAL_CENTER);
  for (int i = 1; i <= Field::MAX_PLAYER; ++i) {
    if (i != goalie && pVision->OurPlayer(i).Pos().dist(rawBallPos) < ballToGoalDist) {
      isBallBehindBacks = false;
      break;
    }
  }
  if (isBallBehindBacks == true
      && pVision->Ball().Vel().mod() < 150
      && InOurPenaltyArea(rawBallPos, 1 == false))
    return OURBACKS;

  /**
   * 3. ������ᴩ���������
   */
  return BALL;
}

bool CGoalie2015Hefei::needToClearBall(const CVisionModule* pVision) {
  bool result = false;
  const MobileVisionT& ball = pVision->Ball();
  int advancer = BestPlayer::Instance()->getOurBestPlayer(); 
  const PlayerVisionT enemy = pVision->TheirPlayer(BestPlayer::Instance()->getTheirBestPlayer());
  // 150 120
  if (((ball.Vel().mod() < 150 && InOurPenaltyArea(ball.Pos(), -30)) ||
    (ball.Vel().mod() < 50 && InOurPenaltyArea(ball.Pos(), PENALTY_BUFFER)) && ball.Valid()
    && !checkWeHaveHelper(pVision) && !InOurPenaltyArea(enemy.Pos(), DEFEND_PENALTY_BUFFER)) ||
    ((ball.Vel().mod() < 120 && InOurPenaltyArea(ball.Pos(), -30)) ||
    (ball.Vel().mod() < 40 && InOurPenaltyArea(ball.Pos(), PENALTY_BUFFER)) && ball.Valid()
    && !checkWeHaveHelper(pVision) && InOurPenaltyArea(enemy.Pos(), DEFEND_PENALTY_BUFFER))){
    result = true;
    if (DEBUG_INFO_FLAG)
      GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(50,0),"need to clear ball");
  }
  return result;
}

bool CGoalie2015Hefei::checkWeHaveHelper(const CVisionModule* pVision) {
  bool result = false;
  int helper[Field::MAX_PLAYER+1]={0};
  int helpernum = 0;
  int bestnum = BestPlayer::Instance()->getOurBestPlayer();
  const MobileVisionT& ball = pVision->Ball();
  for (int i = 1;i<= Field::MAX_PLAYER;i++){
    if (pVision->OurPlayer(i).Pos().dist(ball.Pos())<30){
      helpernum ++;
      helper[helpernum] = i;
    }
  }
  if (InOurPenaltyArea(ball.Pos(),PENALTY_BUFFER) && !InOurPenaltyArea(ball.Pos(),0)){
    if (pVision->TheirPlayer(bestnum).X()<ball.X() && 
        abs((GOAL_CENTER - ball.Pos()).dir() - (GOAL_CENTER - pVision->TheirPlayer(bestnum).Pos()).dir())<Math::PI/4
        && helpernum>=3)
      result = true;
  }
  return result;
}

bool CGoalie2015Hefei::isBallShotToGoal(const CVisionModule* pVision) {
  const MobileVisionT& ball = pVision->Ball();
  const CGeoPoint& ballPos = ball.Pos();
  CGeoLine baseLine(LEFT_PENALTY_END, RIGHT_PENALTY_END);
  CGeoLine ballVelLine(ballPos, ball.Vel().dir());
  CGeoLineLineIntersection intersetion(baseLine, ballVelLine);
  const CGeoPoint& point = intersetion.IntersectPoint();
  bool isBallShotToTarget = false;
  if (ball.Vel().mod() > 50
      && ball.VelX() < 0
      && point.y() < RIGHT_GOAL_POST.y() + Vehicle::V2::PLAYER_SIZE
      && point.y() > LEFT_GOAL_POST.y() - Vehicle::V2::PLAYER_SIZE)
    isBallShotToTarget = true;
  return isBallShotToTarget;
}

int CGoalie2015Hefei::getTheirReceiverCount(const CVisionModule* pVision) { int count = 0;
  const MobileVisionT& ball = pVision->Ball();

  for (int i = 1; i <= Field::MAX_PLAYER; ++i) {
    const PlayerVisionT& enemy = pVision->TheirPlayer(i);
    double ballToEnemyDir = (enemy.Pos() - ball.Pos()).dir();
    double enemyDir = enemy.Dir();
    double diffDir  = fabs(Normalize(ballToEnemyDir - enemyDir));

    double blockDir;
    if (diffDir <= Math::PI * 100 / 180) // �Է����Ӳ�����
      blockDir = DefendUtils::calcBlockAngle(ball.Pos(), enemy.Pos()) + Math::PI*3/180;
    else                             // �Է����ܽӵ���
      blockDir = DefendUtils::calcBlockAngle(ball.Pos(), enemy.Pos()) + Math::PI*10/180;

    if (ballToEnemyDir + blockDir <= Math::PI && ballToEnemyDir - blockDir >= -Math::PI) {
      if (ball.Vel().dir() < ballToEnemyDir + blockDir && ball.Vel().dir() > ballToEnemyDir - blockDir)
        ++count;
    } else if (ballToEnemyDir - blockDir >= -Math::PI) {
      if (ball.Vel().dir() < Normalize(ballToEnemyDir + blockDir) || ball.Vel().dir() > ballToEnemyDir - blockDir)
        ++count;
    } else if (ballToEnemyDir + blockDir <= Math::PI) {
      if (ball.Vel().dir() > Normalize(ballToEnemyDir - blockDir) || ball.Vel().dir() < ballToEnemyDir + blockDir)
        ++count;
    } else {
      if (ball.Vel().dir() < Normalize(ballToEnemyDir + blockDir) || ball.Vel().dir() > Normalize(ballToEnemyDir - blockDir))
        ++count;
    }
  }

  return count;
}
