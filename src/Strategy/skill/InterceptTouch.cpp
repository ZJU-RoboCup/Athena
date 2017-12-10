#include "InterceptTouch.h"

#include <cmath>
#include <iostream>
#include <string>

#include "geometry.h"
#include "param.h"
#include "utils.h"

#include "DribbleStatus.h"
#include "Factory.h"
#include "GDebugEngine.h"
#include "VisionModule.h"
#include "KickStatus.h"
#include <tinyxml/ParamReader.h>
using namespace std;
using namespace Param;

namespace {

namespace Threshold {

const double MIN_DIFF_DIR = Math::PI / 36;  // С�����ֵ���㷽����ȷ
const double MIN_P2L_DIST = 5;              // С�����ֵ���㵽��
const double MAX_P2L_DIST = 10;             // �������ֵ����û����
const double MIN_INTERCEPT_DIST = 300;      // �ܽص������������ С�����ֵ���ܽص�
const double MAX_NEED_QUICK_RUN_ME_TO_TARGET_DIST = 20;    // ֹͣ���ٵ���ĳ���Ŀ��������ֵ �������ֵ����Ҫ
const double MIN_BALL_MOVING_VEL = 40;      // ����С�����ֵ���㲻�ڹ�

const int MAX_BUFFER_COUNTER_THRESHOLD = 2;
const int MIN_DRIBBLE_DIST_THRESHOLD = 30;  // ������ľ�����ֵ С�����ֵ�Ϳ�����
const int SAFE_DISTANCE = 30;
}

const int DRIBBLE_POWER = 3;
const int BALL_SPEED_STAT = 60;
static CGeoPoint waitPos;
int FRICTION;
}

CInterceptTouch::CInterceptTouch() : buffer_counter_(0), last_cycle_(0) {
	DECLARE_PARAM_READER_BEGIN(FieldParam)
		READ_PARAM(FRICTION)
	DECLARE_PARAM_READER_END
}

void CInterceptTouch::plan(const CVisionModule* pVision) {
  using namespace Threshold;

	if ( pVision->Cycle() - last_cycle_ > 6 )
		setState(BEGINNING);

  const int runner = task().executor;
  if (task().player.pos.x() == 999 && task().player.pos.y() == 999) {
    if (state() == BEGINNING)
      waitPos = pVision->OurPlayer(runner).RawPos();
  } else {
    waitPos = task().player.pos;
  }
  const double touchDir = task().player.angle;
  const double power = task().player.kickpower;
  const double bufferDist = task().player.chipkickpower;
  const bool useChip = task().player.ischipkick;
  const bool testMode = task().ball.front;
  const PlayerVisionT& me = pVision->OurPlayer(runner);
  const MobileVisionT& ball = pVision->Ball();
  const CGeoPoint& myPos = pVision->OurPlayer(runner).Pos();
  const CGeoPoint& ballPos = ball.RawPos();
  const double ballVelDir = ball.Vel().dir();
  
  // �ж����Ƿ����Ҷ���
  const double ballVelMod = ball.Vel().mod();
  const double meToBallDir = (myPos - ballPos).dir();
  const double diffBetweenBallVelDirAndMeToBallDir = Utils::Normalize(ballVelDir - meToBallDir);
  bool isBallMovingToMe = false;
  if (ballVelMod > MIN_BALL_MOVING_VEL && diffBetweenBallVelDirAndMeToBallDir < Math::PI / 2)
    isBallMovingToMe = true;

  // �ж��Ƿ��ܽص���
  const CGeoLine ballVelLine(ballPos, ballVelDir);
  const double perpendicularDir = Utils::Normalize(ballVelDir + Math::PI / 2);
  const CGeoLine perpLineAcrossMyPos(myPos, perpendicularDir);
  const CGeoPoint projectionPos = CGeoLineLineIntersection(ballVelLine, perpLineAcrossMyPos).IntersectPoint();
  const double meToBallVelLineDist = projectionPos.dist(myPos);

  auto ballDir = ball.Vel().dir();
  auto rawBallPos = ball.RawPos();

  bool canInterceptBall = false;
  if (ball.Valid() && isBallMovingToMe) {
    if (projectionPos.x() > -Field::PITCH_LENGTH / 2 && projectionPos.x() < Field::PITCH_LENGTH / 2 &&
        projectionPos.y() > -Field::PITCH_WIDTH / 2  && projectionPos.y() < Field::PITCH_WIDTH / 2) {
      if (meToBallVelLineDist <= MIN_INTERCEPT_DIST)
        canInterceptBall = true;
    }
  }
  const double distMe2Ball = (me.Pos() - ball.RawPos()).mod();
  const double instancePrediction = ballVelMod*ballVelMod / FRICTION;
  bool ballArriveMe = instancePrediction - distMe2Ball > SAFE_DISTANCE && isBallMovingToMe;
  //cout << instancePrediction - distMe2Ball << '\t' << isBallMovingToMe << endl;
  // �ж�״̬��ת
  switch (state()) {
    case BEGINNING: {
      if (canInterceptBall){
        //cout << "can InterceptBall" << endl;
        setState(GOTO_PROJ_POS);
      }
      else{
        //cout << "can not intercept" << endl;
        setState(SIMPLY_GOTO_WAIT_POS);
      }
    } break;

    case SIMPLY_GOTO_WAIT_POS: {
      if (canInterceptBall)
        setState(GOTO_PROJ_POS);
    } break;

    case GOTO_PROJ_POS: {
      if (canInterceptBall == false) {
        setState(SIMPLY_GOTO_WAIT_POS);
        buffer_counter_ = 0;
      } else {
        if (meToBallVelLineDist < MIN_P2L_DIST)
          ++buffer_counter_;
        else
          buffer_counter_ = 0;
        if (buffer_counter_ == MAX_BUFFER_COUNTER_THRESHOLD) {
          setState(INTERCEPT);
          buffer_counter_ = 0;
        }
      }
    } break;
	case SAVE: {
		if (ballArriveMe)
			if (canInterceptBall)
				setState(GOTO_PROJ_POS);
			else
				setState(SIMPLY_GOTO_WAIT_POS);
	}
  }

  if (!testMode && !ballArriveMe)
	  setState(SAVE);
  // ������˾�ǿ������
  const double myDir = me.Dir();
  const double diffBetweenMyDirAndTouchDir = fabs(Utils::Normalize(myDir - touchDir));
  if (diffBetweenMyDirAndTouchDir < MIN_DIFF_DIR) {
    if (useChip)
      KickStatus::Instance()->setChipKick(runner, power);
    else
      KickStatus::Instance()->setKick(runner, power);
  }

  // ����״ִ̬��
  string msg;
  const double meToBallDist = myPos.dist(ballPos);
  CGeoPoint runPos;
  switch (state()) {
    case GOTO_PROJ_POS: {
      runPos = projectionPos;
      // ��Ҫ���ٵ���
      const double myVelMod = me.Vel().mod();
      const double ballToTargetDist = runPos.dist(ballPos);
      const double ballToTargetTime = ballToTargetDist / (ballVelMod + 0.1);
      const double meToTargetDist = runPos.dist(myPos);
      const double meToTargetTime = meToTargetDist / (myVelMod + 0.1);
      if (ballVelMod > 300 && ballToTargetTime < meToTargetTime && meToTargetDist >= MAX_NEED_QUICK_RUN_ME_TO_TARGET_DIST) {
        const double meToTargetDir = (runPos - myPos).dir();
        runPos = runPos + Utils::Polar2Vector(meToTargetDist * meToTargetTime / ballToTargetTime, meToTargetDir);
        //GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-120, 0), "QUICK_RUN");
      }
      //GDebugEngine::Instance()->gui_debug_line(myPos + Utils::Polar2Vector(100, perpendicularDir + Math::PI), myPos + Utils::Polar2Vector(100, perpendicularDir));
	  setSubTask(PlayerRole::makeItGoto(runner, runPos, touchDir));
	  // cout << runPos << endl;
	  msg = "GOTO_PROJ_POS";
    } break;
    case INTERCEPT: {
      const CGeoLine perpLineAcrossWaitPos(waitPos, perpendicularDir);
      const CGeoPoint interceptPos = CGeoLineLineIntersection(perpLineAcrossWaitPos, ballVelLine).IntersectPoint();
      //GDebugEngine::Instance()->gui_debug_line(waitPos + Utils::Polar2Vector(100, perpendicularDir + Math::PI), waitPos + Utils::Polar2Vector(100, perpendicularDir));
      runPos = interceptPos + Utils::Polar2Vector(bufferDist, touchDir + Math::PI);
	  setSubTask(PlayerRole::makeItGoto(runner, runPos, touchDir));
      msg = "INTERCEPT";
    } break;
    default:
    case SIMPLY_GOTO_WAIT_POS: {
      runPos = waitPos;// + Utils::Polar2Vector(bufferDist, touchDir + Math::PI);
	  setSubTask(PlayerRole::makeItGoto(runner, runPos, touchDir));
      msg = "SIMPLY_GOTO_WAIT_POS";
    } break;
	case SAVE: {
		msg = "SAVE";
		//if (ballVelMod > BALL_SPEED_STAT) {
			setSubTask(PlayerRole::makeItChaseKickV2(runner, touchDir, task().player.flag));
		//}
		//else {
		//	setSubTask(PlayerRole::makeItGoAndTurnKick(runner, touchDir, task().player.flag,800));
		//}
		//setSubTask(PlayerRole::makeItShootBall(runner, touchDir, false, 5 * Param::Math::PI / 180, 1000, 1000));
	}
  }

  

  // ����������˾�����
  //if (meToBallDist < MIN_DRIBBLE_DIST_THRESHOLD)
	 // DribbleStatus::Instance()->setDribbleCommand(runner, DRIBBLE_POWER);

  //GDebugEngine::Instance()->gui_debug_line(ballPos, ballPos + Utils::Polar2Vector(500, ballVelDir),COLOR_GREEN);
  GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-300, 0), msg.c_str(),COLOR_GREEN);
  last_cycle_ = pVision->Cycle();
  CPlayerTask::plan(pVision);
}

CPlayerCommand* CInterceptTouch::execute(const CVisionModule* pVision) {
  if (subTask())
    return subTask()->execute(pVision);
  return NULL;
}
