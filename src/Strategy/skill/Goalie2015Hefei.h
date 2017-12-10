#ifndef STRATEGY_SKILL_GOALIE_2015_HEFEI_H_
#define STRATEGY_SKILL_GOALIE_2015_HEFEI_H_
#include "PlayerTask.h"
#include "geometry.h"

/**
 *  @brief  守门员2015年合肥版本
 *  @author wangru@zju.edu.cn
 */

class CGoalie2015Hefei : public CStatedTask {
 public:
	enum FillInTarget {	NONE, BALL, OURBACKS };

  CGoalie2015Hefei();

  virtual bool isEmpty() const { return false; }
  virtual void plan(const CVisionModule* pVision);
  virtual CPlayerCommand* execute(const CVisionModule* pVision);

 protected:
  virtual void toStream(std::ostream& os) const { os << "Skill: CGoalie2015Hefei\n"; }

 private:
  FillInTarget needToFillIn(const CVisionModule* pVision);
  bool needToClearBall(const CVisionModule* pVision);
  bool checkWeHaveHelper(const CVisionModule* pVision);
  bool isBallShotToGoal(const CVisionModule* pVision);
  int getTheirReceiverCount(const CVisionModule* pVision);
  CGeoPoint getFillInBallPosition(const CVisionModule* pVision);
  CGeoPoint getFillInBacksPosition(const CVisionModule* pVision);

 private:
  bool clearBall;
  FillInTarget fillIn;
  CGeoPoint lastGoaliePoint;
};

#endif