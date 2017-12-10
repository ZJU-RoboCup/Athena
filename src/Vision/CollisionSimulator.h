#ifndef _COLLISION_SIMULATOR_H_
#define _COLLISION_SIMULATOR_H_
#include "WorldDefine.h"
/**
* ��ײģ����,���Ƴ��������ײ
*/
class CCollisionSimulator{
public:
	CCollisionSimulator() : _hasCollision(false){ }
	void reset(const CGeoPoint& ballRawPos, const CVector& ballVel);
	void simulate(PlayerVisionT robot, const double time); // ģ��һ��ʱ��
	const MobileVisionT& ball() const { return _ball; }
	bool hasCollision() const { return _hasCollision; }
private:
	MobileVisionT _ball; // �����Ϣ
	CVector _ballRelToRobot; // ���ڻ����˾ֲ�����ϵ�е�λ��
	bool _hasCollision; // �Ƿ���ײ
};
#endif // _COLLISION_SIMULATOR_H_