/**
* @file BallPredictor.h
* ���ļ�Ϊ��λ����ϢԤ���ඨ��.
* @date $Date: 2004/06/20 11:52:30 $
* @version $Revision: 1.6 $
* @author peter@mail.ustc.edu.cn
*/

#ifndef _BALL_PREDICTOR_H_
#define _BALL_PREDICTOR_H_
#include "FilteredObject.h"
#include <geometry.h>
#include "KalmanFilter.h"
#include "WorldDefine.h"
#include <vector>
#include <server.h>
#include <param.h>
namespace{
	const int VALID_NUM = 7;   //  ����С������Ҫ��֡��
	const double MAX_SPEED =  1000;
	const double MAX_DIST = 1000.0/Param::Vision::FRAME_RATE*4;
	const int MAX_LOGS = 16;
}

struct BallVisionData : public MobileVisionT{
	int cycle;
};

class CBallVisionLogger{
public:
	bool visionValid(int cycle) const { return _vision[cycle % MAX_INFORMATIONS].cycle == cycle; }
	const BallVisionData& getVision(int cycle) const { return _vision[cycle % MAX_INFORMATIONS]; }
	BallVisionData& getVision(int cycle) { return _vision[cycle % MAX_INFORMATIONS]; }
	void setVision(int cycle, const BallVisionData& vision)
	{
		_vision[cycle % MAX_INFORMATIONS] = vision;
		_vision[cycle % MAX_INFORMATIONS].cycle = cycle;
	}

private:
	static const int MAX_INFORMATIONS = MAX_LOGS; // ����n�����ڵ���Ϣ
	BallVisionData _vision[MAX_INFORMATIONS]; //����Ԥ�����ʷ��Ϣ
};
/**
* CBallPredictor.
* �����Ӿ�Ԥ�����λ����Ϣ
*/
class CBallPredictor{
public:
	CBallPredictor(); // ���캯��
	~CBallPredictor();
	void updateVision(int cycle, const PosT& pos, bool invert,bool); // �����Ӿ���Ϣ
	int ballLostTime() const{ return _ballLostTime; }
	BallVisionData& getData(int cycle) { return _visionLogger.getVision(cycle); }
	const MobileVisionT& getResult(int cycle) const { return _visionLogger.getVision(cycle); }
	void setCollisionResult(int cycle, const MobileVisionT& ball);
	int visibility() const { return _visibility; }
	int activity() const { return _activity; }
	void setPos(const CGeoPoint & pos){
		_visionLogger.getVision(_cycle).SetPos(pos);
	}
	void setPos(int cycle,const CGeoPoint & pos){
		_visionLogger.getVision(cycle).SetPos(pos);
	}
	void setVel(int cycle,const CVector & vel){
		_visionLogger.getVision(cycle).SetVel(vel);
	}
	bool isSpeedError()const { return _errorSpeed; };
protected:
	void predictLost(int cycle); // �򿴲�����ʱ��Ԥ�����λ��
	bool checkValid(int cycle); // ȥ������������
private:
	//FilteredObject _ballFilter; // KalmanFiter Old
	KalmanFilter _ballKalmanFilter; // Kalman Filter New 23/11/2017 --mark
	CBallVisionLogger _visionLogger;
	MobileVisionT _ballLinePredictData[60];
	int _ballLostTime; // ��������Ĵ���
	int _ballInvalidMovedCycle; // �����Ϣ�������������
	int _visibility, _activity; // �ɼ��Ⱥͻ��
	bool _hasCollision;
	int _cycle;
	bool _errorSpeed;
	CGeoPoint _lastRawBallPos; //filted
	PosT _visionDataQueue[VALID_NUM];  // ��ʷ����
};
#endif // _BALL_PREDICTOR_H_

