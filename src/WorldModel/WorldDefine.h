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
	CGeoPoint _rawPos; // �Ӿ���ԭʼ��Ϣ��û�о���Ԥ��
};
/************************************************************************/
/*                       MobileVisionT                                  */
/************************************************************************/
class MobileVisionT : public ObjectPoseT, public VisionObjectT{

};
/************************************************************************/
/*                        ��������̬���ݽṹ                              */
/************************************************************************/
struct PlayerPoseT : public ObjectPoseT{ // Ŀ����Ϣ
public:	
	PlayerPoseT() : _dir(0), _rotVel(0){ }
	double Dir() const { return _dir;}
	void SetDir(double d) { _dir = d; }
	double RotVel() const { return _rotVel; }
	void SetRotVel(double d){ _rotVel = d; }
private:
	double _dir; // ����
	double _rotVel; // ��ת�ٶ�
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
/*                        �������������ݽṹ                            */
/************************************************************************/
struct PlayerCapabilityT{
	double maxAccel; // �����ٶ�
	double maxSpeed; // ����ٶ�
	double maxAngularAccel; // ���Ǽ��ٶ�
	double maxAngularSpeed; // �����ٶ�
	double maxDec;          // �����ٶ�
	double maxAngularDec;   // ���Ǽ��ٶ�
};
#endif // _WORLD_DEFINE_H_
