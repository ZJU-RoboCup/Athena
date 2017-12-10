#ifndef _WAIT_KICK_POS_
#define _WAIT_KICK_POS_

#include <VisionModule.h>

class CWaitKickPos {
public:
	///> 构造函数
	CWaitKickPos();

	///> 对外的调用接口
	void GenerateWaitKickPos(const CGeoPoint pointA,const double angle,const int player,const double kickdir);
	void GenerateWaitKickPos(const CGeoPoint pointA,const CGeoPoint pointB,const int player,const double kickdir);
	CGeoPoint getKickPos(int player);
	void reset();
};

typedef NormalSingleton< CWaitKickPos > WaitKickPos;

#endif 