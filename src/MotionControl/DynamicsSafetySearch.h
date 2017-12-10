#ifndef _DYNAMICS_SAFETY_SEARCH_H_
#define _DYNAMICS_SAFETY_SEARCH_H_
#include <VisionModule.h>
#include <GDebugEngine.h>
#include <RobotCapability.h>
#include <TimeCounter.h>
#include "cmu/vector.h"
#include <math.h>
#include <utils.h>
#include <geometry.h>
#include <singleton.h>
#include "QuadraticEquation.h"
#include "CubicEquation.h"
#include "QuarticEquation.h"
/************************************************************************/
/* �켣�ࣺ����С�������й켣�������ж��������켣֮���Ƿ���ײ				 
/************************************************************************/
class Trajectory {
public: 
	Trajectory(){_qlength = 0; _limitTime = EPSILON;}
	~Trajectory(){}
	void MakeTrajectory(const int player,const vector2f pos,const vector2f vel, const vector2f acc,
						const float t0,const float tc,const float tend,
						const float Radius,const float DMAX, const int type);
	bool CheckTrajectory(const Trajectory& T1);
	CQuadraticEquation q[3]; // ���η���ʽ��ʵ��
	int _qlength;
	int _type; //С��������
	int _carnum;
	float Radius;
	float _limitTime;
	float _tend;
};
/************************************************************************/
/* ��̬��ȫ�����࣬��鵱ǰ�����£�С���Ƿ�����ϰ�����ײ�������ײ�������
   һ�����ŵİ�ȫ���ٶ�
/************************************************************************/
class CDynamicSafetySearch {
public:
	CDynamicSafetySearch();
	~CDynamicSafetySearch() {}
	CVector SafetySearch(const int player,CVector Vnext, const CVisionModule* pVision, 
						 const int t_priority, const CGeoPoint target, 
						 const int flags,const float stoptime, double max_acc);
private:
	bool CheckAccel(const int player, vector2f acc,const CVisionModule* pVision, 
					vector2f& crash_car_dir, int& crash_car_num, int& type,const float limitTime);
	float Evaluation(const vector2f tempacc, const int player);
	bool CheckRobot(const int player, vector2f pi, vector2f vi,vector2f Ai, 
					const int obstacle, vector2f pj, vector2f vj, vector2f Aj,int type,const float limitTime);
	vector2f RandomAcc(float start_angle,float end_angle,float max_radius); //�ڼ��ٶȿռ������Ѱ�Ҽ��ٶ�
	vector2f OutOfTrouble(const int player, vector2f pi, vector2f dir,float dist,const CVisionModule* pVision); //����������
	vector2f CVector2vector2f(const CVector vec1);
	vector2f CGeoPoint2vector2f(const CGeoPoint pos);
	CVector vector2f2CVector(const vector2f vec1);
	CGeoPoint vector2f2CGeoPoint(const vector2f pos1);
	void refresh_priority(const int player,const int t_priority, const vector2f target, const vector2f pos);

	float _e;             // Ȩֵ
	float _gamma;         // С������ʻʱ��
	vector2f _pos;        // С����ǰ��λ��
	vector2f _vel;        // С����ǰ���ٶ�
	vector2f _nvel;       // С����һʱ�̵��ٶ�
	vector2f _acc;        // С���ļ��ٶ�
	vector2f _t_acc;      // ��ʱ���ٶ�
	vector2f last_acc[6]; // ����С��
	float last_e[6];      // �����ϴε����ŵ�����
	int priority[6];      // �ж�С�����ϵ����ȼ�
	bool find_flag[6];    // �Ƿ�����ҽ�
	float _stoptime;
	float _DECMAX;          // С�����ļ��ٶ�
	float _AECMAX;          // С�����ļ��ٶ�
	float _VAMX;          // С�������ٶ�
	float _C;             // ����ִ��һ�����ڵ�ʱ��
	int _CNum;            // Ԥ����ٸ����ں������
	int _flag;
};
typedef NormalSingleton< CDynamicSafetySearch > DynamicSafetySearch;
#endif 