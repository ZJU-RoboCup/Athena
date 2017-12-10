#ifndef _ROBOT_CAPABILITY_H_
#define _ROBOT_CAPABILITY_H_
#include <geometry.h>
#include <param.h>
#include <singleton.h>
#include <vector>
/**
* 描述机器人的能力类.
*/
class CRobotCapability{
public:
	// 击球能力
	virtual double minShootAngleWidth() const = 0; // 最小射门角度范围

	// 挑球能力
	virtual double minChipKickObstacleDist() const = 0; // 挑球障碍物最近距离
	virtual double maxChipKickDist() const = 0; // 最远挑球距离
	virtual double maxChipShootDist() const = 0; // 最大挑射距离
	virtual double minChipShootExtra(const double dist) const = 0; // 挑球射门过顶的额外距离
	
	virtual double maxSpeedGoalie(const double angle) const = 0;
	virtual double maxAccelerationGoalie(const CVector& velocity, const double angle) const = 0;
	virtual double maxDecGoalie(const CVector& velocity, const double angle) const = 0;

	virtual double maxSpeedBack(const double angle) const = 0;
	virtual double maxAccelerationBack(const CVector& velocity, const double angle) const = 0;
	virtual double maxDecBack(const CVector& velocity, const double angle) const = 0;

	// 运动能力
	virtual double maxAcceleration(const CVector& velocity, const double angle) const = 0; // 某个方向的最大加速度
	virtual double maxAngularAcceleration() const = 0;
  virtual double maxDec(const CVector& velocity, const double angle) const = 0; // 某个方向的最大减速度
  virtual double maxAngularDec() const = 0;  //   z
	virtual double maxSpeed(const double angle) const = 0; // 某个方向的最大速度
	virtual double maxAngularSpeed() const = 0; // 最大旋转速度
	
	virtual ~CRobotCapability(){ } // 虚析构函数
};
class CRobotCapFactory{
public:
	CRobotCapFactory();
	const CRobotCapability* getRobotCap(const int side, const int num)
	{
		if( side == Param::Field::POS_SIDE_LEFT ){
			return _robotCaps[LeftRobotType[num]];
		}
		return _robotCaps[RightRobotType[num]];
	}
	~CRobotCapFactory()
	{
		for( int i=0; i<_robotCaps.size(); ++i ){
			delete _robotCaps[i];
		}
	}
private:
	std::vector< CRobotCapability* > _robotCaps;
	int LeftRobotType[Param::Field::MAX_PLAYER+1];
	int RightRobotType[Param::Field::MAX_PLAYER+1];
};
typedef NormalSingleton< CRobotCapFactory > RobotCapFactory; // 声明为Singleton
#endif // _ROBOT_CAPABILITY_H_