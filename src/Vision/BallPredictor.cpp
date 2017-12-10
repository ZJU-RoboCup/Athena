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
	const double ZERRO_SPEED = 10; // �ٶ�С�����ֵ��Ϊ��ֹ
	bool IS_ERROR_RECORD = false; //�Ƚ�Neural��Ԥ��Ч��
	bool IS_SIMULATION = false;	//�Ƿ��Ƿ���
	double _lastValidDir = 0; // ��¼���ٴ���100cm/sʱ���ٶȷ�����Ϊ���ٵ�ʱ���ٶȷ���ֵ���������ٵ�ʱ������������Ч������ȷ����interceptAndShooter��֤������ѣ�

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
	// ÿ�δ���һ��_visionLogger������ݣ�ֻ��������Ԥ�����
	// �ú����Ĵ��������Ǹ���thisCycle֡���������Ϣ
	BallVisionData& thisCycle = _visionLogger.getVision(cycle);
	const int invertFactor = invert ? -1 : 1;

	// ��TODO������ԭʼ���Ƿ񿴵���ͬ�Ĵ���,�򿴲����Ĵ�����Ҫ��ע
	if (raw_pos.valid) {	// ��������ֱ�Ӹ���λ��
		_ballLostTime = 0;
		thisCycle.SetRawPos(raw_pos.x * invertFactor, raw_pos.y * invertFactor);
		thisCycle.cycle = cycle;
	} else {				// û�����򣬲²�һ��λ��
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
	// �ж����Ƿ��ж�ʧ����Ҫ��������ײģ�� setCollisionResult
	bool collisionRelate = (_hasCollision && _ballLostTime == 0) 
							|| (!_hasCollision && _ballLostTime < NORMAL_BALL_LOST_TIME);
	// �о�����ʱ�����˲�����Ӧ��������ײģ�͵����⣬���ע_hasCollision��־λ���趨�ӿ�
	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	
	if (collisionRelate && checkValid(cycle)) {
		/////////////////////////////////////////////////////////////////////////
		/// 1.����Kalman�˲����������λ���Լ�����
		const CGeoPoint& pos = thisCycle.RawPos();
		//InaccurateBallPredictor::Instance()->updateVision(cycle, pos.x(), pos.y()); // ͬʱ����һ������ȷģ�ͣ��������źͽ���
		//double ballPos[2] = { pos.x(), pos.y() };
		//double ballPosVel[2][2];	// ��һ��: λ��;  �ڶ���: �ٶ�
		//_ballFilter.updatePosition(ballPos, ballPosVel);
		auto& tempMatrix = ballFilterSwitch ? _ballKalmanFilter.follow(pos.x(),pos.y()) : _ballKalmanFilter.update(pos.x(),pos.y());
		// result.SetPos(CGeoPoint(ballPosVel[0][0], ballPosVel[1][0])); // �˲����³�����λ��,���ܲ�׼
		//const CVector ballVel(ballPosVel[0][1],ballPosVel[1][1]); // ����ٶ�(����ڶ���)// not use the method, use new method.
		const CVector ballVel(tempMatrix(2, 0)*Param::Vision::FRAME_RATE, tempMatrix(3, 0)*Param::Vision::FRAME_RATE);
		
		// ����Kalman�˲��Ľ��Ԥ�⽫�����λ�ú��ٶ�
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
		/// 2.��ʱ������������ʱ֡�ʽ�λ�ú��ٶȽ�������
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
		/// 3.�������ٴ�С�������ر��Ƿ���
		const int ACTIVITY = _activity;
		if (ACTIVITY > 3) {
			// �������ٷ���: ��С���˷���
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
		} else if(_cycle > 5) {	// _cycyle�ڴ˲�û�н���ά������
			double predictDir = _lastValidDir;
			double velMod = thisCycle.Vel().mod();
			if (velMod < 0.1) {
				velMod = 0.1;
			}
			thisCycle.SetVel(velMod*std::cos(predictDir),velMod*std::sin(predictDir));
		}
	} else {	// ��ʧ����µ�Ԥ��
		predictLost(cycle);
	}

	// ������Ŀɼ�������
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

		thisCycle.SetPos(lastCycle.Pos() + lastCycle.Vel() / Param::Vision::FRAME_RATE); // λ��
		thisCycle.SetVel(lastCycle.Vel() * ( 1 + Param::Field::BALL_DECAY / Param::Vision::FRAME_RATE )); // �ٶ�
		thisCycle.SetValid(true);
	} else {
		// �ܳ�ʱ��û�п�������,����ԭ����λ��?
		thisCycle.SetValid(false); // ��Ϣ������			
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
		return true;	//����ϸ�����Ԥ�ⲻ����,��ô��������������ڵ���Ϣ
	}

	const double BALL_OUT_BUFFER = (Param::Rule::Version == 2003) ? -20 : -50;
	const double MAX_BALL_MOVE_DIST_PER_CYCLE = 30; //ÿһ��������λ��
	BallVisionData& thisCycle = _visionLogger.getVision(cycle);
	const BallVisionData& lastCycle = _visionLogger.getVision(cycle-1);
	if (Utils::OutOfField(thisCycle.RawPos(), BALL_OUT_BUFFER)) {
		return false; // �򲻿����ڳ���,����Ϣ����
	}
	
	if ((thisCycle.RawPos() - lastCycle.RawPos()).mod2() > MAX_BALL_MOVE_DIST_PER_CYCLE * MAX_BALL_MOVE_DIST_PER_CYCLE) {
		if (++_ballInvalidMovedCycle < 5) {
			thisCycle.SetRawPos(lastCycle.RawPos() + lastCycle.Vel() / Param::Vision::FRAME_RATE);//�����Ӿ��ӵ�Ĺ��̣�������������
			return false; // ��������ô��		
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