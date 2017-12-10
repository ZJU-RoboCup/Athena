#include "BallPredictor.h"
#include <utils.h>
#include <fstream>
#include <istream>
#include "geometry.h"
#include <tinyxml/ParamReader.h>
#include "WorldDefine.h"
#include "GDebugEngine.h"

namespace {
	const double VERBOSE_MODE = false;
	const int MAX_BALL_LOST_TIME = 30;
	const int NORMAL_BALL_LOST_TIME = 8;
	const double ZERRO_SPEED = 10; // 速度小于这个值认为静止
	bool IS_ERROR_RECORD = false; //比较Neural的预测效果
	bool IS_SIMULATION = false;	//是否是仿真
	double _lastValidDir = 0; // 记录球速大于100cm/s时的速度方向，作为球速低时的速度方向值。以免球速低时方向跳动；（效果还不确定，interceptAndShooter验证情况不佳）

	bool LOG_BALL_SPEED = true;
	std::ofstream ballSpeedLog;

	double BALL_DELC_CHANGE_POINT = 250;
	double BALL_FAST_DEC = 360;
	double BALL_SLOW_DEC = 27.5;
}


CBallPredictor::CBallPredictor() : _ballInvalidMovedCycle(0), _ballLostTime(2000), _hasCollision(false), _visibility(0), _activity(0),_lastRawBallPos(0,0)
{
	DECLARE_PARAM_READER_BEGIN(General)
		READ_PARAM(IS_SIMULATION)
	DECLARE_PARAM_READER_END
}

CBallPredictor::~CBallPredictor()
{
	if ( ballSpeedLog.is_open() )
		ballSpeedLog.close();
}

void CBallPredictor::updateVision(int cycle, const PosT& raw_pos, bool invert,bool ballFilterSwitch)
{
	// 每次处理一个_visionLogger里的数据，只用来保留预测输出
	// 该函数的处理结果就是更新thisCycle帧里面的球信息
	BallVisionData& thisCycle = _visionLogger.getVision(cycle);
	const int invertFactor = invert ? -1 : 1;

	// 【TODO】根据原始球是否看到不同的处理,球看不到的处理需要关注
	if (raw_pos.valid) {	// 若看到球，直接更新位置
		_ballLostTime = 0;
		thisCycle.SetRawPos(raw_pos.x * invertFactor, raw_pos.y * invertFactor);
		thisCycle.cycle = cycle;
	} else {				// 没看到球，猜测一个位置
		++ _ballLostTime;
		for (int i=1; i<NORMAL_BALL_LOST_TIME*1.5; ++i) {
			if (_visionLogger.visionValid(cycle - i)) {
				for (int j=cycle-i; j<cycle; ++j) {
					const MobileVisionT& previousVision = _visionLogger.getVision(j);
					BallVisionData& nextVision = _visionLogger.getVision(j+1);
					nextVision.SetRawPos(previousVision.RawPos() + previousVision.Vel() / Param::Vision::FRAME_RATE);// TODO add velocity reduce.
					nextVision.cycle = j+1;
				}
				break;
			}
		}
	}

	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	// 判断球是否有丢失，需要结合球的碰撞模型 setCollisionResult
	bool collisionRelate = (_hasCollision && _ballLostTime == 0) 
							|| (!_hasCollision && _ballLostTime < NORMAL_BALL_LOST_TIME);
	// 感觉比赛时的球滤波问题应该是球碰撞模型的问题，请关注_hasCollision标志位的设定接口
	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	
	if (collisionRelate && checkValid(cycle)) {
		/////////////////////////////////////////////////////////////////////////
		/// 1.进行Kalman滤波，估计球的位置以及球速
		const CGeoPoint& pos = thisCycle.RawPos();
		//InaccurateBallPredictor::Instance()->updateVision(cycle, pos.x(), pos.y()); // 同时更新一个不精确模型，用来守门和接球
		//double ballPos[2] = { pos.x(), pos.y() };
		//double ballPosVel[2][2];	// 第一列: 位置;  第二列: 速度
		//_ballFilter.updatePosition(ballPos, ballPosVel);
		auto& tempMatrix = ballFilterSwitch ? _ballKalmanFilter.follow(pos.x(),pos.y()) : _ballKalmanFilter.update(pos.x(),pos.y());
		// result.SetPos(CGeoPoint(ballPosVel[0][0], ballPosVel[1][0])); // 滤波更新出来的位置,可能不准
		//const CVector ballVel(ballPosVel[0][1],ballPosVel[1][1]); // 球的速度(数组第二列)// not use the method, use new method.
		const CVector ballVel(tempMatrix(2, 0)*Param::Vision::FRAME_RATE, tempMatrix(3, 0)*Param::Vision::FRAME_RATE);
		
		// 根据Kalman滤波的结果预测将来球的位置和速度
		if ( LOG_BALL_SPEED ){
			if ( !ballSpeedLog.is_open() ){
				ballSpeedLog.open("ballSpeedLog.txt", std::ios_base::out);
			}
			ballSpeedLog<<ballVel.mod()<<"  "<<ballVel.dir()<<"  "
				<<pos.x()<<"  "<<pos.y()<< "  "<< tempMatrix(0,0) <<"  "<< tempMatrix(1,0) << std::endl;
		}


		thisCycle.SetVel(ballVel);
		thisCycle.SetPos(pos);

		/////////////////////////////////////////////////////////////////////////
		/// 2.延时补偿，根据延时帧率将位置和速度进行修正
		for( int i=0; i<Param::Latency::TOTAL_LATED_FRAME; ++i ){
			thisCycle.SetPos(thisCycle.Pos() + thisCycle.Vel() / Param::Vision::FRAME_RATE);
			//thisCycle.SetVel(thisCycle.Vel() * ( 1 + Param::Field::BALL_DECAY / Param::Vision::FRAME_RATE ));
			CVector uniVec = thisCycle.Vel()/(thisCycle.Vel().mod()+1.0);
			/*if ( thisCycle.Vel().mod() > 315 )
				thisCycle.SetVel( uniVec * ( thisCycle.Vel().mod() - 298/Param::Vision::FRAME_RATE ));
			else if ( thisCycle.Vel().mod() > 5 )
				thisCycle.SetVel(uniVec * ( thisCycle.Vel().mod() - 36/Param::Vision::FRAME_RATE ));*/
			if ( thisCycle.Vel().mod() > BALL_DELC_CHANGE_POINT )
				thisCycle.SetVel( uniVec * ( thisCycle.Vel().mod() - BALL_FAST_DEC/Param::Vision::FRAME_RATE ));
			else if ( thisCycle.Vel().mod() > 5 )
				thisCycle.SetVel(uniVec * ( thisCycle.Vel().mod() - BALL_SLOW_DEC/Param::Vision::FRAME_RATE ));
			else{
				thisCycle.SetVel(CVector(0,0));
			}
		}
		thisCycle.SetValid(true);

		/////////////////////////////////////////////////////////////////////////
		/// 3.估计球速大小及方向，特别是方向
		const int ACTIVITY = _activity;
		if (ACTIVITY > 3) {
			// 计算球速方向: 最小二乘方法
			double x[MAX_LOGS], y[MAX_LOGS];
			for (int i=0; i < ACTIVITY ;i++) {
				x[i] = _visionLogger.getVision(cycle-i).RawPos().x();
				y[i] = _visionLogger.getVision(cycle-i).RawPos().y();
			}
			// modified by zhyaic 2012.5.18
			double movingDirection = thisCycle.Vel().dir();
			_lastValidDir = movingDirection;
			thisCycle.SetVel(thisCycle.Vel().mod()*std::cos(movingDirection),thisCycle.Vel().mod()*std::sin(movingDirection));
			//std::cout<<"LSQ: "<<Utils::Rad2Deg(movingDirection)<< Param::Output::NewLineCharacter;
		} else if(_cycle > 5) {	// _cycyle在此并没有进行维护？？
			double predictDir = _lastValidDir;
			double velMod = thisCycle.Vel().mod();
			if (velMod < 0.1) {
				velMod = 0.1;
			}
			thisCycle.SetVel(velMod*std::cos(predictDir),velMod*std::sin(predictDir));
		}
	} else {	// 球丢失情况下的预测
		predictLost(cycle);
	}

	// 更新球的可见度与活动度
	if (raw_pos.valid) {
		_visibility = 100;
	} else if (_visibility > 0) {
		--_visibility;
	}

	if (thisCycle.Vel().mod2() > 100) {
		_activity++;
		if (_activity > MAX_LOGS) {
			_activity = MAX_LOGS;
		}
	} else if (_activity > 0) {
		--_activity;
	}

	_hasCollision = false;

	return;
}

void CBallPredictor::predictLost(int cycle)
{
	BallVisionData& thisCycle = _visionLogger.getVision(cycle);
	if (_ballLostTime < MAX_BALL_LOST_TIME /*&& _visionLogger.visionValid(cycle - 1)*/) {
		BallVisionData& thisCycle = _visionLogger.getVision(cycle);
		const BallVisionData& lastCycle = _visionLogger.getVision(cycle - 1);

		thisCycle.SetPos(lastCycle.Pos() + lastCycle.Vel() / Param::Vision::FRAME_RATE); // 位置
		thisCycle.SetVel(lastCycle.Vel() * ( 1 + Param::Field::BALL_DECAY / Param::Vision::FRAME_RATE )); // 速度
		thisCycle.SetValid(true);
	} else {
		// 很长时间没有看到球了,保持原来的位置?
		thisCycle.SetValid(false); // 信息不可用			
	}
	//_ballFilter.reset();

	return ;	
}

bool CBallPredictor::checkValid(int cycle)
{
	if (! _visionLogger.visionValid(cycle)) {
		return false;
	}

	if (! _visionLogger.visionValid(cycle - 1) || ! _visionLogger.getVision(cycle - 1).Valid()) {
		return true;	//如果上个周期预测不可用,那么总是信任这个周期的信息
	}

	const double BALL_OUT_BUFFER = (Param::Rule::Version == 2003) ? -20 : -50;
	const double MAX_BALL_MOVE_DIST_PER_CYCLE = 30; //每一祯球的最大位移
	BallVisionData& thisCycle = _visionLogger.getVision(cycle);
	const BallVisionData& lastCycle = _visionLogger.getVision(cycle-1);
	if (Utils::OutOfField(thisCycle.RawPos(), BALL_OUT_BUFFER)) {
		return false; // 球不可能在场外,故信息不对
	}
	
	if ((thisCycle.RawPos() - lastCycle.RawPos()).mod2() > MAX_BALL_MOVE_DIST_PER_CYCLE * MAX_BALL_MOVE_DIST_PER_CYCLE) {
		if (++_ballInvalidMovedCycle < 5) {
			thisCycle.SetRawPos(lastCycle.RawPos() + lastCycle.Vel() / Param::Vision::FRAME_RATE);//处理视觉杂点的过程，根据试验修正
			return false; // 不可能这么快		
		}
	} else {
		_ballInvalidMovedCycle = 0;		
	}

	return true;
}

void CBallPredictor::setCollisionResult(int cycle, const MobileVisionT& ball)
{
	MobileVisionT& oldBall = _visionLogger.getVision(cycle);
	oldBall = ball;
	_hasCollision = true;

	return ;
}