#ifndef _WAIT_KICK_POS_
#define _WAIT_KICK_POS_

#include <VisionModule.h>

class CWaitKickPos {
public:
	///> ���캯��
	CWaitKickPos();

	///> ����ĵ��ýӿ�
	void GenerateWaitKickPos(const CGeoPoint pointA,const double angle,const int player,const double kickdir);
	void GenerateWaitKickPos(const CGeoPoint pointA,const CGeoPoint pointB,const int player,const double kickdir);
	CGeoPoint getKickPos(int player);
	void reset();
};

typedef NormalSingleton< CWaitKickPos > WaitKickPos;

#endif 