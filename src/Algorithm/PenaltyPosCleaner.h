#ifndef __PENALTY_POS_CLEANER_H__
#define __PENALTY_POS_CLEANER_H__

#include "singleton.h"
#include <string>
#include <vector>
#include <map>
#include "geometry.h"
#include "DefendUtils.h"
#include "VisionModule.h"
using namespace std;
//struct point 原始点,原始点每次reset后更新一次
struct SPoint{
	SPoint():_roleName("NULL"),_num(0),_point(CGeoPoint(0,0)),_angle(0.0){}
	SPoint(const string roleName,const int num,const CGeoPoint point,const double angle):
	_roleName(roleName),_num(num),_point(point),_angle(angle){}
	string _roleName;
	int _num;
	CGeoPoint _point;
	double _angle;
};

//struct impact point发生碰撞的点,构建该点的数组，同时维护一个ImpactGroup
struct SIPoint{
	SIPoint():_roleName("NULL"),_num(0),_groupName("NULL"),_point(CGeoPoint(0,0)),_angle(0.0){}
	SIPoint(const string roleName,const int num,const CGeoPoint point,const double angle):
	_roleName(roleName),_num(num),_groupName("NULL"),_point(point),_angle(angle){}
	bool operator < (const SIPoint& p) const {
		if (_angle < p._angle)
		{
			return true;
		}
		return false;
	}
	string _roleName;
	int _num;
	string _groupName;
	CGeoPoint _point;
	double _angle;
};

struct SGroup{
	SGroup(string name):_num(1){_names.push_back(name);}
	int _num;
	vector<string> _names;
};

typedef vector<SPoint> SPointVector;
typedef vector<SIPoint> SIPointVector;
typedef map<string,SGroup> GroupStatus;

class CPenaltyPosCleaner
{
public:
	CPenaltyPosCleaner();
	~CPenaltyPosCleaner();

	//outter interface
	bool add(const string roleName,const int num,const CGeoPoint point);//返回是否大于MaxPlayer
	CGeoPoint get(const string roleName);
	void clean(const CVisionModule* pVision);
	GroupStatus* getGroupStatus(){return _groupStatus;}
	SIPointVector* getImpactVector(){return _impactPoints;}
protected:
	//main calc func
	bool calc(const CVisionModule* pVision);
	//protected func
	bool checkImpact(const CVisionModule* pVision);
	double calcAngleShift(double angle,posSide side);
	double calcAngle(CGeoPoint point);
	bool checkPosImpact(const CVisionModule* pVision,const SIPoint& p1,const SIPoint& p2);
	void calcPoint();
private:
	bool _clear;//是否完成计算
	bool _reset;//每帧重新开始的标志
	int _inputNum;//输入点的数量
	int _calcTimes; //迭代计算次数
	SPointVector* _oriPoints;
	SIPointVector* _impactPoints;
	GroupStatus* _groupStatus;
};

typedef NormalSingleton<CPenaltyPosCleaner> PenaltyPosCleaner;

#endif
