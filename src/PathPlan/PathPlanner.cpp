#include "PathPlanner.h"
//#include "AStarPathPlanner.h"
//#include "StupidPathPlanner.h"
#include "RRTPathPlanner.h"
//#include "BangPathPlanner.h"
//#include "PfadPathPlanner.h"
//#include "DelaunayPathPlanner.h"
//#include "NewGeoPathPlanner.h"

	int CPathPlanner::_obstacle_masks[5] = {0,0,0,0,0}; 

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	CPathPlanner::CPathPlanner(const CVisionModule* pVision, const int player,
/// 	const CGeoPoint& target, const int flags, const double avoidLength,
/// 	const PlanType planType)
///
/// @brief	Constructor. 
///
/// @author	cliffyin(cliffyin@zju.edu.cn)
/// @date	2010-3-25
///
/// @param	pVision		ָ���Ӿ�����ģ���ָ��.
/// @param	player		·���滮��Ե�С��,��С������λ��Ϊ�滮���.
/// @param	target		·���滮���յ�.
/// @param	flags		�����һЩ��ǩ.
/// @param	avoidLength	�����ĳ��ȣ�ָ����С������뾶֮��ľ���.
/// @param	planType	ѡ��Ĺ滮��ʽ,RRT GEO��,Ĭ�Ϸ�ʽ��GEO.
////////////////////////////////////////////////////////////////////////////////////////////////////

CPathPlanner::CPathPlanner(const CVisionModule* pVision, const TaskT& task, const double avoidLength, const PlanType planType)
{

	switch( planType )
	{
	case RRT:
		*this = CRRTPathPlanner(pVision,task,avoidLength);
		break;
	/*case GEO:
		*this = CGeoPathPlanner(pVision,task.executor,task.player.pos,task.player.flag,avoidLength);
		break;
	case ASTAR:
		*this = CAStarPathPlanner(pVision,task.executor,task.player.pos,task.player.flag,avoidLength);
	    break;
	case STUPID:
		*this = CStupidPathPlanner(pVision,task.executor,task.player.pos,task.player.flag,avoidLength);
	    break;
	case BANGBANG:
		*this = CBangPathPlanner(pVision,task.executor,task.player.pos,task.player.flag,avoidLength);
		break;
	case PFAD:
		*this = CPfadPathPlanner(pVision,task.executor,task.player.pos,task.player.flag,avoidLength);
		break;
	case DELAUNAY:
		*this = CDelaunayPathPlanner(pVision,task.executor,task.player.pos,task.player.flag,avoidLength);
		break;	
	case NEWGEO:
		*this = CNewGeoPathPlanner(pVision,task.executor,task.player.pos,task.player.flag,avoidLength);
		break;	*/

	default:
		printf("Default PathPlanner!\n");
		*this = CRRTPathPlanner(pVision,task,avoidLength);
	    break;
	}
	//*this = realPlanner;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void CPathPlanner::resetObstacleMask()
///
/// @brief	�����ϰ�����������
///
/// @author	cliffyin(cliffyin@zju.edu.cn)
/// @date	2010-3-25
////////////////////////////////////////////////////////////////////////////////////////////////////

void CPathPlanner::resetObstacleMask()
{
	for (int i=0; i<5; i++)
		_obstacle_masks[i] = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void CPathPlanner::setObstacleMask(int robotid, int mask_code)
///
/// @brief	����һ��������ָ����С��������������. 
///
/// @author	Cliffyin(cliffyin Zju.edu.cn)
/// @date	2010-3-25
///
/// @param	robotid		С���ĳ���(0-4). 
/// @param	mask_code	��������. 
////////////////////////////////////////////////////////////////////////////////////////////////////

void CPathPlanner::setObstacleMask(int robotid, int mask_code)
{
	_obstacle_masks[robotid-1] |= mask_code;
}
