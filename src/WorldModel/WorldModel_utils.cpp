#include "WorldModel.h"
#include "ValueRange.h"
#include "GDebugEngine.h"
/************************************************************************/
/* �����ռ�                                                             */
/************************************************************************/
namespace {
	///> ��������
	// ���������ֵ
	const double Allow_Start_Kick_Dist = 5*Param::Vehicle::V2::PLAYER_SIZE;
	const double Allow_Start_Kick_Angle = 2.5 * Param::Vehicle::V2::KICK_ANGLE;
	static bool IS_SIMULATION = false;
	// �ȶ�����������ž���
	const double beta = 1.0;
	const int critical_cycle = 10;
	int kick_last_cycle = 0;
	const int kick_stable_cnt[Param::Field::MAX_PLAYER + 1] = { 1,1,1,1,1,1,1 };     //С���ʱ��ҪСһ��
	int aim_count[Param::Field::MAX_PLAYER + 1] = {0,0,0,0,0,0,0};
	int aim_cycle[Param::Field::MAX_PLAYER + 1] = {0,0,0,0,0,0,0};
	// �жϸ�������: �����жϽǶ��Ƿ��е�����,����CMU���жϷ�ʽ
	bool deltaMarginKickingMetric(int current_cycle, double gt, double delta, double mydir, int myNum)
	{
		// �������ǰ�����ݣ���Ҫ�������
		if (current_cycle - aim_cycle[myNum] > critical_cycle) {
			aim_count[myNum] = 0;
		}
		aim_cycle[myNum] = current_cycle;

		// ���㵱ǰ��margin������¼
		double gl = Utils::Normalize(gt - delta);	// Ŀ�귽������ƫ�����ž�����ֵ
		double gr = Utils::Normalize(gt + delta);	// Ŀ�귽������ƫ�����ž�����ֵ
		double current_margin = max(Utils::Normalize(mydir - gl), Utils::Normalize(gr - mydir));

		// ����margin�ж��Ƿ�ﵽ���򾫶�
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

	///> ���ڿ���
	// ���������ֵ
	const bool Allow_Start_Dribble = true;
	const double Allow_Start_Dribble_Dist = 6.0 * Param::Vehicle::V2::PLAYER_SIZE;
	const double Allow_Start_Dribble_Angle = 5.0 * Param::Vehicle::V2::KICK_ANGLE;

}

///> ��#�� ����ʱ�����Ƿ������λ��������
const bool  CWorldModel::KickDirArrived(int current_cycle, double kickdir, double kickdirprecision, int myNum) {
	static int last_cycle[Param::Field::MAX_PLAYER] = {-1,-1,-1,-1,-1,-1};
	static bool _dir_arrived[Param::Field::MAX_PLAYER] = {false,false,false,false,false,false};

	// ����
	kick_last_cycle = current_cycle;

	// ����
	//if (last_cycle[myNum-1] < current_cycle) {
		const PlayerVisionT& me = this->_pVision->OurPlayer(myNum);
		// ������ǰ �� ���� & �Ƕ�
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

		
		// �Ƕ��Ƿ������
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
