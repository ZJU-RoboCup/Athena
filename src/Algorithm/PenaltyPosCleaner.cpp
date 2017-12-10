#include "PenaltyPosCleaner.h"
#include "param.h"
#include "utils.h"
#include <algorithm>
#include <math.h>
#include "PlayInterface.h"
#include "TaskMediator.h"
#ifdef _DEBUG
#include "GDebugEngine.h"
#endif
namespace{
    CGeoPoint OUR_GOAL_CENTER;
	const double IMPACT_DIST = Param::Vehicle::V2::PLAYER_SIZE * 2 + 1;
	const double REAL_POS_CHECK_LIMIT = 25;//当真实车位置到到IMPACT_DIST+REAL_POS_CHECK_LIMIT内部时才认为碰撞
	double distBuffer = 1.0;
	double HALF_ANGLE = asin(8.0/90);
}

CPenaltyPosCleaner::CPenaltyPosCleaner(){
	_clear = false;
	_reset = true;
	_inputNum = 0;
	_calcTimes = 0;
	_oriPoints = new SPointVector[Param::Field::MAX_PLAYER];
	_impactPoints = new SIPointVector[Param::Field::MAX_PLAYER];
	_groupStatus = new GroupStatus;
	_oriPoints->clear();
	_impactPoints->clear();
	_groupStatus->clear();
	OUR_GOAL_CENTER = CGeoPoint(-Param::Field::PITCH_LENGTH/2.0,0);
}

CPenaltyPosCleaner::~CPenaltyPosCleaner()
{
	delete _oriPoints;
	delete _impactPoints;
}

bool CPenaltyPosCleaner::add(const string roleName,const int num,const CGeoPoint point){
	if (true == _reset)
	{
		_reset = false;
		_oriPoints->clear();
		_inputNum = 0;
	}
#ifdef _DEBUG
	for (SPointVector::iterator it = _oriPoints->begin();it != _oriPoints->end();++it)
	{
		if (roleName == it->_roleName)
		{
			cout << roleName << " ---- "<< "ERROR: Penalty Pos Cleaner: role name repeated!!" << endl;
			break;
			return false;
		}
		if (num == it->_num)
		{
			if (num != 0) {
				cout << num << " ---- " << "ERROR: Penalty Pos Cleaner: vehicle num repeated!!" << endl;
			}
			break;
			return false;
		}
	}
#endif	
	_oriPoints->push_back(SPoint(roleName,num,point,calcAngle(point)));
	_inputNum++;
	if (_inputNum >= Param::Field::MAX_PLAYER)//门将不匹配
	{
		cout << "Error: Penalty Pos Cleaner: roleNum OverFlew " <<endl;
		return false;
	}
	return true;
}

CGeoPoint CPenaltyPosCleaner::get(const string roleName){
	for(SPointVector::iterator it = _oriPoints->begin();it != _oriPoints->end();++it)
	{
		if (roleName == it->_roleName)
		{
			return it->_point;
		}
	}
	cout<<"ERROR: Penalty Pos Cleaner : there is no role called " << roleName <<" !!" <<endl;
	return CGeoPoint(-200,0);
}


void CPenaltyPosCleaner::clean(const CVisionModule* pVision){
	//1.首次过滤，将离禁区线远的点排除在外
	_impactPoints->clear();
	_calcTimes = 0;
	for (SPointVector::iterator it = _oriPoints->begin();it != _oriPoints->end();++it)
	{
		if (DefendUtils::D_ELLIPSE1.HasPoint(it->_point))
		{
			_impactPoints->push_back(SIPoint(it->_roleName,it->_num,it->_point,it->_angle));
		}
	}
	//2.排序
	std::sort(_impactPoints->begin(),_impactPoints->end());
	//3.calc
	if (true == calc(pVision))
	{
		//将所有的点按角色赋值
		for (SIPointVector::iterator it = _impactPoints->begin();it != _impactPoints->end();++it)
		{
			for (SPointVector::iterator it2 = _oriPoints->begin();it2 != _oriPoints->end();++it2)
			{
				if (it2->_roleName == it->_roleName)
				{
					it2->_num = it->_num;
					it2->_point = it->_point;
					it2->_angle = it->_angle;
					break;
				}
			}
		}
	}
	_reset = true;
}

bool CPenaltyPosCleaner::calc(const CVisionModule* pVision)
{
	_calcTimes++;
	//检测
	if (false == checkImpact(pVision))
	{
		if (1 == _calcTimes)
		{
			_groupStatus->clear();
		}
		return true;
	}
	//计算
	calcPoint();
	return calc(pVision);
}

bool CPenaltyPosCleaner::checkImpact(const CVisionModule* pVision)
{
	bool impact = false;
	for (SIPointVector::iterator it = _impactPoints->begin();it != _impactPoints->end();++it)
	{
		for (SIPointVector::iterator it2 = it+1;it2 != _impactPoints->end();++it2)
		{
			if ("NULL" == it2->_groupName)// 如果it2无组别
			{				
				if (checkPosImpact(pVision,(*it),(*it2)))//检测是否碰撞
				{
					impact = true;
					if ("NULL" == it->_groupName)//如果it也无组别，则设置组别名为it的角色名
					{
						it->_groupName = it->_roleName;
					} 
					it2->_groupName = it->_groupName;
				}
			}else{//如果it2有组别
				if ("NULL" == it->_groupName)//如果it无组别
				{
					if (checkPosImpact(pVision,(*it),(*it2)))//检测碰撞
					{
						impact = true;
						it->_groupName = it2->_groupName;//将it加入it2组
					}
				} else if (it->_groupName != it2->_groupName)//it有组别，it2有组别，而且两者不是同一组
				{
					if (checkPosImpact(pVision,(*it),(*it2)))//发生了碰撞,it2全组加入it组
					{
						impact = true;
						string tempGroupName = it2->_groupName;
						for (SIPointVector::iterator it3 = _impactPoints->begin();it3 != _impactPoints->end();++it3)
						{
							if (it3->_groupName == tempGroupName)
							{
								it3->_groupName = it->_groupName;
							}
						}
					}
				}
			}
		}			
	}
	return impact;
}

double CPenaltyPosCleaner::calcAngle(CGeoPoint point)
{
	return CVector(point-OUR_GOAL_CENTER).dir();
}

bool CPenaltyPosCleaner::checkPosImpact(const CVisionModule* pVision,const SIPoint& p1,const SIPoint& p2)
{
	if (p1._point.dist(p2._point) < IMPACT_DIST)
	{
		if (p1._num != 0 && p2._num != 0)
		{
			CGeoPoint RealP1 = pVision->OurPlayer(p1._num).Pos();
			CGeoPoint RealP2 = pVision->OurPlayer(p2._num).Pos();
			if (RealP1.dist(RealP2) < IMPACT_DIST + REAL_POS_CHECK_LIMIT)
			{
				return true;
			}
		}			
	}
	return false;
}

void CPenaltyPosCleaner::calcPoint()
{
	_groupStatus->clear();
	//组别数据统计
	for (SIPointVector::iterator it = _impactPoints->begin();it != _impactPoints->end();++it)
	{
		if ("NULL" != it->_groupName)
		{
			GroupStatus::iterator se = _groupStatus->find(it->_groupName);
			if (_groupStatus->end() == se)//completeGroup中没有该组
			{
				_groupStatus->insert(GroupStatus::value_type(it->_groupName,SGroup(it->_roleName)));
			}
			else {
				se->second._num += 1;
				se->second._names.push_back(it->_roleName);
			}
		}
	}
	//根据组别数据统计计算具体的点
	for (GroupStatus::iterator it = _groupStatus->begin(); it != _groupStatus->end();++it)
	{
		double leftAngle = 0,rightAngle = 0,midAngle = 0;
		int num = 1;
		double useAngle = 0;
		posSide side = POS_SIDE_LEFT;
		for (SIPointVector::iterator it2 = _impactPoints->begin(); it2 != _impactPoints->end();++it2)
		{
			if (it2->_groupName == it->first)
			{
				midAngle += it2->_angle;
				if (num++ > it->second._num)
				{
					break;
				}
			}
		}
		//update HALF_ANGLE
		HALF_ANGLE = asin((9.0-distBuffer)/90); 
		//calc angle
		midAngle = midAngle/it->second._num;
		leftAngle = midAngle - it->second._num * HALF_ANGLE;
		leftAngle = leftAngle < -Param::Math::PI/2+HALF_ANGLE ? -Param::Math::PI/2+HALF_ANGLE : leftAngle;
		leftAngle = leftAngle > Param::Math::PI/2-2*HALF_ANGLE ? Param::Math::PI/2-2*HALF_ANGLE : leftAngle;
		rightAngle = midAngle + it->second._num * HALF_ANGLE;
		rightAngle = rightAngle > Param::Math::PI/2 ? Param::Math::PI/2 : rightAngle;
		rightAngle = rightAngle < -Param::Math::PI/2+2*HALF_ANGLE ? -Param::Math::PI/2+2*HALF_ANGLE : rightAngle;
#ifdef _DEBUG
		GDebugEngine::Instance()->gui_debug_line(OUR_GOAL_CENTER,OUR_GOAL_CENTER+Utils::Polar2Vector(500,leftAngle),COLOR_YELLOW);
		GDebugEngine::Instance()->gui_debug_line(OUR_GOAL_CENTER,OUR_GOAL_CENTER+Utils::Polar2Vector(500,rightAngle),COLOR_YELLOW);
#endif
		if (rightAngle < Param::Math::PI / 2 - 0.01)
		{
			useAngle = leftAngle;
			side = POS_SIDE_RIGHT;//采用右边的线，则向左算
		} else {
			useAngle = rightAngle;
			side = POS_SIDE_LEFT;
		}
		//calc points
		for (SIPointVector::iterator it2 = _impactPoints->begin(); it2 != _impactPoints->end();++it2)
		{
			if (it2->_groupName == it->first)
			{
				it2->_point = DefendUtils::calcPenaltyLinePoint(useAngle,side,1.0);//为防止进禁区不使用distBuffer，直接写入1.0
				it2->_angle = calcAngle(it2->_point);
				useAngle = calcAngleShift(useAngle,side);
			}
		}
	}
}

double CPenaltyPosCleaner::calcAngleShift(double angle,posSide side)
{
	//先这样简单处理，以后可以变为calcAngleBlock(CGeoPoint point,posSide side)
	if (POS_SIDE_LEFT == side)
	{
		return angle - 2 * HALF_ANGLE;
	}else if (POS_SIDE_RIGHT == side)
	{
		return angle + 2 * HALF_ANGLE;
	}
	return angle;
}
