#ifndef _WORLD_MODEL_H_
#define _WORLD_MODEL_H_

#include "OptionModule.h"
#include "VisionModule.h"
#include <singleton.h>
#include "defence/EnemySituation.h"
#include "BallStatus.h"
#include "NormalPlayUtils.h"
#include "BallSpeedModel.h"
extern bool record_run_pos_on;
///
/// @file   WoldModel.h
/// @author Penghui Yin <cliffyin@zju.edu.cn>
/// @date   Sun Oct 22 15:26:36 2010
/// 
/// @brief  对于视觉模块的进一步处理，提供视觉的高层信息，协同视觉模块
///			进一步维护世界模型，其中包含以下几方面的内容：
///			1.基于基本数据的带参条件真值获取
///				见WorldModel_con.cpp，函数接口对外开放，方便条件获取
///         2.视觉模块数据预处理，组合计算得到一些基本数据
///				见WorldModel_basic.cpp，函数接口对外开放，方便调用
///			3.基于视觉信息的预测计算
///				见WorldModel_pre.cpp，函数接口对外开放，方便数据获取
///			4.基于视觉信息的各种高层算法，生成一些最优数据
///				见WorldModel_alg.cpp，函数接口对外开放，方便数据获取
///			5.基于视觉信息的工具函数
///				见WorldModel_utils.cpp，函数接口对外开放，方便数据获取


/************************************************************************/
/*							WorldModel.h                                */
/************************************************************************/

class CWorldModel{
public:
	CWorldModel() : _pVision(0), _pOption(0){ }
	void registerVision(const CVisionModule* pVision) { _pVision = pVision; }
	void registerOption(const COptionModule* pOption) { _pOption = pOption; }
	const CVisionModule* vision() const { return _pVision; }
	const COptionModule* option() const { return _pOption; }
private:
	const CVisionModule* _pVision;
	const COptionModule* _pOption;
	PlayerVisionT me;
	PlayerVisionT he;
	MobileVisionT ball;

/************************************************** WorldModel_con ***************************************************/
/// To be continue...
public:
/************************************************** WorldModel_basic ***************************************************/
/*
	1.球
	2.我方小车
	3.对方小车
	4.双方小车
	5.静态环境
	6. ... ...
*/
private:
	// 默认我方小车号
	const static int myDefaultNum;
	// 默认对方小车号
	const static int enemyDefaultNum;

	CEnemySituation enemySuation;


public:
	// 自己到球的矢量
	const CVector	self2ball					(int current_cycle, int myNum = myDefaultNum, int enemyNum = enemyDefaultNum);
	// 自己到球的距离
	const double	self2ballDist				(int current_cycle, int myNum = myDefaultNum, int enemyNum = enemyDefaultNum);
	// 自己到球的角度
	const double	self2ballDir				(int current_cycle, int myNum = myDefaultNum, int enemyNum = enemyDefaultNum);
	
/************************************************** WorldModel_utils ***************************************************/
/// To be continue...
	const string	CurrentRefereeMsg			();

public:

	// 给Lua的接口
	int		OurRobotNum();
	int		GetRealNum(int);
	int		GetTacticNum(int);
	bool	IsBallKicked(int);

	bool	IsBestPlayerChanged();
	string	CurrentBayes();
	bool    CanDefenceExit();
	// 能否在球的位置点射门
	
	//球是否漏过markingTouch区域
	bool NeedExitAttackDef(CGeoPoint leftUp, CGeoPoint rightDown, int mode);

	//球距离markingTouch车juli
	bool ball2MarkingTouch(CGeoPoint leftUp, CGeoPoint rightDown);
	
	//判断球是否朝markingFront车传
	bool isMarkingFrontValid(CGeoPoint checkPos, double checkDir);

	//判断开球车的数量
	int getEnemyKickOffNum();
	bool checkEnemyKickOffNumChanged();
	//返回需要markingTouch的区域，1是左边，2是右边
	int getMarkingTouchArea(CGeoPoint leftPos1,CGeoPoint leftPos2,CGeoPoint rightPos1,CGeoPoint rightPos2);
	void	SPlayFSMSwitchClearAll(bool clear_flag = false);
	const bool	KickDirArrived (int current_cycle, double kickdir, double kickdirprecision, int myNum = myDefaultNum);

	const bool canProtectBall(int current_cycle, int myNum = myDefaultNum);
	const bool canShootOnBallPos(int current_cycle, int myNum = myDefaultNum);
	const bool canPassOnBallPos(int current_cycle,CGeoPoint& passPos,CGeoPoint& guisePos, int myNum = myDefaultNum);
	const bool canKickAtEnemy(int current_cycle, CGeoPoint& kickDir,int myNum = myDefaultNum,int priority=0);
	const string getBallStatus(int current_cycle,int meNum=0);
	const string getAttackDecision(int current_cycle,int meNum = 0);
	const int getAttackerAmount(int current_cycle, int meNum = 0);
	const bool generateTwoPassPoint(CGeoPoint& passOne,CGeoPoint& passTwo){
		CGeoPoint predictBallPos=BallSpeedModel::Instance()->posForTime(15,_pVision);
		return NormalPlayUtils::generateTwoPassPoint(predictBallPos,passOne,passTwo);
	}
	const CGeoPoint getTandemPos(int myNum=myDefaultNum){
		CGeoPoint tandemPos=CGeoPoint(0,0);
		NormalPlayUtils::generateTandemCond(_pVision,tandemPos,myNum);
		return tandemPos;
	}
	const double getTandemDir(int myNum=myDefaultNum){
		CGeoPoint tandemPos=CGeoPoint(0,0);
		return NormalPlayUtils::generateTandemCond(_pVision,tandemPos,myNum);
	}
	const int getBallToucher(){
		return BallStatus::Instance()->getBallToucher();
	}
	const int getLastBallToucher() {
		return BallStatus::Instance()->getLastBallToucher();
	}
	const CGeoLine getBallChipLine() {
		return BallStatus::Instance()->ballChipLine();
	}
	const bool IsOurBallByAutoReferee(){
		return BallStatus::Instance()->getBallToucher()<6?false:true;
	}
	void clearBallStateCouter(){
		BallStatus::Instance()->clearBallStateCouter();
	}
	int getSuitSider(){
		return NormalPlayUtils::getSuitSider(_pVision);
	}
	void setBallHandler(int num=0);

	bool isRecordRunPos(){
		return record_run_pos_on;
	}

	// For FreeKick
	bool isBeingMarked(int myNum);
	bool isPassLineBlocked(int myNum, double allowance = 4.0);
	bool isPassLineBlocked(CGeoPoint pos, double allowance = 4.0);
	bool isShootLineBlocked(int myNum, double range = 150, double allowance = 3.0);
	bool isShootLineBlockedFromPoint(CGeoPoint pos, double range = 150);	
	bool CWorldModel::isLightKickBlocked(int myNum, double dir, double R, double halfAngle);
	void drawReflect(int myNum);
	const CGeoPoint getLeftReflectPos(int myNum);
	const CGeoPoint getRightReflectPos(int myNum);
	const CGeoPoint getReflectPos(int myNum);
	const CGeoPoint getReflectTouchPos(int myNum, double y);
	double getPointShootMaxAngle(CGeoPoint);
	bool isMarked(int);
};

typedef NormalSingleton< CWorldModel > WorldModel;

#define WORLD_MODEL WorldModel::Instance()

#endif // _WORLD_MODEL_H_
