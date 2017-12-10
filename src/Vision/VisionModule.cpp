// Time-stamp: <14:27:11  10-13-2009>

#include <string>
#include <cmath>
#include <VisionModule.h>
#include <WorldModel.h>
#include <utils.h>
#include <playmode.h>
#include <tinyxml/ParamReader.h>
#include <DribbleStatus.h>
#include <RobotCapability.h>
#include <TaskMediator.h>
#include <KickStatus.h>
#include <BestPlayer.h>
#include <PlayInterface.h>
#include <fstream>
#include <RobotSensor.h>
#include <GDebugEngine.h>
#include <RobotsCollision.h>
#include <BallStatus.h>
#include "bayes/MatchState.h"
#include "defence/DefenceInfo.h"
#include "XBoxController.h"
#include "Semaphore.h"
Semaphore vision_to_decision(0);

int CVisionModule::RECEIVE_X_MIN;
int CVisionModule::RECEIVE_X_MAX;
int CVisionModule::RECEIVE_Y_MIN;
int CVisionModule::RECEIVE_Y_MAX;

///
/// @file   VisionModule.cpp
/// @author Yonghai Wu <liunian@zju.edu.cn>
/// @date   Tue Oct 13 14:26:36 2009
/// 
/// @brief  视觉预处理，包括滤波，预测等等
/// 
/// 
///
namespace {
	bool VERBOSE_MODE = false; // 输出信息
	bool IS_SIMULATION = false; // 是否仿真
	XBoxController xboxCtrl(1);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	CVisionModule::CVisionModule(const COptionModule* pOption) : _pOption(pOption),
/// 	_timeCycle(0), _lastTimeCycle(0),  _ballKicked(false),
/// 	_ballInvalidMovedCycle(0),_chipkickDist(0.0),_kickerDir(0.0)
///
/// @brief	Constructor. 
///
/// @author	Yonghai Wu
/// @date	2009-10-13
///
/// @param	pOption	If non-null, the option. 
////////////////////////////////////////////////////////////////////////////////////////////////////

CVisionModule::CVisionModule() 
:ballPosCollectorCounter(0),_timeCycle(0), _lastTimeCycle(0), _ballKicked(false), _ourGoalie(0), _theirGoalie(0), _theirGoalieStrategyNum(0), _validNum(0),_TheirValidNum(0)
{	
	{
		DECLARE_PARAM_READER_BEGIN(General)
			READ_PARAM(IS_SIMULATION)
		DECLARE_PARAM_READER_END
	}
	{
		DECLARE_PARAM_READER_BEGIN(VisionMediator)
			READ_PARAM(RECEIVE_X_MIN)
			READ_PARAM(RECEIVE_X_MAX)
			READ_PARAM(RECEIVE_Y_MIN)
			READ_PARAM(RECEIVE_Y_MAX)
		DECLARE_PARAM_READER_END
	}
	
	WorldModel::Instance()->registerVision(this);
	for (int i=0; i < Param::Field::MAX_PLAYER; i++){
		//恢复初始化对方车预测为能识别角度信息
		_theirPlayerPredictor[i].setIsHasRotation(false);
	}

	RobotsCollisionDetector::Instance()->setVision(this);
	_last_xbox_pressed = PMNone;

	_theirPenaltyNum = 0;
	_ballVelChangeCouter = true;
}


void CVisionModule::registerOption(const COptionModule* pOption)
{
	_pOption = pOption;
	_gameState.init(pOption->MyColor());
}

CVisionModule::~CVisionModule(void)
{

}


void CVisionModule::SetRefRecvMsg(const RefRecvMsg msg)
{
	_refRecvMsg = msg;
}

void CVisionModule::SetNewVision(const CServerInterface::VisualInfo& vInfo)
{	
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////重要说明，请注意////////////////////////////
	//	定义我方小车位置在左侧，对方车位置在右侧，笛卡尔坐标定义<x y theta>	//
	//	比赛时不管是左边还是右边，对于策略处理而言统一定义为我方在左半场	//
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// 默认我方在球场的左边，切记

	GDebugEngine::Instance()->gui_debug_line(CGeoPoint(RECEIVE_X_MIN / 10, RECEIVE_Y_MIN / 10), CGeoPoint(RECEIVE_X_MIN / 10, RECEIVE_Y_MAX / 10), COLOR_RED);
	GDebugEngine::Instance()->gui_debug_line(CGeoPoint(RECEIVE_X_MIN / 10, RECEIVE_Y_MIN / 10), CGeoPoint(RECEIVE_X_MAX / 10, RECEIVE_Y_MIN / 10), COLOR_RED);
	GDebugEngine::Instance()->gui_debug_line(CGeoPoint(RECEIVE_X_MAX / 10, RECEIVE_Y_MAX / 10), CGeoPoint(RECEIVE_X_MIN / 10, RECEIVE_Y_MAX / 10), COLOR_RED);
	GDebugEngine::Instance()->gui_debug_line(CGeoPoint(RECEIVE_X_MAX / 10, RECEIVE_Y_MAX / 10), CGeoPoint(RECEIVE_X_MAX / 10, RECEIVE_Y_MIN / 10), COLOR_RED);

	static PosT _lastBall;
	static double _lastDistBuffer = 0;
	const auto& p1 = _lastBall;
	const auto& p2 = vInfo.ball.valid ? vInfo.ball: _lastBall;
	auto distBuffer = std::sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
	bool ballFilterSwitch = false;
	if (vInfo.ball.valid && Utils::OutOfField(CGeoPoint(vInfo.ball.x, vInfo.ball.y), -5)) {
		ballFilterSwitch = true;
	}else if (distBuffer > 0.5 && _lastDistBuffer > 0.5) {
		for (int i = 0; i < 12; i++) {
			const auto& p1 = vInfo.player[i].pos;
			const auto& p2 = vInfo.ball;
			auto dist2 = std::sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
			if (dist2 < 15 + distBuffer) {
				ballFilterSwitch = true;
				break;
			}
		}
	}
	_lastDistBuffer = distBuffer;
	_lastBall = vInfo.ball;
	const bool invert = !(_pOption->MySide() == Param::Field::POS_SIDE_LEFT);

	if (_pOption->MyColor() == TEAM_BLUE) {
		_ourGoal = _refRecvMsg.blueGoal;
		_theirGoal = _refRecvMsg.yellowGoal;
		_ourGoalie = _refRecvMsg.blueGoalie;
		_theirGoalie = _refRecvMsg.yellowGoalie;
	} else{
		_ourGoal = _refRecvMsg.yellowGoal;
		_theirGoal = _refRecvMsg.blueGoal;
		_ourGoalie = _refRecvMsg.yellowGoalie;
		_theirGoalie = _refRecvMsg.blueGoalie;
	}
	/////////////////////////////////////////////////////////////////////////////
	/// @brief Step 0. 比赛开始时，记录调试信息，辅助调试
	/////////////////////////////////////////////////////////////////////////////
	// 更新当前的时间周期
	SetCycle(vInfo.cycle);
	
	// 更新新图像数据中的原始球信息
	_lastRawBallPos = _rawBallPos;
	_rawBallPos.SetValid(vInfo.ball.valid);
	// add by zhyaic 2013.6.3 原始的位置应该反向
	const int invertFactor = invert ? -1 : 1;
	_rawBallPos.SetPos(vInfo.ball.x * invertFactor,vInfo.ball.y * invertFactor);

	/////////////////////////////////////////////////////////////////////////////
    /// @brief Step 1. 进行球预测,也就是输入当前球的观测进行滤波
    /////////////////////////////////////////////////////////////////////////////
    _ballPredictor.updateVision(vInfo.cycle, vInfo.ball, invert, ballFilterSwitch);
	
	//printf("%d\n", RobotCapFactory::Instance()->getRobotCap(0,2)->maxSpeed(0));
	/////////////////////////////////////////////////////////////////////////////
	/// @brief Step 2: 进行我方和对方机器人位置预测，关注滤波器
    /////////////////////////////////////////////////////////////////////////////
	//【#TODO】机器人碰撞检测器清空，
	RobotsCollisionDetector::Instance()->clearLastResult();
	const MobileVisionT& thisBall = _ballPredictor.getData(Cycle());	
	// 确定球员的信息是否需要反向，确保正确更新比赛双方球员的信息
	const bool player_invert = ( (_pOption->MyColor()==TEAM_YELLOW) && IS_SIMULATION);
	for (int i = 0; i < Param::Field::MAX_PLAYER; ++ i) {
		const VehicleInfoT& ourPlayer = vInfo.player[player_invert? i+Param::Field::MAX_PLAYER : i];
		const VehicleInfoT& theirPlayer = vInfo.player[player_invert? i : i+Param::Field::MAX_PLAYER];

		RobotsCollisionDetector::Instance()->setCheckNum(0, i+1);
		_ourPlayerPredictor[i].updateVision(vInfo.cycle, ourPlayer, thisBall, invert, vInfo.ourRobotIndex[i]);
		RobotsCollisionDetector::Instance()->setCheckNum(1, i+1);
		_theirPlayerPredictor[i].updateVision(vInfo.cycle, theirPlayer, thisBall, invert, vInfo.theirRobotIndex[i]);
	} 

	//【#TODO】更新双方当前在场上的球员数量，我方排除门将，对方全部
	CheckBothSidePlayerNum();
	
    /////////////////////////////////////////////////////////////////////////////
	/// @brief Step 3: 更新双向通讯的数据，仅针对实物模式
    /////////////////////////////////////////////////////////////////////////////
	if (! IS_SIMULATION) {
		RobotSensor::Instance()->Update(this->Cycle());
	}
	
	/////////////////////////////////////////////////////////////////////////////
	/// @brief Step 4: 碰撞检测
	/////////////////////////////////////////////////////////////////////////////
	// 在预测模型里面加入了简单的碰撞模拟，车可以推着球走
	// 但是有时候己方和对方的车都和球碰撞，两个结果要选择一个
	// 使用我方的结果，对带球比较有利，但是可能会发生球实际上被对方带走，
	// 而还认为在我方控制的情况
	// 现在的做法是，看到的球的时候优先用我方的结果，没看到的时候优先用对方的结果，
	// 可以防止上述情况发生
	// 【#TODO】
	_hasCollision = false;

	if (vInfo.ball.valid) { // 看到球了
		// 优先考虑对方
		for (int i = 0; i < Param::Field::MAX_PLAYER; ++ i) {
			if (_theirPlayerPredictor[i].collideWithBall()) {
				static int block_cycle = 0;
				if (!BallStatus::Instance()->getChipKickState()) {
					_hasCollision = true;
					_ballPredictor.setCollisionResult(Cycle(), _theirPlayerPredictor[i].ballCollidedResult());
					break;
				} else if (block_cycle++>15) {
					// 在对手身上一定时间内,才认为是与对手相碰了,否则可能是调球越过了车顶
					_hasCollision = true;
					_ballPredictor.setCollisionResult(Cycle(), _theirPlayerPredictor[i].ballCollidedResult());
					BallStatus::Instance()->setChipKickState(false);
					block_cycle = 0;
					break;
				} else {
					//std::cout<<"have opp but ball is on the air"<<endl;
				}
			}
		}
		// 然后考虑我方
		for (int i = 0; i < Param::Field::MAX_PLAYER; ++ i) {
			if (_ourPlayerPredictor[i].collideWithBall()) {
				_hasCollision = true;
				_ballPredictor.setCollisionResult(Cycle(), _ourPlayerPredictor[i].ballCollidedResult());
				// 先注释掉，这样拿球可能会变快
				//_ballPredictor.getData(Cycle()).SetVel(0, 0); // 方便带球任务, 不然球速太快
				//cout<<"set: "<<_ballPredictor.getResult(_timeCycle).Vel().mod()<<endl;
				break;
			}
		}
	} else { // 没有看到球
		// 优先考虑我方
		for (int i = 0; i < Param::Field::MAX_PLAYER; ++ i) {
			if (_ourPlayerPredictor[i].collideWithBall()) {
				_hasCollision = true;
				_ballPredictor.setCollisionResult(Cycle(), _ourPlayerPredictor[i].ballCollidedResult());
				//_ballPredictor.getData(Cycle()).SetVel(0, 0);  // 方便带球任务, 不然球速太快
				break;
			}
		}
		// 然后考虑对方
		for (int i = 0; i < Param::Field::MAX_PLAYER; ++ i) {
			if (_theirPlayerPredictor[i].collideWithBall()) {
				static int block_cycle = 0;
				if (!BallStatus::Instance()->getChipKickState()) {
					_hasCollision = true;
					_ballPredictor.setCollisionResult(Cycle(), _theirPlayerPredictor[i].ballCollidedResult());
					break;
				} else if (block_cycle++>15) {
					_hasCollision = true;
					_ballPredictor.setCollisionResult(Cycle(), _theirPlayerPredictor[i].ballCollidedResult());
					BallStatus::Instance()->setChipKickState(false);
					block_cycle = 0;
					break;
				} else {
				}
			}
		}
	}
	
	//judgeBallVelStable();

	static bool isChip = false;
	static int chipCycle = 0, notChipCycle = 0;
	static int CHIPCYCLE = 4, NOTCHIPCYCLE = 1;
	static int CHIPLENGTH = 1.1, NOTCHIPLENGTH = 1.3;
	_ballPos[ballPosCollectorCounter] = _rawBallPos.Pos();
	CGeoLine l(_ballPos[ballPosCollectorCounter],_ballPos[(ballPosCollectorCounter + 4) % 10]);
	CGeoPoint p = _ballPos[(ballPosCollectorCounter + 7) % 10];
	if (l.projection(p).dist(p) > CHIPLENGTH) {
		chipCycle++;
		if (chipCycle > CHIPCYCLE) {
			isChip = true;
			chipCycle = CHIPCYCLE;
			notChipCycle = 0;
		}
	}
	if (l.projection(p).dist(p) < NOTCHIPLENGTH) {
		notChipCycle++;
		if (notChipCycle > NOTCHIPCYCLE) {
			isChip = false;
			chipCycle = 0;
			notChipCycle = NOTCHIPCYCLE;
		}
	}
	//cout << l.projection(p).dist(p) << endl;
	ballPosCollectorCounter = (ballPosCollectorCounter+1) % 10;
	/////////////////////////////////////////////////////////////////////////////
	/// @brief Step 5: 更新与决策相关部分的上层信息
	/////////////////////////////////////////////////////////////////////////////
	// 【#TODO】 球状态模块更新状态, 这部分到时仍需要再细致调试下
	BallStatus::Instance()->UpdateBallStatus(this);
	if ((isChip || BallStatus::Instance()->IsChipKickedOut()) &&
		//_rawBallPos.Valid() && 
		_rawBallPos.Vel().mod()>50) {

		_newBallPos.SetPos(BallStatus::Instance()->ballChipLine().projection(_rawBallPos.Pos()));
		//_rawBallPos.SetPos(_newBallPos.Pos());
	}
	//CGeoPoint point1(BallStatus::Instance()->ballChipLine().point1());
	//CGeoPoint tpoint2(BallStatus::Instance()->ballChipLine().point2());
	//CGeoPoint point2 = (point1 + ((tpoint2 - point1) * 400));
	//GDebugEngine::Instance()->gui_debug_line(point1,point2);
	//GDebugEngine::Instance()->gui_debug_x(_rawBallPos.Pos(), 1);
	//_newBallPos.SetPos(BallStatus::Instance()->ballChipLine().projection(_rawBallPos.Pos()));
	//GDebugEngine::Instance()->gui_debug_x(_newBallPos.Pos(),1);
	//GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-200, 100), std::to_string(BallStatus::Instance()->getBallToucher()).c_str(), 1);
	
	// 【#TODO】 更新敌我双方对于球的势能，越小越有利于拿球，贝叶斯滤波中有使用
	BestPlayer::Instance()->update(this); 
	
	// 【#TODO】 更新贝叶斯滤波器，评估目前比赛攻防形式
	MatchState::Instance()->update();

	DefenceInfo::Instance()->updateDefenceInfo(this);

	/////////////////////////////////////////////////////////////////////////////
	/// @brief Step 6: 更新裁判盒信息 及处理球数据相关的特殊情况
	/////////////////////////////////////////////////////////////////////////////
	CheckKickoffStatus(vInfo);
	int ref_mode = vInfo.mode;
	_last_xbox_pressed = ref_mode;
//==========================================================
//=============================原始手柄============================
	if (xboxCtrl.IsConnected()) {
		if(xboxCtrl.GetState().Gamepad.wButtons & XINPUT_GAMEPAD_Y) {
			ref_mode = PMIndirectYellow;
		} else if(xboxCtrl.GetState().Gamepad.wButtons & XINPUT_GAMEPAD_X) {
			ref_mode = PMIndirectBlue;
		} else if(xboxCtrl.GetState().Gamepad.wButtons & XINPUT_GAMEPAD_B) {
			ref_mode = PMHalt;
		} else if(xboxCtrl.GetState().Gamepad.wButtons & XINPUT_GAMEPAD_A) {
			ref_mode = PMStop;
		} else if(xboxCtrl.GetState().Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER){
			ref_mode = PMStart;
		} else if(xboxCtrl.GetState().Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER){
			ref_mode = PMTimeoutYellow;
		} else {
			ref_mode = _last_xbox_pressed;
		}
		_last_xbox_pressed = ref_mode;
	} 
//===============================end=======================================
	// 更新裁判盒信息，一般当且仅当比赛模式为停球状态时，判断球是否被踢出
	if (ref_mode >= PMStop && ref_mode< PMNone) {
		_gameState.transition(playModePair[ref_mode].ch, _ballKicked);
	}

	//更新裁判盒信息
	UpdateRefereeMsg();

	// 特殊情况一
	// 一些特殊比赛状态下，对于球的特殊处理，与场地的尺寸参数相关
	// 一般要求球看不到才予以处理
	if (!IS_SIMULATION) {
		if (_gameState.kickoff()) {				// 开球时
			if (!Ball().Valid() || Ball().Pos().dist(CGeoPoint(0,0)) > 20) {
				_ballPredictor.setPos(Cycle(),CGeoPoint(0,0));
				_ballPredictor.setVel(Cycle(),CVector(0,0));
			}
		}
		double penaltyX = Param::Field::PENALTY_MARK_X;

		if (_gameState.ourPenaltyKick()) {		// 我方点球时
			if (!Ball().Valid() || Ball().Pos().dist(CGeoPoint(penaltyX,0)) > 20) {
				_ballPredictor.setPos(Cycle(),CGeoPoint(penaltyX,0));
				_ballPredictor.setVel(Cycle(),CVector(0,0));
			}
		}

		if (_gameState.theirPenaltyKick()) {	// 对方点球时
			if (!Ball().Valid() || Ball().Pos().dist(CGeoPoint(-penaltyX,0)) > 20) {
				_ballPredictor.setPos(CGeoPoint(-penaltyX,0));
				_ballPredictor.setVel(Cycle(),CVector(0,0));
			}
		}
	}

	// 特殊情况二：
	// 红外有信息，若球没看到，则予以位置修正
	bool sensorBall = false;
	for (int i = 1; i <= Param::Field::MAX_PLAYER; i ++) {
		if (RobotSensor::Instance()->IsInfoValid(i) && RobotSensor::Instance()->IsInfraredOn(i)) {
			sensorBall = true;

			if (Ball().Valid()) {	// 球看到，作红外信号假象检查，因为通讯可能会丢
				if (Ball().Pos().dist(OurPlayer(i).Pos()) > Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE + 5) {
					RobotSensor::Instance()->ResetInraredOn(i);
				}
			} else {				// 球看不到，根据红外信号纠正球的位置
				_ballPredictor.setPos(OurPlayer(i).Pos() + Utils::Polar2Vector(8.5,OurPlayer(i).Dir()));
				_ballPredictor.setVel(Cycle(),CVector(0,0));
			}

			break;
		}
	}

	// 这一步是保证PlayInterface与OurPlayer中Valid的同步
	PlayInterface::Instance()->clearRealIndex();
	for (int i = 1; i <= Param::Field::MAX_PLAYER; ++i ) {
		PlayInterface::Instance()->setRealIndex(i, _ourPlayerPredictor[i-1].getResult(_timeCycle).realNum);
	}

	// 从googlebuf中获得对方的门将
	if(IS_SIMULATION){
		_theirGoalieStrategyNum = _theirGoalie;
	} else{
		for (int i = 1; i <= Param::Field::MAX_PLAYER; ++i) {
			if (_theirPlayerPredictor[i-1].getResult(_timeCycle).realNum == _theirGoalie) {
				_theirGoalieStrategyNum = i;
				break;
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////
	/// @brief Step 7: 向调试面板中显示一些必要的信息
	/////////////////////////////////////////////////////////////////////////////

	// 输出当前球的预测位置 ： 忽略球是否在场上
	GDebugEngine::Instance()->gui_debug_x(this->Ball().Pos(),COLOR_RED);
	// 输出当前球的预测速度 并 输出数值直观显示
	GDebugEngine::Instance()->gui_debug_line(this->Ball().Pos(),this->Ball().Pos()+this->Ball().Vel(),COLOR_ORANGE);
	char velbuf[20];
	const double outballspeed = Ball().Vel().mod();
	sprintf(velbuf, "%f", outballspeed);
	if (outballspeed <= 800){
		GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-320,150), velbuf, COLOR_BLACK);
	} else{
		GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-320,150), velbuf, COLOR_RED);
	}

	// 输出我方小车的预测位置 : 忽略车是否在场上
	for (int i = 1; i <= Param::Field::MAX_PLAYER; i ++) {
		GDebugEngine::Instance()->gui_debug_robot(OurPlayer(i).Pos(), OurPlayer(i).Dir());
	}
	// 输出我方小车的红外信号
	if (sensorBall) {
		GDebugEngine::Instance()->gui_debug_arc(Ball().Pos(), 4*Param::Field::BALL_SIZE, 0, 360, COLOR_PURPLE);
		GDebugEngine::Instance()->gui_debug_arc(Ball().Pos(), 2*Param::Field::BALL_SIZE, 0, 360, COLOR_PURPLE);
	}
	vision_to_decision.Signal();
	return ;
}

void CVisionModule::CheckKickoffStatus(const CServerInterface::VisualInfo& info)
{
	if (_gameState.canEitherKickBall()) {	// 若允许去踢球
		if (! _ballKicked ){	// 球没有被判断为踢出
			if (gameState().ourRestart()) {
				const double OUR_BALL_KICKEDBUFFER = 5 + 3 + 1;	
				const CVector ballMoved = Ball().Pos() - _ballPosSinceNotKicked;
				if( ballMoved.mod2() > OUR_BALL_KICKEDBUFFER * OUR_BALL_KICKEDBUFFER ){
					_ballKicked = true;
				}
			} else {
				CBestPlayer::PlayerList theirList =  BestPlayer::Instance()->theirFastestPlayerToBallList();
				if (theirList.empty()) {
					_ballKicked = false;
				} else {
					const double THEIR_BALL_KICKED_BUFFER = 5 + 5;
					const CVector ballMoved = Ball().Pos() - _ballPosSinceNotKicked;
					if( ballMoved.mod2() > THEIR_BALL_KICKED_BUFFER * THEIR_BALL_KICKED_BUFFER ){
						_ballKicked = true;
					}
				}
			}
				
		}
	} else {					// 球已经被判断为踢出
		_ballKicked = false;
		_ballPosSinceNotKicked = Ball().Pos();
	}

	return ;
}

void CVisionModule::CheckBothSidePlayerNum()
{
	// 统计我方实际在场上的小车个数
	_validNum = 0;
	int tempGoalieNum = PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie());
	for (int i = 1; i <= Param::Field::MAX_PLAYER; i++) {		
		if (OurPlayer(i).Valid() && i != tempGoalieNum) {	
			_validNum++;
		}
	}
	_validNum = _validNum>(Param::Field::MAX_PLAYER - 1)?(Param::Field::MAX_PLAYER - 1):_validNum;

	// 统计对方实际在场上的小车个数
	_TheirValidNum = 0;
	for (int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
		if (TheirPlayer(i).Valid())	{
			_TheirValidNum ++;
		}
	}
	_TheirValidNum = _TheirValidNum > Param::Field::MAX_PLAYER ? Param::Field::MAX_PLAYER : _TheirValidNum;

	return;
}

void CVisionModule::SetPlayerCommand(int num, const CPlayerCommand* pCmd)
{
	_ourPlayerPredictor[num-1].updateCommand(Cycle(), pCmd);
	CDribbleStatus* dribbleStatus = DribbleStatus::Instance();
	if( pCmd->dribble() ){
		dribbleStatus->setDribbleOn(pCmd->number(), Cycle(), Ball().Pos());
	}else{
		dribbleStatus->setDribbleOff(pCmd->number());
	}

	return ;
}

void CVisionModule::UpdateRefereeMsg()
{
	if (_lastRefereeMsg != _refereeMsg && _refereeMsg == "theirPenaltyKick") { // 记录当前是对方第几个点球
		_theirPenaltyNum++;
	}

	_lastRefereeMsg=_refereeMsg;
	
	
	if (! _gameState.canMove()) {
		_refereeMsg = "gameHalt";
	} else if( _gameState.gameOver()){
		_refereeMsg = "gameOver";

	//add by gty 16/6/11
	} else if( _gameState.isOurTimeout() || _gameState.isTheirTimeout()){
		_refereeMsg = "ourTimeout";
	} else if(!_gameState.allowedNearBall()){
		// 对方发球
		if(_gameState.theirIndirectKick()){
			_refereeMsg = "theirIndirectKick";
		} else if (_gameState.theirDirectKick()){
			_refereeMsg = "theirIndirectKick";
		} else if (_gameState.theirKickoff()){
			_refereeMsg = "theirKickOff";
		} else if (_gameState.theirPenaltyKick()){
			_refereeMsg = "theirPenaltyKick";
		} else{
			_refereeMsg = "gameStop";
		}
	} else if( _gameState.ourRestart()){
		if( _gameState.ourKickoff() ) {
			_refereeMsg = "ourKickOff";
		} else if(_gameState.penaltyKick()){
			_refereeMsg = "ourPenaltyKick";
		} else if(_gameState.ourIndirectKick()){
			_refereeMsg = "ourIndirectKick";
		} else if(_gameState.ourDirectKick()){
			_refereeMsg = "ourIndirectKick";
		}
	} else {
		_refereeMsg ="";
	}
	//cout<<"_refereeMsg2:"<<_refereeMsg<<endl;
}

const string CVisionModule::GetCurrentRefereeMsg() const{
	return _refereeMsg;
}

const string CVisionModule::GetLastRefereeMsg() const{
	return _lastRefereeMsg;
}


void CVisionModule::judgeBallVelStable(){
	if (ballVelValid())
	{
		if (fabs(Utils::Normalize(this->Ball().Vel().dir()-this->Ball(_lastTimeCycle-2).Vel().dir()))>Param::Math::PI*10/180 && this->Ball().Vel().mod()>20)
		{
			_ballVelChangeCouter++;
			_ballVelChangeCouter=min(_ballVelChangeCouter,4);
		}else{
			_ballVelChangeCouter--;
			_ballVelChangeCouter=max(_ballVelChangeCouter,0);
		}
		if (_ballVelChangeCouter>=3){
			_ballVelDirChanged=true;		
		}
		if (_ballVelChangeCouter==0)
		{
			_ballVelDirChanged=false;
		}
	}

}

bool CVisionModule::ballVelValid(){
	if (!_rawBallPos.Valid() || fabs(_rawBallPos.X() - _lastRawBallPos.X())<0.0000000001 )
	{
		return false;
	}else{
		return true;
	}
}