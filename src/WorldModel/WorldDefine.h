#ifndef _WORLD_DEFINE_H_
#define _WORLD_DEFINE_H_
#include <geometry.h>
/************************************************************************/
/*                       ObjectPoseT                                    */
/************************************************************************/
class ObjectPoseT{
public:
	ObjectPoseT() : _valid(false), _pos(CGeoPoint(-9999,-9999)){ }
	const CGeoPoint& Pos() const { return _pos; }
	void SetPos(double x,double y) { _pos = CGeoPoint(x,y);	}
	void SetPos(const CGeoPoint& pos) { _pos = pos;	}
	double X() const { return _pos.x();	}
	double Y() const { return _pos.y();	}
	void SetVel(double x,double y) { _vel = CVector(x,y); }
	void SetVel(const CVector& vel) { _vel = vel; }
	const CVector& Vel() const { return _vel; }
	double VelX() const { return _vel.x(); }
	double VelY() const { return _vel.y(); }
	void SetValid(bool v) { _valid = v; }
	bool Valid() const { return _valid;	}
private:
	CGeoPoint _pos;
	CVector _vel;
	bool _valid;
};
/************************************************************************/
/*                      VisionObjectT                                   */
/************************************************************************/
class VisionObjectT{
public:
	VisionObjectT() : _rawPos(CGeoPoint(-9999,-9999)){ }
	const CGeoPoint& RawPos() const { return _rawPos; }
	void SetRawPos(double x, double y) { _rawPos = CGeoPoint(x,y); }
	void SetRawPos(const CGeoPoint& pos) { _rawPos = pos; }
private:
	CGeoPoint _rawPos; // 视觉的原始信息，没有经过预测
};
/************************************************************************/
/*                       MobileVisionT                                  */
/************************************************************************/
class MobileVisionT : public ObjectPoseT, public VisionObjectT{

};
/************************************************************************/
/*                        机器人姿态数据结构                              */
/************************************************************************/
struct PlayerPoseT : public ObjectPoseT{ // 目标信息
public:	
	PlayerPoseT() : _dir(0), _rotVel(0){ }
	double Dir() const { return _dir;}
	void SetDir(double d) { _dir = d; }
	double RotVel() const { return _rotVel; }
	void SetRotVel(double d){ _rotVel = d; }
private:
	double _dir; // 朝向
	double _rotVel; // 旋转速度
};
/************************************************************************/
/*                      PlayerTypeT                                     */
/************************************************************************/
class PlayerTypeT{
public:
	void SetType(int t) { _type = t; }
	int Type() const { return _type; }
private:
	int _type;
};
/************************************************************************/
/*                       PlayerVisionT                                  */
/************************************************************************/
class PlayerVisionT : public PlayerPoseT, public VisionObjectT, public PlayerTypeT{

};

/************************************************************************/
/*                        机器人能力数据结构                            */
/************************************************************************/
struct PlayerCapabilityT{
	double maxAccel; // 最大加速度
	double maxSpeed; // 最大速度
	double maxAngularAccel; // 最大角加速度
	double maxAngularSpeed; // 最大角速度
	double maxDec;          // 最大减速度
	double maxAngularDec;   // 最大角减速度
};
#endif // _WORLD_DEFINE_H_
