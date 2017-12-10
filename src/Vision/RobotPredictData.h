#ifndef _ROBOT_PREDICT_DATA_H_
#define _ROBOT_PREDICT_DATA_H_
#include <geometry.h>
#include "WorldDefine.h"
#include <iostream>
#include <stdio.h>
#include <float.h>
// �Ӿ���Ϣ
struct RobotRawVisionData{
	RobotRawVisionData():cycle(0),x(0),y(0),dir(0),xVel(0),yVel(0),rotVel(0){}
	int cycle;
	double x; // x ����
	double y; // y ����
	double dir; // ���峯��
	double xVel; // x �����ٶ�
	double yVel; // y �����ٶ�
	double rotVel; // ��ת�ٶ�
};
// ��¼�����˵�ԭʼ�Ӿ���Ϣ
class CRobotRawVisionLogger{
public:
	bool visionValid(int cycle) const { return _vision[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS].cycle == cycle; }
	const RobotRawVisionData& getVision(int cycle) const { return _vision[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS]; }
	void setVision(int cycle, double x, double y, double dir, double xVel, double yVel, double rotVel)
	{
		_vision[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS].cycle = cycle;
		_vision[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS].x = x;
		_vision[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS].y = y;
		_vision[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS].dir = dir;
		_vision[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS].xVel = xVel;
		_vision[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS].yVel = yVel;
		_vision[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS].rotVel = rotVel;
	}
private:
	static const int MAX_INFORMATIONS = 16; // ����n�����ڵ���Ϣ
	RobotRawVisionData _vision[MAX_INFORMATIONS];
};
// ����
struct RobotCommandEffect{
	int cycle; // ���ڵ����ڣ�������֤�Ƿ���Ч
	CVector vel; // ƽ���ٶ�
	double rot_speed; // ��ת�ٶ�
};
// ��¼�����˵�����
class CRobotCommandLogger{
public:
	bool commandValid(int cycle) const { return _command[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS].cycle == cycle; }
	const RobotCommandEffect& getCommand(int cycle) const { return _command[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS];}
	void setCommand(int cycle, const CVector& vel, double rot_speed)
	{
		_command[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS].cycle = cycle;

		if ( _finite(vel.mod()) && _finite(rot_speed) ){
			_command[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS].vel = vel;
			_command[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS].rot_speed = rot_speed;
		}
		else{
			printf("%.2f , %.2f, %.2f command error!\n", vel.x(), vel.y(), rot_speed);
			_command[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS].vel = CVector();
			_command[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS].rot_speed = 0.0;
		}
	}
	void print(int cycle)
	{
       
	std::cout<<" cycle: "<<cycle<<std::endl;
	for ( int i = 0; i < MAX_INFORMATIONS; i++) {
		std::cout<<"i:"<<i<<" RobotComandLogger: cycle = "<< _command[(cycle-i+MAX_INFORMATIONS) % MAX_INFORMATIONS].cycle << " velx: "<<_command[i].vel.x()
			<<" vely: "<<_command[i].vel.y()<<" rotvel: "<<_command[i].rot_speed<<std::endl;
		}
	}
private:
	static const int MAX_INFORMATIONS = 16; // ����n�����ڵ���Ϣ
	RobotCommandEffect _command[MAX_INFORMATIONS];
};
// ��¼Ԥ������Ӿ���Ϣ
struct RobotVisionData : public PlayerVisionT{
	int cycle;
	int realNum;
};
class CRobotVisionLogger{
public:
	bool visionValid(int cycle) const { return _vision[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS].cycle == cycle; }
	const RobotVisionData& getVision(int cycle) const { return _vision[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS]; }
	RobotVisionData& getVision(int cycle) { return _vision[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS]; }
	void setVision(int cycle, const RobotVisionData& vision, int realNum)
	{
		_vision[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS] = vision;
		_vision[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS].cycle = cycle;
		_vision[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS].realNum = realNum;
	}
private:
	static const int MAX_INFORMATIONS = 16; // ����n�����ڵ���Ϣ
	RobotVisionData _vision[MAX_INFORMATIONS];
};
// ��¼ͳ����Ϣ
struct RobotVisionErrorData{
	int cycle; // ���ڵ�����
	double distanceTravelled; // �ߵľ���
	double rotationTravelled; // ת�ĽǶ�
	double predictedTranslationalError; // ƽ�����
	double predictedRotationalError; // ת�����
	double normalizedTranslationalError;
	double normalizedRotationalError;
	bool resetTranslationalVelocity;
	bool resetRotationalVelocity;
};
class CRobotVisionErrorLogger{
public:
	bool errorValid(int cycle) const { return _error[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS].cycle == cycle; }
	const RobotVisionErrorData& getError(int cycle) const { return _error[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS];}
	RobotVisionErrorData& getError(int cycle) { return _error[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS];}
	void setError(int cycle, const RobotVisionErrorData& error)
	{
		_error[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS] = error;
		_error[(cycle+MAX_INFORMATIONS) % MAX_INFORMATIONS].cycle = cycle;
	}
private:
	static const int MAX_INFORMATIONS = 16; // ����n�����ڵ���Ϣ
	RobotVisionErrorData _error[MAX_INFORMATIONS];
};
#endif // _ROBOT_PREDICT_DATA_H_