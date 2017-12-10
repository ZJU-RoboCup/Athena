#ifndef _PATH_PLANNER_H_
#define _PATH_PLANNER_H_
#include <geometry.h>
#include <list>
#include "misc_types.h"
class CVisionModule;
// �ϰ�������
struct Obstacle{
	CGeoPoint pos; // �ϰ����λ��
	CVector   vel; // �ϰ�����ٶ�
	double    radius; // �ϰ���İ뾶
};
struct PathList : public std::list< CGeoPoint >{ // ·��
	PathList() : length(0){ }
    CGeoPoint  startNode;
	double length; // ·������
};
enum PlanType
{
	RRT = 0,
	GEO = 1,
	ASTAR = 2,
	STUPID = 3,
	BANGBANG = 4,
	PFAD = 5,
	DELAUNAY = 6,
	NEWGEO = 7,
};

// �ϰ���������
enum ObstacleMask{
	MASK_BALL = 0x0001,
	MASK_TEAMMATE0 = 0x0002,
	MASK_TEAMMATE1 = 0x0004,
	MASK_TEAMMATE2 = 0x0008,
	MASK_TEAMMATE3 = 0x0010,
	MASK_TEAMMATE4 = 0x0020,
	MASK_OPP0      = 0x0040,
	MASK_OPP1      = 0x0080,
	MASK_OPP2      = 0x0100,
	MASK_OPP3      = 0x0200,
	MASK_OPP4      = 0x0400
};

// ·���滮��ӿ�

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class	CPathPlanner{
///
/// @brief	����·���滮��ʵ�ֵĽӿ���,���캯����ʵ�ֹ滮��ͳһ�ӿ�.
///
/// @author	cliffyin(cliffyin@zju.edu.cn)
/// @date	2010-3-25
////////////////////////////////////////////////////////////////////////////////////////////////////

class CPathPlanner{
public:
	CPathPlanner(){ }
	CPathPlanner(const CVisionModule* pVision,const TaskT& task, const double avoidLength, const PlanType planType = RRT);
	const PathList& getPath() const { return _path; }
	bool isPathValid() const{ return !_path.empty(); }
	static void resetObstacleMask();
	static void setObstacleMask(int robotid, int mask_code);
	static int _obstacle_masks[5];
	CVector get_vel(){return _vel;};
protected:
	PathList _path;
	CVector _vel;
};
#endif // _PATH_PLANNER_H_
