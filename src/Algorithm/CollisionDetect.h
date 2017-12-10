#pragma once
#include "VisionModule.h"
#include "param.h"
#include "DataQueue.hpp"
#include <MultiThread.h>

#define MAX_FRAME_NUM 20  //最大存储帧数
struct VisionInfo {
	CGeoPoint BallPos;
	CGeoPoint OurPlayer[Param::Field::MAX_PLAYER];
	CGeoPoint TheirPlayer[Param::Field::MAX_PLAYER];
};

class CCollisionDetect
{
public:
	CCollisionDetect();
	virtual ~CCollisionDetect();
	void initialize(CVisionModule*);
	void visionAlart();
	static CThreadCreator::CallBackReturnType THREAD_CALLBACK doCollisionDetect(CThreadCreator::CallBackParamType lpParam);

private:
	void recordPos(int);
	void analyzeData();
	bool IsOnEdge(CGeoPoint);

	CThreadCreator *_collision_detect_thread = 0;
	CVisionModule* _pVision;			///<图像指针
	DataQueue<VisionInfo> _VisionQueue;
	int  LinePoint[MAX_FRAME_NUM];
	int _RecordFlag = 0;
	int _RecordIndex = -1;
	int PointN;
	int LastTouch=0;//0 for NOT sure; 1 for our ;2 for their; 3 for wall;
	int LastTouchNumber = 0;//car number; 0 for not sure or wall
};
typedef NormalSingleton< CCollisionDetect > CollisionDetect;

