/**
* @file RobotPredictor.h
* ���ļ�Ϊ������λ����ϢԤ����Ķ���.
* @date $Date: 2004/06/18 01:50:22 $
* @version $Revision: 1.12 $
* @author peter@mail.ustc.edu.cn
*/

#ifndef _ROBOT_PREDICTOR_H_
#define _ROBOT_PREDICTOR_H_

#include "WorldDefine.h"
#include "FilteredObject.h"
#include "RobotPredictData.h"
#include "RobotPredictError.h"
#include "CollisionSimulator.h"
#include <PlayerCommand.h>
/**
* CRobotPredictor.
* ͨ�������˵��������Ӿ�ȷ�������˵�λ����Ϣ
*/
class CNeuralNetwork;
class CRobotPredictor{
public:
	CRobotPredictor();
	CRobotPredictor (bool isHasRotaion);
	void updateCommand(int cycle, const CPlayerCommand* cmd); // ��������
	void updateVision(int cycle, const VehicleInfoT& player, const MobileVisionT& ball, bool invert, int realNum); // �����Ӿ���Ϣ
	void updateOurVision(int cycle, const VehicleInfoT& player, const MobileVisionT& ball, bool invert, int realNum); // �����Ӿ���Ϣ
	void updateTheirVision(int cycle, const VehicleInfoT& player, const MobileVisionT& ball, bool invert, int realNum); // �����Ӿ���Ϣ
     
	CRobotCommandLogger& getCommandLogger(){return _commandLogger;}
	const RobotVisionData& getResult(int cycle) const { return _visionLogger.getVision(cycle); }
	const RobotRawVisionData& getRawData(int cycle) const { return _rawVisionLogger.getVision(cycle);}
	RobotVisionData& getData(int cycle){ return _visionLogger.getVision(cycle); }
	const CVector getRawSpeed(int cycle) const {return CVector(2*_filterData.getVision(cycle).xVel,2*_filterData.getVision(cycle).yVel);}
	int lostTime() const{ return _robotLostTime; }
	bool collideWithBall() const { return _collisionSimulator.hasCollision(); }            // ֻ�ǵ����ؿ����Ƿ���ײ����ײ����
	const MobileVisionT& ballCollidedResult() const { return _collisionSimulator.ball(); }
	void setIsHasRotation(bool isHasRot) { _isHasRotation = isHasRot; }
protected:
	void updateRawVision(int cycle, double x, double y, double dir, bool seen);
	bool checkValid(int cycle, const CGeoPoint& pos);
	void predictLost(int cycle, const MobileVisionT& ball); // Ԥ�⿴��������Ϣ
private:
	FilteredObject _robotFilter; // KalmanFilter
	FilteredObject _robotRotationFilter; // ����robot�����˲�
	int _robotLostTime; // ��������������
	bool _isHasRotation; // С���Ƿ���нǶ�(����)��Ϣ
	CRobotCommandLogger _commandLogger; // �����¼
	CRobotRawVisionLogger _rawVisionLogger; // ԭʼ�Ӿ���Ϣ��¼
	CRobotRawVisionLogger _filterData;
	CRobotVisionLogger _visionLogger; // Ԥ�������Ϣ��¼
	CRobotPredictError _predictError; // Ԥ��������
	CCollisionSimulator _collisionSimulator;
	RobotCommandEffect cmd;
};
#endif // _ROBOT_PREDICTOR_H_
