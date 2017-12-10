#ifndef _RRT_PATH_PLANNER_H_
#define _RRT_PATH_PLANNER_H_
#include "PathPlanner.h"
#include <VisionModule.h>
/**
* CRRTPathPlanner.
* CMU's Random Tree path planner.
*/
class CRRTPathPlanner : public CPathPlanner{
public:
	CRRTPathPlanner(const CVisionModule* pVision, const TaskT& task, const double avoidLength);
};
#endif