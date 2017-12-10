#ifndef _KICK_DIRECTION_
#define _KICK_DIRECTION_

#include <VisionModule.h>

#define NEW_COMPENSATE	true

class CVisionModule;
/**
* CKickDirection.
*1. 对具有初速度的球，做出一个以小车为中心的对踢球角度的补偿。
* 
* player 指队员号(只是为了取得robotCap,若小车性能一样,可任取1~6值)
* pos 用于直接计算某点处(而不是球的位置)的射门空挡
*/

class CKickDirection{
public:
	CKickDirection(); 
	void GenerateShootDir(const int player , const CGeoPoint pos, bool showFlag = 0);
	bool getKickValid ()  {return _kick_valid;}
	bool getCompensate () {return _is_compensated;}
	double getRawKickDir () {return _raw_kick_dir;}
	double getCompensateDir () {return _compensate_value;}
	double getRealKickDir () {return _real_kick_dir;}
	double getRawDir(){return rawdir;}
//svm法计算补偿角度
	double calCompensate(double x, double y);
	double calGussiFuncA(double x1,double y1,double x2,double y2);
	double calGussiFuncB(double x1,double y1,double x2,double y2);
// --------------------ftq-----------------
	double getInSpeed(){return shoot_speed;}
	double getInDir(){return shoot_dir;}

	void getData(double shoot_vel, double shoot_dir, double outdir);

private:
	void reset();

	bool  _kick_valid;			// 判断能否踢球，即路线是否被完全封堵
	bool  _is_compensated;		// 小车方向是否需要补偿
	double _raw_kick_dir;		// 希望球传出的角度，或者射门的最佳角度
	double _compensate_value;	// 补偿角度
	double _real_kick_dir;		// 经过角度补偿，小车实际需要摆出的角度
	double rawdir;//原始待补偿的球速线和车到目标点的夹角

	//----------------------------------------------------------------------------------
	double outdir; // 实际的射出角度
	double shoot_speed;
	double shoot_dir;
	//----------------------------------------------------------------------------------
	// added by FTQ

	CGeoPoint _kick_target;		// 需要踢到的点
	double _compensate_factor;  // 根据速度调整的补偿因子,预留讨论
};

typedef NormalSingleton< CKickDirection > KickDirection;

#endif // _KICK_DIRECTION_