#ifndef _SMART_GOTO_POSITION_V1_H_
#define _SMART_GOTO_POSITION_V1_H_
#include <skill/PlayerTask.h>
#include <WorldDefine.h>
/************************************************************************/
/*                      SmartGotoPosition                               */
/************************************************************************/
class CSmartGotoPositionV2 : public CPlayerTask{
public:
	CSmartGotoPositionV2();
	virtual void plan(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const;

// ![22/5/2010 cliffyin: do trajectory check for obstacle_avoidance]
private:
	bool current_trajectory_safe;
	//void doTrajCheck(const CVisionModule* pVision,int playerNum,const PlayerPoseT& middlePose,PlayerPoseT& nextStep);

	CGeoPoint collidePos;
	CVector collideVel;
	double collideRot;
	int collideNum;
	CGeoPoint generateObstacleAvoidanceTmpPoint(const CVisionModule* pVision,int playerNum,const CGeoPoint& middlePoint);
	CGeoPoint generateObstacleAvoidanceStatic(const CVisionModule* pVision,int playerNum,const CGeoPoint& middlePoint);		//collide with static obstacle
	CGeoPoint generateObstacleAvoidanceDynamic(const CVisionModule* pVision,int playerNum,const CGeoPoint& middlePoint);	//collide with dynamic obstacle

	int playerFlag;
	PlayerPoseT nextStep;
};

#endif