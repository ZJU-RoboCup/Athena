#ifndef _DEFENDUTILS_H_
#define _DEFENDUTILS_H_
#include <geometry.h>
#include <param.h>
#include <string>
#include <misc_types.h>

enum posSide{
	POS_SIDE_LEFT = 0,
	POS_SIDE_RIGHT,
	POS_SIDE_MIDDLE
};

namespace DefendUtils{
	extern CGeoEllipse D_ELLIPSE;
	extern CGeoEllipse D_ELLIPSE1;

	extern int getPenaltyFronter();
	extern bool isPosInOurPenaltyArea(const CGeoPoint& pos);
     /** 
	@brief  计算某一个点相对于直角坐标系坐标原点的对称点 */
	extern CGeoPoint reversePoint(const CGeoPoint& p);
	/** 
	@brief  计算我方某一队员 对目标点至我方球门的射门角度的 阻碍角 */
	extern double calcBlockAngle(const CGeoPoint& target,const CGeoPoint& player);
	/** 
	@brief  禁区左半圆上的点是否在界内 */
	extern bool leftCirValid(const CGeoPoint& p);
	/** 
	@brief  禁区右半圆上的点是否在界内 */
	extern bool rightCirValid(const CGeoPoint& p);
	/** 
	@brief  找出敌方最有可能射门的队员 */
	extern int getEnemyShooter();
	/** 
	@brief  给出实际射门线，并且更新防守目标点和防守朝向 */
	//传入的第3个参数是防守目标的R坐标值的引用，第四个是相应的R角度
	extern CGeoLine getDefenceTargetAndLine(CGeoPoint& RdefenceTarget,double& RdefendDir);

	/** 
	@brief  计算Stop状态下后卫的站位点 */
	extern CGeoPoint calcDefenderPoint(const CGeoPoint Rtarget,const double Rdir,posSide Rside);
	/**
	@brief  计算守门员的站位点 */
	//传入的第四个参数是上次记录的守门员站位点的正常坐标，在球进入禁区时守门员将调用它进行清球，更新站位，传入的第五个参数为防守目标的朝向的R值

	//计算三车后卫另一辆车的防守角度
	extern CGeoLine getSideBackTargetAndLine(CGeoPoint& RSideTarget,double& RSideDir);
	
	//传入模式,模式0为左右及单后卫调用算法，模式1为边后卫调用算法
	extern CGeoPoint calcDefenderPointV2(const CGeoPoint Rtarget,const double Rdir,const posSide Rside,int mode = 0,double ratio=-1.0);

	// mode传入0代表左右后卫和单后卫算法，传入1代表边后卫调用算法，传入3代表中后卫算法
	extern CGeoPoint calcDefenderPointV3(const CGeoPoint& RTarget, double RDir, posSide RSide, int mode);

	extern CGeoPoint calcGoaliePointV2(const CGeoPoint Rtarget,const double Rdir,const posSide Rside,CGeoPoint laststoredpoint,int mode = 0);

	// mode控制守门员是否上前，1为一定不上前，0为不确定，还要考虑球的位置
	extern CGeoPoint calcGoaliePointV3(const CGeoPoint& RTarget, double RDir, posSide RSide, const CGeoPoint& lastpoint, int mode);

	extern CGeoPoint calcPenaltyLinePoint(const double dir, const posSide side,double ratio);
	/*ratio取0-1*/

  /**
   * @brief 判断球是否射向球门
   * @return 球在我方后场 禁区线之前 且射向我方球门则返回true
   */
	extern bool isBallShotToTheGoal();

	extern bool BallIsToPenaltyArea();

	extern bool inHalfCourt(CGeoPoint target,double dir,int sideFactor);

	extern bool getBallShooted();

	extern bool getEnemyPass();

	//防头球的点
	extern CGeoPoint getCornerAreaPos();
	//防开球的点
	extern CGeoPoint getIndirectDefender(double radius ,CGeoPoint leftUp ,CGeoPoint rightDown,int mode);

	//void clearKickOffDefArea();

	extern CGeoPoint getMiddleDefender(double bufferX);

	extern CGeoPoint getDefaultPos(int index);

	extern bool checkInDeadArea();

	extern void changeBasicData();

	extern double calcBalltoOurPenaty();
}
#endif
