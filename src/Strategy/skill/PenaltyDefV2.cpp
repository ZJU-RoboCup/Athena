#include "PenaltyDefV2.h"
#include "skill/Factory.h"
#include <VisionModule.h>
#include <PlayerCommand.h>
#include <CommandFactory.h>
#include <ControlModel.h>
#include "BallStatus.h"
#include <BestPlayer.h>
#include <robot_power.h>
#include "RobotCapability.h"
#include "GDebugEngine.h"
#include "GoaliePosV1.h"

namespace {
	bool VERBOSE_MODE       = false;
	int maxPenaltyNum       = 100;
	double self_x           = -Param::Field::PITCH_LENGTH / 2.0 + 8;
	int defaultAverageCycle = 90;
	double defaultVariance  = 0.0;
	CGeoPoint leftPos       = CGeoPoint(-Param::Field::PITCH_LENGTH / 2.0 + 8, -30);
	CGeoPoint rightPos      = CGeoPoint(-Param::Field::PITCH_LENGTH / 2.0 + 8, -2);
	CGeoPoint randomLeft    = CGeoPoint(-Param::Field::PITCH_LENGTH / 2.0 + 8, -30);
	CGeoPoint randomRight   = CGeoPoint(-Param::Field::PITCH_LENGTH / 2.0 + 8, 30);
	CGeoPoint randomLeftArrive  = CGeoPoint(-Param::Field::PITCH_LENGTH / 2.0 + 8, -35);
	CGeoPoint randomRightArrive = CGeoPoint(-Param::Field::PITCH_LENGTH / 2.0 + 8 , 35);
	int AHEAD               = 18; // 提前移动的帧数
	int CATEGORY            = 0;
	bool resetFlag          = false;
	int FRAME               = 0;  // 从zeus2005.xml读入的defaultAverageCycle
}

CPenaltyDefV2::CPenaltyDefV2() {
	DECLARE_PARAM_READER_BEGIN(PENALTY_DEFENCE)
	READ_PARAM(CATEGORY)
	READ_PARAM(FRAME)
	READ_PARAM(AHEAD)
	DECLARE_PARAM_READER_END
	_category = CATEGORY;
	_theirPenaltyNum = 0;
	_isFirst = false;
	for (int i = 0; i < maxPenaltyNum; i++) { _stillCycle[i] = -1; }
	_averageStillCycle = defaultAverageCycle;
	_stillCycleCnt = 0;
	_isNormalStartLastCycle = false;
	_initOppDir = Param::Math::PI;
	_readyCnt   = 0;
	_readyFlag  = false;
	_variance   = defaultVariance;
	switchCnt   = 0;
	double self_x           = -Param::Field::PITCH_LENGTH / 2.0 + 8;
	CGeoPoint leftPos       = CGeoPoint(-Param::Field::PITCH_LENGTH / 2.0 + 8, -20);
	CGeoPoint rightPos      = CGeoPoint(-Param::Field::PITCH_LENGTH / 2.0 + 8, -2);
}

CPenaltyDefV2::~CPenaltyDefV2() {}

void CPenaltyDefV2::plan(const CVisionModule* pVision) {
	if (resetFlag && (CATEGORY == 3 || _state == CATEGORY_3)) {
		VisionModule::Instance()->ResetTheirPenaltyNum();
		resetData();
		resetFlag = false;
	}

	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1 ) { _state = BEGINNING; }
	// 视觉预处理
	const MobileVisionT& ball = pVision->Ball();
	const int runner = task().executor;
	const int flag = task().player.flag;
	const PlayerVisionT& self  = pVision->OurPlayer(runner);
	const PlayerVisionT& enemy = pVision->TheirPlayer(this->GetNearestEnemy(pVision));
	if (VERBOSE_MODE) {
		GDebugEngine::Instance()->gui_debug_line(ball.Pos(), CGeoPoint(-Param::Field::PITCH_LENGTH / 2, Param::Field::GOAL_WIDTH / 2), COLOR_WHITE);
		GDebugEngine::Instance()->gui_debug_line(ball.Pos(), CGeoPoint(-Param::Field::PITCH_LENGTH / 2, -Param::Field::GOAL_WIDTH / 2), COLOR_WHITE);
	}

	static double face_dir = 0.0;

	bool isNormalStart = VisionModule::Instance()->gameState().canEitherKickBall();
	bool isGameOn      = VisionModule::Instance()->gameState().gameOn();

	static int leftOrRight = 1; // 1为left, -1为right

	if (isNormalStart && !_isNormalStartLastCycle) { // 收到normal start的那一帧
		captureFlag = true;
	}

	_theirPenaltyNum = VisionModule::Instance()->GetTheirPenaltyNum();
	if (VERBOSE_MODE) { cout << "_theirPenaltyNum: " << _theirPenaltyNum << endl; }
	if (_theirPenaltyNum == 1) { _isFirst = true; }
	else { _isFirst = false; }
	if (_isFirst == true && VERBOSE_MODE) { cout << "First get into CPenaltyDefV2 !" << endl; }

	if (_readyFlag == false && captureFlag) {
		_readyFlag  = isOppReady(pVision);
		_initOppDir = enemy.Dir();
	}

	if (isNormalStart && _readyFlag && !isOppTurn(pVision) && captureFlag) {
		_stillCycleCnt++;
	}

	if (isOppTurn(pVision) && captureFlag) {
		_stillCycle[_theirPenaltyNum - 1] = _stillCycleCnt;
		if (_state == CATEGORY_1) {
			_stillCycle[_theirPenaltyNum - 1] = 0;
		}
		for (int i = 0; i < _theirPenaltyNum - 1; i++) {
			if (_stillCycle[i] == -1) {
				_stillCycle[i] = 0;
			}
		}
		_stillCycleCnt = 0;
		_readyFlag     = false;
		captureFlag    = false;
	}

	//if (true || VERBOSE_MODE) { printStillCycle(); }

	if (FRAME == 0 || CATEGORY != 2) {
		calAverageStillCycle(); // 计算平均时间
		calVariance(); // 计算时间的方差
		if (_variance > 50) {
			resetFlag = true;
		}
		else {
			resetFlag = false;
		}
	}
	else {
		_averageStillCycle = FRAME;
		_variance = defaultVariance;
	}
	cout << "_averageStillCycle: " << _averageStillCycle << endl;
	//cout << "_variance: " << _variance << endl;
	//cout << "AHEAD: " << AHEAD << endl;
	if(false || VERBOSE_MODE) { cout << "isGameOn: " << isGameOn << endl; }
	if(false || VERBOSE_MODE) { cout << "isNormalStart:" << isNormalStart << "  still Cycle count: " << _stillCycleCnt << endl; }

	static CGeoPoint RandomPos = CGeoPoint(-Param::Field::PITCH_LENGTH / 2.0 + 8, 0);
	static int randomCnt = 0;

	switch (_state) {
	case BEGINNING:
		{
			//cout << "_category: " << _category << endl;
			if (_category == 1) {
				_state = CATEGORY_1;
				if (VERBOSE_MODE) { cout << "BEGINNING --> CATEGORY_1" << endl; }
			}
			else if (_category == 2) {
				_state = CATEGORY_2;
				if (VERBOSE_MODE) { cout << "BEGINNING --> CATEGORY_2" << endl; }
			}
			else if (_category == 3) {
				_state = CATEGORY_3;
				if (VERBOSE_MODE) { cout << "BEGINNING --> CATEGORY_3" << endl; }
			}
			else if (_category == 0) {
				_state = CATEGORY_0;
				if (VERBOSE_MODE) { cout << "BEGINNING --> CATEGORY_0" << endl; }
			}
		}
		break;

	// CATEGORY_0 分支----------------------------------------------------------------------------
	case CATEGORY_0:
		{
			int num = rand() % 3 + 1;
			//cout << "num: " << num << endl;
			if (num == 1) {
				_state = CATEGORY_1;
			}
			else if (num == 2) {
				_state = CATEGORY_2;
			}
			else if (num == 3) {
				_state = CATEGORY_3;
			}
			else {
				_state = CATEGORY_0;
			}
		}
		break;

	// CATEGORY_1 分支----------------------------------------------------------------------------
	case CATEGORY_1:
		{
			_state = WAIT0;
			if (VERBOSE_MODE) { cout << "CATEGORY_1 --> RANDOM1" << endl; }
		}
		break;
	case RANDOM1:
		{
			if (self.Pos().dist(randomLeftArrive) < 5) {
				_state = RANDOM2;
			}
			switchCnt++;
			if (switchCnt >= rand() % 30 + 15) {
				_state = WAIT0;
				switchCnt = 0;
			}
		}
		break;
	case RANDOM2:
		{
			if (self.Pos().dist(randomRightArrive) < 5) {
				_state = RANDOM1;
			}
			switchCnt++;
			if (switchCnt >= rand() % 30 + 15) {
				_state = WAIT0;
				switchCnt = 0;
			}
		}
		break;
	case WAIT0:
		{
			switchCnt++;
			if (switchCnt >= 60) {
				_state = RANDOM1;
				switchCnt = 0;
			}
		}
		break;

	// CATEGORY_2 分支----------------------------------------------------------------------------
	case CATEGORY_2:
		if (_isFirst) {
			_state = POS_1;
			if (VERBOSE_MODE) { cout << "CATEGORY_2 --> POS_1" << endl; }
		}
		else {
			_state = WAIT;
			if (VERBOSE_MODE) { cout << "CATEGORY_2 --> WAIT" << endl; }
		}
		break;
	case POS_1:
		if (self.Pos().dist(leftPos) < 2) {
			_state = POS_2;
			if (VERBOSE_MODE) { cout << "POS_1 --> POS_2" << endl; }
		}
		if (_theirPenaltyNum != 1) {
			_state = WAIT;
			if (VERBOSE_MODE) { cout << "POS_1 --> WAIT" << endl; }
		}
		break;
	case POS_2:
		if (self.Pos().dist(rightPos) < 2) {
			_state = POS_1;
			if (VERBOSE_MODE) { cout << "POS_2 --> POS_1" << endl; }
		}
		if (_theirPenaltyNum != 1) {
			_state = WAIT;
			if (VERBOSE_MODE) { cout << "POS_2 --> WAIT" << endl; }
		}
		break;
	case WAIT:
		if (_stillCycleCnt >= _averageStillCycle - AHEAD) {
			_state = RUSH;
			if (VERBOSE_MODE) { cout << "WAIT --> RUSH" << endl; }
		}
		if (_theirPenaltyNum == 1) {
			_state = POS_1;
			if (VERBOSE_MODE) { cout << "WAIT --> POS_1" << endl; }
		}
		if (self.Pos().y() < 0) {
			leftOrRight = 1;
		}
		else {
			leftOrRight = -1;
		}
		break;
	case RUSH:
		if (self.Pos().dist(CGeoPoint(self_x, leftOrRight * 22)) < 5) {
			leftOrRight = 0;
		}
		break;

	// CATEGORY_3 分支----------------------------------------------------------------------------
	case CATEGORY_3:
		if (_isFirst) {
			_state = POS_3;
			if (VERBOSE_MODE) { cout << "CATEGORY_2 --> POS_3" << endl; }
		}
		else {
			_state = WAIT2;
			if (VERBOSE_MODE) { cout << "CATEGORY_2 --> WAIT2" << endl; }
		}
		break;
	case POS_3:
		if (self.Pos().dist(leftPos) < 2) {
			_state = POS_4;
			if (VERBOSE_MODE) { cout << "POS_3 --> POS_4" << endl; }
		}
		if (_theirPenaltyNum != 1) {
			_state = WAIT2;
			if (VERBOSE_MODE) { cout << "POS_3 --> WAIT2" << endl; }
		}
		break;
	case POS_4:
		if (self.Pos().dist(rightPos) < 2) {
			_state = POS_3;
			if (VERBOSE_MODE) { cout << "POS_4 --> POS_3" << endl; }
		}
		if (_theirPenaltyNum != 1) {
			_state = WAIT2;
			if (VERBOSE_MODE) { cout << "POS_4 --> WAIT2" << endl; }
		}
		break;
	case WAIT2:
		if (_stillCycleCnt >= _averageStillCycle - AHEAD) {
			_state = RUSH2;
			if (VERBOSE_MODE) { cout << "WAIT2 --> RUSH2" << endl; }
		}
		if (_theirPenaltyNum == 1) {
			_state = POS_3;
			if (VERBOSE_MODE) { cout << "WAIT2 --> POS_3" << endl; }
		}
		if (self.Pos().y() < 0) {
			leftOrRight = 1;
		}
		else {
			leftOrRight = -1;
		}
		break;
	case RUSH2:
		if (self.Pos().dist(CGeoPoint(self_x, leftOrRight * 22)) < 5) {
			leftOrRight = 0;
		}
		break;
	//---------------------------------------------------------------------------------------------
	case STOP:
		setSubTask(PlayerRole::makeItStop(runner));
		break;
	default:
		_state = BEGINNING;
		break;
	}

	//cout << "_state: " << _state << endl;

	switch(_state) {
	// CATEGORY_0 分支----------------------------------------------------------------------------

	// CATEGORY_1 分支----------------------------------------------------------------------------
	case RANDOM1:
		{
			face_dir = 0.0;
			TaskT new_task;
			new_task.executor = runner;
			new_task.player.pos = randomLeft;
			new_task.player.angle = face_dir;
			new_task.player.max_acceleration = 1400;
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(new_task));
		}
		break;
	case RANDOM2:
		{
			face_dir = 0.0;
			TaskT new_task;
			new_task.executor = runner;
			new_task.player.pos = randomRight;
			new_task.player.angle = face_dir;
			new_task.player.max_acceleration = 1400;
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(new_task));
		}
		break;
	case WAIT0:
		{
			CGeoPoint waitPos = GoaliePosV1::Instance()->GetPenaltyShootPos(pVision);
			if (waitPos.y() > 17) {
				waitPos.setY(17);
			}
			if (waitPos.y() < -17) {
				waitPos.setY(-17);
			}
			face_dir = 0.0;
			TaskT new_task;
			new_task.executor = runner;
			new_task.player.pos = waitPos;
			new_task.player.angle = face_dir;
			new_task.player.max_acceleration = 1400;
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(new_task));
			
		}

	// CATEGORY_2 分支----------------------------------------------------------------------------
	case POS_1:
		{
			if ( Utils::Normalize(enemy.Dir() - Param::Math::PI) < -0.03 ) {
				leftPos.setY(20);
				rightPos.setY(2);
			}
			else {
				leftPos.setY(-20);
				rightPos.setY(-2);
			}
			face_dir = 0.0;
			TaskT new_task;
			new_task.executor = runner;
			new_task.player.pos = leftPos;
			new_task.player.angle = face_dir;
			new_task.player.max_acceleration = 1400;
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(new_task));
		}
		break;
	case POS_2:
		{
			face_dir = 0.0;
			TaskT new_task;
			new_task.executor = runner;
			new_task.player.pos = rightPos;
			new_task.player.angle = face_dir;
			new_task.player.max_acceleration = 150;
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(new_task));
		}
		break;
	case WAIT:
		{
			CGeoPoint waitPos = GoaliePosV1::Instance()->GetPenaltyShootPos(pVision);
			if (waitPos.y() > 17) {
				waitPos.setY(17);
			}
			if (waitPos.y() < -17) {
				waitPos.setY(-17);
			}
			face_dir = 0.0;
			TaskT new_task;
			new_task.executor = runner;
			new_task.player.pos = waitPos;
			new_task.player.angle = face_dir;
			new_task.player.max_acceleration = 1400;
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(new_task));
		}
		break;
	case RUSH:
		{
			CGeoPoint rushPos = CGeoPoint(self_x, leftOrRight * 200.0);
			face_dir = 0.0;
			TaskT new_task;
			new_task.executor = runner;
			new_task.player.pos = rushPos;
			new_task.player.angle = face_dir;
			new_task.player.max_acceleration = 1400;
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(new_task));
		}
		break;

	// CATEGORY_3 分支----------------------------------------------------------------------------
	case POS_3:
		{
			if ( Utils::Normalize(enemy.Dir() - Param::Math::PI) < -0.03 ) {
				leftPos.setY(20);
				rightPos.setY(2);
			}
			else {
				leftPos.setY(-20);
				rightPos.setY(-2);
			}
			face_dir = 0.0;
			TaskT new_task;
			new_task.executor = runner;
			new_task.player.pos = leftPos;
			new_task.player.angle = face_dir;
			new_task.player.max_acceleration = 1400;
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(new_task));
		}
		break;
	case POS_4:
		{
			face_dir = 0.0;
			TaskT new_task;
			new_task.executor = runner;
			new_task.player.pos = rightPos;
			new_task.player.angle = face_dir;
			new_task.player.max_acceleration = 150;
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(new_task));
		}
		break;
	case WAIT2:
		{
			CGeoPoint waitPos = GoaliePosV1::Instance()->GetPenaltyShootPos(pVision);
			if (waitPos.y() > 17) {
				waitPos.setY(17);
			}
			if (waitPos.y() < -17) {
				waitPos.setY(-17);
			}
			face_dir = 0.0;
			TaskT new_task;
			new_task.executor = runner;
			new_task.player.pos = waitPos;
			new_task.player.angle = face_dir;
			new_task.player.max_acceleration = 1400;
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(new_task));
		}
		break;
	case RUSH2:
		{
			face_dir = 0.0;
			CGeoPoint rushPos = CGeoPoint(self_x, leftOrRight * 200.0);
			TaskT new_task;
			new_task.executor = runner;
			new_task.player.pos = rushPos;
			new_task.player.angle = face_dir;
			new_task.player.max_acceleration = 1400;
			setSubTask(TaskFactoryV2::Instance()->GotoPosition(new_task));
		}
		break;
	}

	_lastCycle  = pVision->Cycle();
	_isNormalStartLastCycle = VisionModule::Instance()->gameState().canEitherKickBall();
	CStatedTask::plan(pVision);
}

CPlayerCommand* CPenaltyDefV2::execute(const CVisionModule* pVision) {
	if( subTask() ) {
		return subTask()->execute(pVision);
	}
	if( _directCommand ) {
		return _directCommand;
	}
	return 0;
}

bool CPenaltyDefV2::isOppTurn(const CVisionModule *pVision) {
	const PlayerVisionT& enemy = pVision->TheirPlayer(this->GetNearestEnemy(pVision));
	double currentOppDir = enemy.Dir();
	if (fabs(Utils::Normalize(currentOppDir - _initOppDir)) > Param::Math::PI / 90) {
		return true;
	}
	return false;
}

// 当球距地方车一定距离范围内，并保持一定时间时，返回ture
bool CPenaltyDefV2::isOppReady(const CVisionModule *pVision) {
	const PlayerVisionT& enemy = pVision->TheirPlayer(this->GetNearestEnemy(pVision));
	const MobileVisionT& ball  = pVision->Ball();
	CVector opp2ball = ball.Pos() - enemy.Pos();
	double opp2ballDist = opp2ball.mod();
	//cout << "opp2ballDist: " << opp2ballDist << endl;
	if (opp2ballDist < 11.5) {
		_readyCnt++;
	}
	//cout << "_readyCnt: " << _readyCnt << endl;
	if (_readyCnt > 5) {
		_readyCnt = 0;
		return true;
	}
	return false;
}

bool CPenaltyDefV2::isOppDribble(const CVisionModule *pVision) {
	return false;
}

int CPenaltyDefV2::GetNearestEnemy(const CVisionModule *pVision) {
	CGeoPoint goal_center(- Param::Field::PITCH_LENGTH / 2, 0);
	double nearest_dist = Param::Field::PITCH_LENGTH;
	int enemy_num = 1;
	for (int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
		if (pVision->TheirPlayer(i).Valid() == true) {
			if (pVision->TheirPlayer(i).Pos().dist(goal_center) < nearest_dist) {
				nearest_dist = pVision->TheirPlayer(i).Pos().dist(goal_center);
				enemy_num = i;
			}
		}
	}
	return enemy_num;
}

void CPenaltyDefV2::printStillCycle() {
	cout << "Still Cycle is : ------------------------------------------------------------" << endl;
	for (int i = 0; i < maxPenaltyNum; i++) {
		if (_stillCycle[i] == -1) { break; }
		cout << "No." << i+1 << ": " << _stillCycle[i] << " | ";
		if ( (i+1) % 7 == 0) { cout << endl; }
	}
	cout << endl;
	cout << "-----------------------------------------------------------------------------" << endl;
}

void CPenaltyDefV2::calAverageStillCycle() {
	int temp = 0;
	int j = 0;
	for (int i = 0; i < maxPenaltyNum; i++) {
		if (_stillCycle[i] == -1) { break; }
		if (_stillCycle[i] > 30) {
			temp += _stillCycle[i];
			j++;
		}
	}
	if (j == 0) {
		_averageStillCycle = defaultAverageCycle;
	}
	else {
		_averageStillCycle = temp / j;
	}
}

void CPenaltyDefV2::calVariance() {
	int j = 0;
	double temp = 0;
	for (int i = 0; i < maxPenaltyNum; i++) {
		if (_stillCycle[i] == -1) { break; }
		if (fabs((double)_stillCycle[i] - _averageStillCycle) < 20 && _stillCycle[i] != 0) {
			temp += (_averageStillCycle - _stillCycle[i]) * (_averageStillCycle - _stillCycle[i]);
			j++;
		}
		else {
			_stillCycle[i] = 0;
		}
	}
	if (j == 0) {
		_variance = defaultVariance;
	}
	else {
		_variance = temp / j;
	}
}

void CPenaltyDefV2::resetData() {
	_theirPenaltyNum = 0;
	for (int i = 0; i < maxPenaltyNum; i++) { _stillCycle[i] = -1; }
	_averageStillCycle = defaultAverageCycle;
	_stillCycleCnt = 0;
	_variance   = defaultVariance;
	_readyFlag  = false;
	captureFlag = false;
}