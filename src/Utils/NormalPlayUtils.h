#ifndef _NORMALPLAY_UTILS_
#define _NORMALPLAY_UTILS_
#include "VisionModule.h"

namespace NormalPlayUtils{
	bool	isEnemyBlockShootLine(CGeoPoint ball,double shootDir,double range);
	bool	isEnemyFrontToBall(const CVisionModule* pVision,double range);
	bool	isEnemyCloseToReceiver(const CVisionModule* pVision,CGeoPoint receiver,double range);
	bool	isPosInSquareArea(CGeoPoint pos,CGeoPoint smallVertex,CGeoPoint bigVertex);
	bool	isPosCloseBorder(CGeoPoint pos,CGeoPoint smallVertex,CGeoPoint bigVertex);	
	bool	isReceiverCloseToPassPos(const CVisionModule* pVision,CGeoPoint passPos,double range);
	int		getOurMostClosetoPos(const CVisionModule* pVision,CGeoPoint pos);
	int		getTheirMostClosetoPos(const CVisionModule* pVision,CGeoPoint pos);
	bool	ballMoveToOurDefendArea(const CVisionModule* pVision);
	bool	noEnemyInPassLine(const CVisionModule* pVision,CGeoPoint ball,CGeoPoint passPos,const double range);
	bool	canPassBetweenTwoPos(const CVisionModule* pVision,CGeoPoint passPos,int meNum);
	void	generateWaitAdvancePos(const CVisionModule* pVision,CGeoPoint& predictPosOne,CGeoPoint& predictPosTwo);
	bool	generateTwoPassPoint(CGeoPoint ball,CGeoPoint& passPosOne,CGeoPoint& passPosTwo);
	double	generateKickAtEnemyDir(const CVisionModule* pVision,const PlayerVisionT& me);
	int     patchForBestPlayer(const CVisionModule* pVision,int lastAdvancerNum);
	double	generateTandemCond(const CVisionModule* pVision,CGeoPoint& tandemPos,int meNum);
	double  antiNormalize(double dir);
	bool	faceTheirGoal(const CVisionModule* pVision, const int executor,const double pre=Param::Math::PI/180*3 );
	int 	getSuitSider(const CVisionModule* pVision);
}

#endif
