#ifndef _KICKOFF_DEF_POS_V2_
#define _KICKOFF_DEF_POS_V2_

#include <VisionModule.h>

// ø™«Ú∑¿ ÿµ„À„∑® [6/15/2011 zhanfei]

class CKickOffDefPosV2
{
public:
	CKickOffDefPosV2();
	CGeoPoint GetMidPos(const CVisionModule *pVision);
	CGeoPoint GetLeftPos(const CVisionModule *pVision);
	CGeoPoint GetRightPos(const CVisionModule *pVision);

private:
	CGeoPoint GetKickOffDefPos(const CVisionModule *pVision, const int pos_num);
	int GetPosNum(const CVisionModule *pVision, const CGeoPoint pos);
	bool VERBOSE_MODE;
};

typedef NormalSingleton< CKickOffDefPosV2 > KickOffDefPosV2;

#endif // _KICKOFF_DEF_POS_V2_