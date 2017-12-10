#ifndef _ROBOT_CAPABILITY_H_
#define _ROBOT_CAPABILITY_H_
#include <geometry.h>
#include <param.h>
#include <singleton.h>
#include <vector>
/**
* ���������˵�������.
*/
class CRobotCapability{
public:
	// ��������
	virtual double minShootAngleWidth() const = 0; // ��С���ŽǶȷ�Χ

	// ��������
	virtual double minChipKickObstacleDist() const = 0; // �����ϰ����������
	virtual double maxChipKickDist() const = 0; // ��Զ�������
	virtual double maxChipShootDist() const = 0; // ����������
	virtual double minChipShootExtra(const double dist) const = 0; // �������Ź����Ķ������
	
	virtual double maxSpeedGoalie(const double angle) const = 0;
	virtual double maxAccelerationGoalie(const CVector& velocity, const double angle) const = 0;
	virtual double maxDecGoalie(const CVector& velocity, const double angle) const = 0;

	virtual double maxSpeedBack(const double angle) const = 0;
	virtual double maxAccelerationBack(const CVector& velocity, const double angle) const = 0;
	virtual double maxDecBack(const CVector& velocity, const double angle) const = 0;

	// �˶�����
	virtual double maxAcceleration(const CVector& velocity, const double angle) const = 0; // ĳ������������ٶ�
	virtual double maxAngularAcceleration() const = 0;
  virtual double maxDec(const CVector& velocity, const double angle) const = 0; // ĳ������������ٶ�
  virtual double maxAngularDec() const = 0;  //   z
	virtual double maxSpeed(const double angle) const = 0; // ĳ�����������ٶ�
	virtual double maxAngularSpeed() const = 0; // �����ת�ٶ�
	
	virtual ~CRobotCapability(){ } // ����������
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
typedef NormalSingleton< CRobotCapFactory > RobotCapFactory; // ����ΪSingleton
#endif // _ROBOT_CAPABILITY_H_