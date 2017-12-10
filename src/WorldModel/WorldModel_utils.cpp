#include "WorldModel.h"
#include "ValueRange.h"
#include "GDebugEngine.h"
/************************************************************************/
/* 命名空间                                                             */
/************************************************************************/
namespace {
	///> 关于踢球
	// 控球控制阈值
	const double Allow_Start_Kick_Dist = 5*Param::Vehicle::V2::PLAYER_SIZE;
	const double Allow_Start_Kick_Angle = 2.5 * Param::Vehicle::V2::KICK_ANGLE;
	static bool IS_SIMULATION = false;
	// 稳定方向，提高射门精度
	const double beta = 1.0;
	const int critical_cycle = 10;
	int kick_last_cycle = 0;
	const int kick_stable_cnt[Param::Field::MAX_PLAYER + 1] = { 1,1,1,1,1,1,1 };     //小嘴巴时需要小一点
	int aim_count[Param::Field::MAX_PLAYER + 1] = {0,0,0,0,0,0,0};
	int aim_cycle[Param::Field::MAX_PLAYER + 1] = {0,0,0,0,0,0,0};
	// 判断辅助函数: 用于判断角度是否有调整好,采用CMU的判断方式
	bool deltaMarginKickingMetric(int current_cycle, double gt, double delta, double mydir, int myNum)
	{
		// 如果是以前的数据，需要进行清空
		if (current_cycle - aim_cycle[myNum] > critical_cycle) {
			aim_count[myNum] = 0;
		}
		aim_cycle[myNum] = current_cycle;

		// 计算当前的margin并做记录
		double gl = Utils::Normalize(gt - delta);	// 目标方向向左偏置射门精度阈值
		double gr = Utils::Normalize(gt + delta);	// 目标方向向右偏置射门精度阈值
		double current_margin = max(Utils::Normalize(mydir - gl), Utils::Normalize(gr - mydir));

		// 根据margin判断是否达到踢球精度
		bool kick_or_not = false;
		if (current_margin > 0 && current_margin < beta*2*delta) {
			if (aim_count[myNum]++ >= kick_stable_cnt[myNum]) {
				kick_or_not = true;
				aim_count[myNum] = kick_stable_cnt[myNum];
			}
		} else {
			aim_count[myNum] --;
			if (aim_count[myNum] < 0) {
				aim_count[myNum] = 0;
			}
		}

		return kick_or_not;
	}

	///> 关于控球
	// 控球控制阈值
	const bool Allow_Start_Dribble = true;
	const double Allow_Start_Dribble_Dist = 6.0 * Param::Vehicle::V2::PLAYER_SIZE;
	const double Allow_Start_Dribble_Angle = 5.0 * Param::Vehicle::V2::KICK_ANGLE;

}

///> 【#】 射门时朝向是否调整到位，含缓冲
const bool  CWorldModel::KickDirArrived(int current_cycle, double kickdir, double kickdirprecision, int myNum) {
	static int last_cycle[Param::Field::MAX_PLAYER] = {-1,-1,-1,-1,-1,-1};
	static bool _dir_arrived[Param::Field::MAX_PLAYER] = {false,false,false,false,false,false};

	// 重置
	kick_last_cycle = current_cycle;

	// 计算
	//if (last_cycle[myNum-1] < current_cycle) {
		const PlayerVisionT& me = this->_pVision->OurPlayer(myNum);
		// 球在身前 ： 距离 & 角度
		bool ball_in_front = true;
		{
			DECLARE_PARAM_READER_BEGIN(General)
			READ_PARAM(IS_SIMULATION)
			DECLARE_PARAM_READER_END
		}		
		if (IS_SIMULATION){
			ball_in_front = (self2ballDist(current_cycle,myNum) < Allow_Start_Kick_Dist)
				&& (fabs(Utils::Normalize(self2ballDir(current_cycle,myNum)-me.Dir())) < Allow_Start_Kick_Angle);
		}

		
		// 角度是否调整好
		//cout<<"ball_in_front "<<ball_in_front<<endl;
		bool my_dir_arrived = ball_in_front && ::deltaMarginKickingMetric(current_cycle,kickdir,kickdirprecision,me.Dir(),myNum);

		_dir_arrived[myNum-1] = /*ball_in_front &&*/ my_dir_arrived;
		last_cycle[myNum-1] = current_cycle;
	//}

	return _dir_arrived[myNum-1];
}
double CWorldModel::getPointShootMaxAngle(CGeoPoint point) {
	CValueRangeList rangeList;
	double leftDir = (CGeoPoint(Param::Field::PITCH_LENGTH / 2, -Param::Field::GOAL_WIDTH / 2) - point).dir();
	double rightDir = (CGeoPoint(Param::Field::PITCH_LENGTH / 2, Param::Field::GOAL_WIDTH / 2) - point).dir();
	CValueRange originRange(leftDir,rightDir);
	rangeList.add(originRange);
	for (int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
		auto& enemy = _pVision->TheirPlayer(i);
		if (enemy.Valid()) {
			double leftDir, rightDir;
			{
				double originDir = (enemy.Pos() - point).dir();
				double d = (enemy.Pos() - point).mod();
				double theta = asin(Param::Vehicle::V2::PLAYER_SIZE / d);
				leftDir = Utils::Normalize(originDir - theta);
				rightDir = Utils::Normalize(originDir + theta);
			}
			rangeList.remove(CValueRange(leftDir, rightDir));
		}
	}
	auto best = rangeList.begin();
	for (auto i = ++rangeList.begin(); i != rangeList.end(); ++i) {
		if (i->getSize() > best->getSize()) {
			best = i;
		}
	}
	return best->getMiddle();
}
bool CWorldModel::isMarked(int num) {
	int closestDist = 9999;
	auto& me = _pVision->OurPlayer(num);
	for (int i = 1; i < Param::Field::MAX_PLAYER; i++) {
		auto& enemy = _pVision->TheirPlayer(i);
		if (enemy.Valid()) {
			double dir1 = (CGeoPoint(Param::Field::PITCH_LENGTH / 2.0, 0) - me.Pos()).dir();
			double dir2 = (enemy.Pos() - me.Pos()).dir();
			double dirDiff = Utils::Normalize(dir1 - dir2);
			if (abs(dirDiff) < Param::Math::PI / 2) {
				double tmpDist = (enemy.Pos() - me.Pos()).mod();
				if (tmpDist < closestDist) {
					closestDist = tmpDist;
				}
			}
		}
	}
	if (closestDist < 30) {
		return true;
	}
	return false;
}
