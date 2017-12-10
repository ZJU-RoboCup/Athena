#ifndef _KICK_DIRECTION_
#define _KICK_DIRECTION_

#include <VisionModule.h>

#define NEW_COMPENSATE	true

class CVisionModule;
/**
* CKickDirection.
*1. �Ծ��г��ٶȵ�������һ����С��Ϊ���ĵĶ�����ǶȵĲ�����
* 
* player ָ��Ա��(ֻ��Ϊ��ȡ��robotCap,��С������һ��,����ȡ1~6ֵ)
* pos ����ֱ�Ӽ���ĳ�㴦(���������λ��)�����ſյ�
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
//svm�����㲹���Ƕ�
	double calCompensate(double x, double y);
	double calGussiFuncA(double x1,double y1,double x2,double y2);
	double calGussiFuncB(double x1,double y1,double x2,double y2);
// --------------------ftq-----------------
	double getInSpeed(){return shoot_speed;}
	double getInDir(){return shoot_dir;}

	void getData(double shoot_vel, double shoot_dir, double outdir);

private:
	void reset();

	bool  _kick_valid;			// �ж��ܷ����򣬼�·���Ƿ���ȫ���
	bool  _is_compensated;		// С�������Ƿ���Ҫ����
	double _raw_kick_dir;		// ϣ���򴫳��ĽǶȣ��������ŵ���ѽǶ�
	double _compensate_value;	// �����Ƕ�
	double _real_kick_dir;		// �����ǶȲ�����С��ʵ����Ҫ�ڳ��ĽǶ�
	double rawdir;//ԭʼ�������������ߺͳ���Ŀ���ļн�

	//----------------------------------------------------------------------------------
	double outdir; // ʵ�ʵ�����Ƕ�
	double shoot_speed;
	double shoot_dir;
	//----------------------------------------------------------------------------------
	// added by FTQ

	CGeoPoint _kick_target;		// ��Ҫ�ߵ��ĵ�
	double _compensate_factor;  // �����ٶȵ����Ĳ�������,Ԥ������
};

typedef NormalSingleton< CKickDirection > KickDirection;

#endif // _KICK_DIRECTION_