#include "WorldModel.h"
#include "VisionModule.h"
#include "RobotCapability.h"
#include "ControlModel.h"
#include "BallStatus.h"
#include "RobotSensor.h"
#include "KickStatus.h"
#include "DribbleStatus.h"
#include "BestPlayer.h"
#include "GDebugEngine.h"
#include "PlayInterface.h"
#include "BufferCounter.h"

// Ĭ�ϲ�����ʼ��
const int CWorldModel::myDefaultNum = 0;
const int CWorldModel::enemyDefaultNum = 0;

// �Լ������ʸ��
const CVector CWorldModel::self2ball(int current_cycle,  int myNum,  int enemyNum) {
	static int last_cycle[Param::Field::MAX_PLAYER] = {-1,-1,-1,-1,-1,-1};
	static CVector _self2ball[Param::Field::MAX_PLAYER];

	if (last_cycle[myNum-1] < current_cycle) {
		_self2ball[myNum-1] = _pVision->Ball().Pos() - _pVision->OurPlayer(myNum).Pos();
		last_cycle[myNum-1] = current_cycle;
	}

	return _self2ball[myNum-1];
}

// �Լ�����ľ���
const double CWorldModel::self2ballDist(int current_cycle,  int myNum,  int enemyNum) {
	return self2ball(current_cycle, myNum, enemyNum).mod();	// ���Լ��ټ�����
}

// �Լ�����ĽǶ�
const double CWorldModel::self2ballDir(int current_cycle,  int myNum,  int enemyNum) {
	return self2ball(current_cycle, myNum, enemyNum).dir();	// ���Լ��ټ�����
}

const string CWorldModel::CurrentRefereeMsg()
{
	return vision()->GetCurrentRefereeMsg();
}