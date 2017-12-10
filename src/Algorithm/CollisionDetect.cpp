#include "CollisionDetect.h"
#include "Semaphore.h"
#include <iostream> 
#include "GDebugEngine.h"
#include <stdlib.h>

#define SPLIT_THRESHOLD 4 //split ��ֵ
extern Semaphore vision_finish_semaphore;

CCollisionDetect::CCollisionDetect():_VisionQueue(MAX_FRAME_NUM)
{
	_pVision = NULL;
}


CCollisionDetect::~CCollisionDetect()
{
}

void CCollisionDetect::initialize(CVisionModule* pVision) {
	_pVision = pVision;
	// �����߳�
	_collision_detect_thread = new CThreadCreator(doCollisionDetect, 0);
	return;
}

void CCollisionDetect::visionAlart()
{
	//detect whether the ball in 30cm of vechile
	CGeoPoint ballPos = _pVision->Ball().RawPos();
	recordPos(0);
	int flag = 0;
	for (int i = 0; i < Param::Field::MAX_PLAYER; i++)
		if (ballPos.dist(_pVision->OurPlayer(i + 1).RawPos()) < 30 ||
			ballPos.dist(_pVision->TheirPlayer(i + 1).RawPos()) < 30 ||
			IsOnEdge(ballPos)) {
			flag = 1;
			break;
		}
	if (flag==1) analyzeData();

}

CThreadCreator::CallBackReturnType THREAD_CALLBACK CCollisionDetect::doCollisionDetect(CThreadCreator::CallBackParamType lpParam) {
	while (true) {
		vision_finish_semaphore.Wait();
		CollisionDetect::Instance()->visionAlart();
	}
}

void CCollisionDetect::recordPos(int index)
{
	VisionInfo Temp;
	for (int i = 0; i < Param::Field::MAX_PLAYER; i++) {
		if (_pVision->OurPlayer(i + 1).Valid())
		{
			Temp.OurPlayer[i] = _pVision->OurPlayer(i + 1).RawPos();
			//std::cout << "have detect our player num" << i << "\t raw@" << Temp.OurPlayer[i] <<  "\t@"<< _pVision->OurPlayer(i + 1).Pos() << std::endl;
		}
		else {
			Temp.OurPlayer[i].setX(9999);
			Temp.OurPlayer[i].setY(9999);
		}
		if (_pVision->TheirPlayer(i + 1).Valid())
		{
			Temp.TheirPlayer[i] = _pVision->TheirPlayer(i + 1).RawPos();
		}
		else {
			Temp.TheirPlayer[i].setX(9999);
			Temp.TheirPlayer[i].setY(9999);
		}
	}
	Temp.BallPos = _pVision->Ball().RawPos();
	_VisionQueue.push(Temp);
}

void CCollisionDetect::analyzeData()
{
	//use split andf merge algorithm
	LinePoint[1] = -MAX_FRAME_NUM + 1; LinePoint[2] = 0;
	PointN = 2;
	while (true) {
		int pi = 1, maxi = 1, i = -MAX_FRAME_NUM + 1;
		double maxDis = -1;
		while (pi < PointN) {
			const auto& p1 = _VisionQueue[LinePoint[pi]], p2 = _VisionQueue[LinePoint[pi + 1]];
			const auto& q = _VisionQueue[i];
			CGeoSegment PassLine(p1.BallPos, p2.BallPos);
			double d = q.BallPos.dist(PassLine.projection(q.BallPos));
			//std::cout << "ball" << i << " @" << q.BallPos << "\t with d:" << d << std::endl;
			if (d > maxDis && d > SPLIT_THRESHOLD && i != LinePoint[pi] && i != LinePoint[pi + 1]) {
				maxi = i;
				maxDis = d;
			}
			if (i == LinePoint[pi + 1]) pi++;
			i++;
		}
		//std::cout << "-----------------\n";
		if (maxi == 1) break;
		for (i = PointN; i > 0; i--)
		{
			if (LinePoint[i] < maxi) break;
			LinePoint[i + 1] = LinePoint[i];
		}
		PointN++; LinePoint[i + 1] = maxi;
	}
	//std::cout << "\tPointN:" << PointN << "\t" << _VisionQueue[LinePoint[PointN]].BallPos << std::endl;
	if (PointN > 2) {
		int OurTouchNum = -1, TheirTouchNum = -1, j = PointN - 1;
		double OurTouchDis = 20.0, TheirTouchDis = 20.0;
		//std::cout << "found PointN=" << PointN << "\tball pos" << _VisionQueue[LinePoint[j]].BallPos << std::endl;
		for (int i = 0; i < Param::Field::MAX_PLAYER; i++)
		{
			if (_VisionQueue[LinePoint[j]].BallPos.dist(_VisionQueue[LinePoint[j]].OurPlayer[i]) < OurTouchDis)
			{
				OurTouchDis = _VisionQueue[LinePoint[j]].BallPos.dist(_VisionQueue[LinePoint[j]].OurPlayer[i]);
				OurTouchNum = i;
			}
			if (_VisionQueue[LinePoint[j]].BallPos.dist(_VisionQueue[LinePoint[j]].TheirPlayer[i]) < TheirTouchDis)
			{
				//std::cout << "the ball " << j - 1 << "th touch THEIR player No." << i << "@" << _VisionQueue[LinePoint[j]].TheirPlayer[i] << "WITH D:" << _VisionQueue[LinePoint[j]].BallPos.dist(_VisionQueue[LinePoint[j]].TheirPlayer[i]) << std::endl;
				TheirTouchDis = _VisionQueue[LinePoint[j]].BallPos.dist(_VisionQueue[LinePoint[j]].TheirPlayer[i]);
				TheirTouchNum = i;
			}
		}
		if (OurTouchNum != -1 && ((TheirTouchNum != -1 && OurTouchDis <= TheirTouchDis - 5) || TheirTouchNum == -1))
		{
			LastTouch = 1;
			LastTouchNumber = OurTouchNum + 1;
			//std::cout << "the ball touch OUR player No." << OurTouchNum << "WITH D:" << OurTouchDis << std::endl;
		}
		else if (TheirTouchNum != -1 && ((OurTouchNum != -1 && TheirTouchDis <= OurTouchDis - 5) || OurTouchNum == -1))
		{
			LastTouch = 2;
			LastTouchNumber = TheirTouchNum + 1;
			//std::cout << "the ball touch THEIR player No." << TheirTouchNum << "WITH D:" << TheirTouchDis << std::endl;
		}
		else if (OurTouchNum != -1 && TheirTouchNum != -1 && abs(OurTouchDis - TheirTouchDis) <= 5)
		{
			LastTouch = 0;
			LastTouchNumber = 0;
			//std::cout << "CANNOT decide the ball between OUR player no." << OurTouchNum << " and THEIR player No." << TheirTouchNum << std::endl;
		}
		else if (IsOnEdge(_VisionQueue[LinePoint[j]].BallPos))
		{
			LastTouch = 3;
			LastTouchNumber = 0;
			//std::cout << "the ball touch WALL" << std::endl;
		}
	}
}

bool CCollisionDetect::IsOnEdge(CGeoPoint _BallPos)
{
	if (_BallPos.x() < -440 || _BallPos.x() > 440
		|| _BallPos.y() < -290 || _BallPos.y() > 290)
		return true;
	return false;
}
