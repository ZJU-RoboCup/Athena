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
/// @brief  �����Ӿ�ģ��Ľ�һ�������ṩ�Ӿ��ĸ߲���Ϣ��Эͬ�Ӿ�ģ��
///			��һ��ά������ģ�ͣ����а������¼���������ݣ�
///			1.���ڻ������ݵĴ���������ֵ��ȡ
///				��WorldModel_con.cpp�������ӿڶ��⿪�ţ�����������ȡ
///         2.�Ӿ�ģ������Ԥ������ϼ���õ�һЩ��������
///				��WorldModel_basic.cpp�������ӿڶ��⿪�ţ��������
///			3.�����Ӿ���Ϣ��Ԥ�����
///				��WorldModel_pre.cpp�������ӿڶ��⿪�ţ��������ݻ�ȡ
///			4.�����Ӿ���Ϣ�ĸ��ָ߲��㷨������һЩ��������
///				��WorldModel_alg.cpp�������ӿڶ��⿪�ţ��������ݻ�ȡ
///			5.�����Ӿ���Ϣ�Ĺ��ߺ���
///				��WorldModel_utils.cpp�������ӿڶ��⿪�ţ��������ݻ�ȡ


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
	1.��
	2.�ҷ�С��
	3.�Է�С��
	4.˫��С��
	5.��̬����
	6. ... ...
*/
private:
	// Ĭ���ҷ�С����
	const static int myDefaultNum;
	// Ĭ�϶Է�С����
	const static int enemyDefaultNum;

	CEnemySituation enemySuation;


public:
	// �Լ������ʸ��
	const CVector	self2ball					(int current_cycle, int myNum = myDefaultNum, int enemyNum = enemyDefaultNum);
	// �Լ�����ľ���
	const double	self2ballDist				(int current_cycle, int myNum = myDefaultNum, int enemyNum = enemyDefaultNum);
	// �Լ�����ĽǶ�
	const double	self2ballDir				(int current_cycle, int myNum = myDefaultNum, int enemyNum = enemyDefaultNum);
	
/************************************************** WorldModel_utils ***************************************************/
/// To be continue...
	const string	CurrentRefereeMsg			();

public:

	// ��Lua�Ľӿ�
	int		OurRobotNum();
	int		GetRealNum(int);
	int		GetTacticNum(int);
	bool	IsBallKicked(int);

	bool	IsBestPlayerChanged();
	string	CurrentBayes();
	bool    CanDefenceExit();
	// �ܷ������λ�õ�����
	
	//���Ƿ�©��markingTouch����
	bool NeedExitAttackDef(CGeoPoint leftUp, CGeoPoint rightDown, int mode);

	//�����markingTouch��juli
	bool ball2MarkingTouch(CGeoPoint leftUp, CGeoPoint rightDown);
	
	//�ж����Ƿ�markingFront����
	bool isMarkingFrontValid(CGeoPoint checkPos, double checkDir);

	//�жϿ��򳵵�����
	int getEnemyKickOffNum();
	bool checkEnemyKickOffNumChanged();
	//������ҪmarkingTouch������1����ߣ�2���ұ�
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
