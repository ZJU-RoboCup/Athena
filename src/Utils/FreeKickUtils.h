#ifndef _FREEKICK_UTILS_
#define _FREEKICK_UTILS_
#include "VisionModule.h"

namespace FreeKickUtils {
	bool isBeingMarked(const CVisionModule* pVision, int myNum);
	int  getTheirClosesRobotToOurRobot(const CVisionModule* pVision, int myNum);
	bool isShootLineBlockedbyTheirMarkingRobot(const CVisionModule* pVision, int myNum, int theirNum);
	bool isPassLineBlockedbyTheirMarkingRobot(const CVisionModule* pVision, int myNum, int theirNum);
	bool isOppNearMe(const CVisionModule* pVision, int myNum, int theirNum);

	bool isShootLineBlocked(const CVisionModule* pVision, int myNum, double range, double allowance = 3.0);
	bool isShootLineBlockedFromPoint(const CVisionModule* pVision, CGeoPoint pos, double range);
	bool isPassLineBlocked(const CVisionModule* pVision, int myNum, double allowance = 4.0);
	bool isPassLineBlocked(const CVisionModule* pVision, CGeoPoint pos, double allowance = 4.0);
	bool isLightKickBlocked(const CVisionModule* pVision, int myNum, double dir, double R,double halfAngle);
	// ”√”⁄∑¥…‰∞Â
	double drawReflect(const CVisionModule* pVision, int myNum);
	const CGeoPoint getLeftReflectPos(const CVisionModule* pVision, int myNum);
	const CGeoPoint getRightReflectPos(const CVisionModule* pVision, int myNum);
	const CGeoPoint getReflectPos(const CVisionModule* pVision, int myNum);
	const CGeoPoint getReflectTouchPos(const CVisionModule* pVision, int myNum, double y);
}

#endif //_FREEKICK_UTILS_