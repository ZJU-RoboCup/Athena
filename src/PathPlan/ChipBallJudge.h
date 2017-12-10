#ifndef _CHIPBALL_JUDGE_H_
#define _CHIPBALL_JUDGE_H_
#include <VisionModule.h>

class CVisionModule;

class CChipBallJudge
{
public:
	CChipBallJudge();
	~CChipBallJudge();
	bool doJudge(const CVisionModule* pVision,double & ballActualMovingDir, CGeoPoint& ballStartPos);
	bool isBallKicked(const CVisionModule* pVision);
	void checkKickCarNum(const CVisionModule* pVision);
	bool checkKickerGoKick(const CVisionModule* pVision);
	bool isChip(const CVisionModule* pVision);
private:
	int _cycle;
	int _lastCycle;
	CGeoPoint ballInialPos;
	int theirKickerID;
	double initialKickerDir;
	bool isFirstGetRefereeMsg;
	bool goKickAlrealdy;
	int count ;
};
typedef NormalSingleton< CChipBallJudge > ChipBallJudge;
#endif