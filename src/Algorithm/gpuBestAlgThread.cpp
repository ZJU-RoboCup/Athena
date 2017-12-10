/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team：		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  gpuBestAlgThread.cpp										*/
/* Func:	  计算最佳跑位点的线程，支持CPU模式和GPU模式				*/
/* Author:	  王群 2012-08-18											*/
/*            叶杨笙 2014-03-01                                         */
/* Refer:	  ###														*/
/* E-mail:	  wangqun1234@zju.edu.cn									*/
/************************************************************************/

#define WIN32_LEAN_AND_MEAN

#include "gpuBestAlgThread.h"
#include "VisionModule.h"
#include "GDebugEngine.h"
#include "ShootRangeList.h"
#include "TimeCounter.h"
#include "tinyxml/ParamReader.h"
#include "param.h"
#include "utils.h"
#include "WorldModel.h"
#include "Network\UDPSocket.h"
#include <iostream>
#include <string>
#include <RobotCapability.h>
#include <CMmotion.h>
#include <fstream>
#include <vector>
#include "Semaphore.h"
#define OURPLAYER_NUM	6
#define THEIRPLAYER_NUM 6
#define BALL_NUM		1

#define UN_GPU_PRECISE 10

extern CMutex* _best_visiondata_copy_mutex;
extern CMutex* _value_getter_mutex;
extern Semaphore vision_finish_semaphore;
CGeoPoint C_ourPlayer_Geo[6];
CGeoPoint C_theirPlayer_Geo[6];
CGeoPoint C_ball_Geo;

//temp
bool UDPSendMessage();


namespace {
	CThreadCreator *_best_calculation_thread = 0;
	CUsecTimer timer;
	float(*C_PointPotential)[GRIDY] = NULL;  //临时用的potential内存
	const bool debug = false; //注意除非很熟悉这段代码，不能打开开关！！否则运算过大系统崩溃
	const double PI = 3.1415926;
	//动态角球
	const double determinDist1 = 60; //敌方车能影响我方射门的距离
	const double determinDist2 = 60; //敌方车能影响我方拉扯的距离
	const double determinDist3 = 25; //可接受敌方车到传球线路的最小距离
	const double determinDist4 = 25; //可接受敌方车到传球点的最小距离
	const double determinDist5 = 25; //可接受敌方车到我方射球车的最小距离
	const double weight1 = -5; //敌方车离传球点距离<determinDist1的数量
	const double weight2 = -5; //敌方车离我方接球车距离<determinDist2的数量
	const double weight3 = 0.8; //敌方车到传球线路距离的权值
	const double weight4 = 0.3;  //敌方车到传球点距离的权值
	const double weight5 = 0.4;  //敌方车到我方接球车距离的权值
	const double weight6 = 0;  //传球线路穿越敌方车数量的权值
	const double weight7 = 80; //射门角度的权值
	const double weight8 = -30; //接球角度的权值
	const double weight9 = -0.05; //传球距离的权值
	const double weight10 = -0.05; //接球距离的权值
	const double fixValueForFlatPass = -(weight1 + weight2 + weight6) * 6 - weight8 * Param::Math::PI/2 -(weight9 + weight10) * 750 + 500; //令determinValueForFlatKick不会少于0的补偿值
	//阀值
	const double thresholdForFlatMinDist = 50;
	const double thresholdForEnemy2MeFlatDist = 100;
	const double thresholdForEnemy2PointDist = 100;
	//动态后场球
	const double eCAPdeterminDist = 60;
	const double eCAMdeterminDist = 45;
	const double eCAPweight = -10;
	const double eCAMweight = -5;
	const double eCPDweight = 15;
	const double eCweight3 = 5;
	const double bCPDweight = -2;
	const double pTTGweight = -1.5;
	const double fixValueForChipPass = -(eCAPweight + eCAMweight + bCPDweight) * 6 - bCPDweight * 500;
	const double thresholdForEnemy2MeChipDist = 50;
	const double mulriple1 = 0.75;
	const double mulriple2 = 0.65;
	//DynamicKick
	const double dDeterminDist1 = 60; //敌方车能影响我方射门的距离
	const double dDeterminDist2 = 60; //敌方车能影响我方拉扯的距离
	const double dDeterminDist3 = 25; //可接受敌方车到传球线路的最小距离
	const double dDeterminDist4 = 25; //可接受敌方车到传球点的最小距离
	const double dDeterminDist5 = 25; //可接受敌方车到我方射球车的最小距离
	const double dWeight1 = -5; //敌方车离传球点距离<determinDist1的数量
	const double dWeight2 = -5; //敌方车离我方接球车距离<determinDist2的数量
	const double dWeight3 = 0.8; //敌方车到传球线路距离的权值
	const double dWeight4 = 0.3;  //敌方车到传球点距离的权值
	const double dWeight5 = 0.4;  //敌方车到我方接球车距离的权值
	const double dWeight6 = 0;  //传球线路穿越敌方车数量的权值
	const double dWeight7 = 60; //射门角度的权值
	const double dWeight8 = -60; //接球角度的权值
	const double dWeight9 = -0.2; //传球距离的权值
	const double dWeight10 = -0.05; //接球距离的权值
	const double dWeight11 = -0.3; //敌方到射门线的距离
	const double dFixValueForFlatPass = -(dWeight1 + dWeight2 + dWeight6) * 6 - dWeight8 * Param::Math::PI / 2 - (dWeight9 + dWeight10 + dWeight11) * 750 + 500;
	static double FRICTION = 43;
}

namespace {
	const double startX = 0;
	const double endX = 320;
	const double startY = -300;
	const double endY = 300;
	const double lengthstep = fabs(endX - startX) / 3;
	const double widthstep = fabs(endY - startY) / 3;
	const CGeoPoint wreckPos[] = {
		CGeoPoint(335,95),
		CGeoPoint(345,105),
		CGeoPoint(355,115),
		CGeoPoint(365,130),
		CGeoPoint(375,140),
		CGeoPoint(335,115),
		CGeoPoint(345,125),
		CGeoPoint(355,135),
		CGeoPoint(365,150),
		CGeoPoint(375,160)
	};

	const CGeoRectangle area[] = {
		CGeoRectangle(0,0,0,0),
		CGeoRectangle(3 * lengthstep, startY, 2 * lengthstep, startY + widthstep),
		CGeoRectangle(3 * lengthstep, startY + widthstep, 2 * lengthstep, startY + 2 * widthstep),
		CGeoRectangle(3 * lengthstep, startY + 2 * widthstep, 2 * lengthstep, startY + 3 * widthstep),
		CGeoRectangle(2 * lengthstep, startY, lengthstep, startY + widthstep),
		CGeoRectangle(2 * lengthstep, startY + widthstep, lengthstep, startY + 2 * widthstep),
		CGeoRectangle(2 * lengthstep, startY + 2 * widthstep, lengthstep, startY + 3 * widthstep),
		CGeoRectangle(lengthstep, startY, startX, startY + widthstep),
		CGeoRectangle(lengthstep, startY + widthstep, startX, startY + 2 * widthstep),
		CGeoRectangle(lengthstep, startY + 2 * widthstep, startX, startY + 3 * widthstep)
	};
}

CGPUBestAlgThread::CGPUBestAlgThread() {
	sendPoint = CGeoPoint(0, 0);
	_pVision = NULL;
	_PointPotential = new float[GRIDX][GRIDY];
	for (int i = 0; i < 10; i++) { //此处的10是说可以开10个区域，和maxPlayer无关
		pointKeeper[i] = CGeoPoint(9999, 9999);
		_lastCycle[i] = 0;
	}

	halfLength = Param::Field::PITCH_LENGTH / 2;
	std::cout << halfLength << endl;
	halfWidth = Param::Field::PITCH_WIDTH / 2;
	std::cout << halfWidth << endl;
	halfGoalWidth = Param::Field::GOAL_WIDTH / 2;
	std::cout << halfGoalWidth << endl;
	attempts = 0;
	_cycle1 = 0;
	_cycle2 = 0;
	detectDefendKickedOverFlag = false;
	jamMode = false;
	writeOver = false;
	determinCycle = 100;
	for (int i = 0; i <= 3; i++) {
		defendedTpye[i] = Free;
	}
	allMarkedFlag = false;
	maxAttempts = 3;
	useOwnPos = false;
	srand((unsigned)time(NULL));
	changeLeaderOverFlag = false;
	DECLARE_PARAM_READER_BEGIN(FieldParam)
	READ_PARAM(FRICTION)
	DECLARE_PARAM_READER_END
}

CGPUBestAlgThread::~CGPUBestAlgThread() {
	delete[]_PointPotential;
}

void CGPUBestAlgThread::initialize(CVisionModule* pVision) {
	_pVision = pVision;
	// 开启 GPU 计算的线程
	_best_calculation_thread = new CThreadCreator(doBestCalculation, 0);
	return;
}

void CGPUBestAlgThread::setSendPoint(const CGeoPoint passPoint) {
	sendPoint = passPoint;
}

void CGPUBestAlgThread::generatePointValue() {
	// 不适用GPU来计算跑位点
	/************************************************************************/
	/* 数据算法数据传入：车球位置信息                                       */
	/************************************************************************/
	// 上锁
	_best_visiondata_copy_mutex->lock();
	// 拷贝
	for (int i = 0; i < OURPLAYER_NUM; i++) {
		if (_pVision->OurPlayer(i + 1).Valid()) {
			C_ourPlayer_Geo[i].setX((float)_pVision->OurPlayer(i + 1).Pos().x());
			C_ourPlayer_Geo[i].setY((float)_pVision->OurPlayer(i + 1).Pos().y());
		}
		else {
			C_ourPlayer_Geo[i].setX(9999);
			C_ourPlayer_Geo[i].setY(9999);
		}

	}
	for (int i = 0; i < THEIRPLAYER_NUM; i++) {
		if (_pVision->TheirPlayer(i + 1).Valid()) {
			C_theirPlayer_Geo[i].setX((float)_pVision->TheirPlayer(i + 1).Pos().x());
			C_theirPlayer_Geo[i].setY((float)_pVision->TheirPlayer(i + 1).Pos().y());
		}
		else {
			C_theirPlayer_Geo[i].setX(9999);
			C_theirPlayer_Geo[i].setY(9999);
		}

	}
	C_ball_Geo.setX((float)sendPoint.x());
	C_ball_Geo.setY((float)sendPoint.y());
	// 解锁
	_best_visiondata_copy_mutex->unlock();
	// 不支持CUDA时的CPU计算
	if (NULL == C_PointPotential) {
		C_PointPotential = new float[GRIDX][GRIDY];
	}
	//计算,注意这里的场地数据！！！！！！！和真实场地不一样，场地变化是需要手动修改！！！！wang qun
	//注意，场地参数必须取为 UN_GPU_PRECISE 的整数倍，否则下面有的循环会跳过！！！ yys
	for (int x = -halfLength; x <= halfLength; x += UN_GPU_PRECISE) {
		for (int y = -halfWidth; y <= halfWidth; y += UN_GPU_PRECISE) {
			C_PointPotential[x + halfLength][y + halfWidth] = getPointPotential(x, y);
		}
	}
	//势能值拷贝至多线程公用区域
	_value_getter_mutex->lock();
	for (int x = -halfLength; x <= halfLength; x += UN_GPU_PRECISE) {
		for (int y = -halfWidth; y <= halfWidth; y += UN_GPU_PRECISE) {
			_PointPotential[x + halfLength][y + halfWidth] = C_PointPotential[x + halfLength][y + halfWidth];
		}
	}
	_value_getter_mutex->unlock();
}

CGeoPoint CGPUBestAlgThread::getBestPoint(const CGeoPoint leftUp, const CGeoPoint rightDown, const int pointKeeperNum, const ReCalModeT& reCalMode, bool forceReset) {
	//防止输入错误
	if (leftUp.x() <= rightDown.x() || leftUp.y() >= rightDown.y()) {
		cout << "ErrorPoint In GPUBestPointCalc!!!!!   " << leftUp << "  " << rightDown << endl;
		return CGeoPoint(0, 0);
	}
	if (Utils::OutOfField(leftUp, 1) || Utils::OutOfField(rightDown, 1)) {
		cout << "ErrorPoint In GPUBestPointCalc!!!!!   " << leftUp << "  " << rightDown << endl;
		return CGeoPoint(0, 0);
	}
	CGeoPoint bestPoint = leftUp;
	//长时间不调用函数，则历史记录点归零
	if (_pVision->Cycle() - _lastCycle[pointKeeperNum] > Param::Vision::FRAME_RATE * 0.1) pointKeeper[pointKeeperNum] = CGeoPoint(9999, 9999);
	//是否采用上次运算结果的判断
	if (_pVision->Cycle() == _lastCycle[pointKeeperNum]
		&& pointKeeper[pointKeeperNum].x() < leftUp.x() && pointKeeper[pointKeeperNum].x() > rightDown.x()
		&& pointKeeper[pointKeeperNum].y() < rightDown.y() && pointKeeper[pointKeeperNum].y() > leftUp.y())
	{
		return pointKeeper[pointKeeperNum];
	}
	if (false == forceReset && pointKeeper[pointKeeperNum].dist(CGeoPoint(9999, 9999)) > 10 && true == isLastOneValid(pointKeeper[pointKeeperNum])
		&& pointKeeper[pointKeeperNum].x() < leftUp.x() && pointKeeper[pointKeeperNum].x() > rightDown.x()
		&& pointKeeper[pointKeeperNum].y() < rightDown.y() && pointKeeper[pointKeeperNum].y() > leftUp.y())
	{
		_lastCycle[pointKeeperNum] = _pVision->Cycle();
		return pointKeeper[pointKeeperNum];
	}
	else {
		float minValue = 1000;
		_value_getter_mutex->lock();
		for (int x = (int)rightDown.x() + 1; x < (int)leftUp.x() - 1; x++) {
			if (0 != x % UN_GPU_PRECISE) {  // 若场地参数不为 UN_GPU_PRECISE 的倍数，则会continue！！！  yys
				continue;
			}
			for (int y = (int)leftUp.y() + 1; y < (int)rightDown.y() - 1; y++) {
				if (0 != y % UN_GPU_PRECISE) {
					continue;
				}
				if (_PointPotential[x + halfLength][y + halfWidth] < minValue) {
					minValue = _PointPotential[x + halfLength][y + halfWidth];
					bestPoint = CGeoPoint(x, y);
				}
			}
		}
		_value_getter_mutex->unlock();
	}
	pointKeeper[pointKeeperNum] = bestPoint;
	_lastCycle[pointKeeperNum] = _pVision->Cycle();
	//std::cout << bestPoint << std::endl;
	return bestPoint;
}

bool CGPUBestAlgThread::isLastOneValid(const CGeoPoint& p) {
	CShootRangeList shootRangeList(_pVision, 1, p);
	const CValueRangeList& shootRange = shootRangeList.getShootRange();
	if (shootRange.size() > 0) {
		const CValueRange* bestRange = shootRange.getMaxRangeWidth();
		if (bestRange->getWidth() > Param::Field::BALL_SIZE*2.0) {
			return true;
		}
	}
	return false;
}

double CGPUBestAlgThread::getPosPotential(const CGeoPoint p) {
	return getPointPotential(p.x(), p.y());
}

CThreadCreator::CallBackReturnType THREAD_CALLBACK CGPUBestAlgThread::doBestCalculation(CThreadCreator::CallBackParamType lpParam) {
	while (true) {
		vision_finish_semaphore.Wait();
		GPUBestAlgThread::Instance()->generatePointValue();
	}
}

/************************************************************************/
/* 评价函数：接球评价                                                   */
/************************************************************************/
void CGPUBestAlgThread::C_evaluateReceive_Geo(const CGeoPoint p, float& reverseReceiveP) {
	float dist_ball2p = p.dist(C_ball_Geo);
	float dir_ball2p = (p - C_ball_Geo).dir();
	float minAngleDiff = 1000;
	//1
	float dist_opp2p = (p - C_theirPlayer_Geo[0]).mod();
	CVector ball2opp = C_theirPlayer_Geo[0] - C_ball_Geo;
	float dist_ball2opp = ball2opp.mod();
	float dir_ball2opp = ball2opp.dir();
	float absDirDiff = fabs(Utils::Normalize(dir_ball2opp - dir_ball2p));
	if (absDirDiff < minAngleDiff && dist_ball2opp < dist_ball2p) {
		minAngleDiff = absDirDiff;// 可以传的角度空当距离（重要评价参数）
	}
	reverseReceiveP += 50.0 / (dist_opp2p + 0.1);
	//2
	dist_opp2p = (p - C_theirPlayer_Geo[1]).mod();
	ball2opp = C_theirPlayer_Geo[1] - C_ball_Geo;
	dist_ball2opp = ball2opp.mod();
	dir_ball2opp = ball2opp.dir();
	absDirDiff = fabs(Utils::Normalize(dir_ball2opp - dir_ball2p));
	if (absDirDiff < minAngleDiff && dist_ball2opp < dist_ball2p) {
		minAngleDiff = absDirDiff;
	}
	reverseReceiveP += 50.0 / (dist_opp2p + 0.1);
	//3
	dist_opp2p = (p - C_theirPlayer_Geo[2]).mod();
	ball2opp = C_theirPlayer_Geo[2] - C_ball_Geo;
	dist_ball2opp = ball2opp.mod();
	dir_ball2opp = ball2opp.dir();
	absDirDiff = fabs(Utils::Normalize(dir_ball2opp - dir_ball2p));
	if (absDirDiff < minAngleDiff && dist_ball2opp < dist_ball2p) {
		minAngleDiff = absDirDiff;
	}
	reverseReceiveP += 50.0 / (dist_opp2p + 0.1);
	//4
	dist_opp2p = (p - C_theirPlayer_Geo[3]).mod();
	ball2opp = C_theirPlayer_Geo[3] - C_ball_Geo;
	dist_ball2opp = ball2opp.mod();
	dir_ball2opp = ball2opp.dir();
	absDirDiff = fabs(Utils::Normalize(dir_ball2opp - dir_ball2p));
	if (absDirDiff < minAngleDiff && dist_ball2opp < dist_ball2p) {
		minAngleDiff = absDirDiff;
	}
	reverseReceiveP += 50.0 / (dist_opp2p + 0.1);
	//5
	dist_opp2p = (p - C_theirPlayer_Geo[4]).mod();
	ball2opp = C_theirPlayer_Geo[4] - C_ball_Geo;
	dist_ball2opp = ball2opp.mod();
	dir_ball2opp = ball2opp.dir();
	absDirDiff = fabs(Utils::Normalize(dir_ball2opp - dir_ball2p));
	if (absDirDiff < minAngleDiff && dist_ball2opp < dist_ball2p) {
		minAngleDiff = absDirDiff;
	}
	reverseReceiveP += 50.0 / (dist_opp2p + 0.1);
	//6
	dist_opp2p = (p - C_theirPlayer_Geo[5]).mod();
	ball2opp = C_theirPlayer_Geo[5] - C_ball_Geo;
	dist_ball2opp = ball2opp.mod();
	dir_ball2opp = ball2opp.dir();
	absDirDiff = fabs(Utils::Normalize(dir_ball2opp - dir_ball2p));
	if (absDirDiff < minAngleDiff && dist_ball2opp < dist_ball2p) {
		minAngleDiff = absDirDiff;
	}
	reverseReceiveP += 50.0 / (dist_opp2p + 0.1);
	//不要阻挡自己人射门
	if (debug) {
		std::cout << "receive is " << reverseReceiveP << std::endl;
	}
	CGeoLine temp_line(C_ball_Geo, CGeoPoint(halfLength, 0));
	float blockMyself = p.dist(temp_line.projection(p));
	if (blockMyself <= 50) {
		reverseReceiveP += C_POINT_PASS_OFF;
	}
	else reverseReceiveP += 20.0 / (blockMyself + 0.1);
	if (debug) {
		std::cout << "block self angle is " << 20.0 / (blockMyself + 0.1) << std::endl;
	}
	reverseReceiveP += 3.0 / (minAngleDiff + 0.01); // 传球的阻挡角,后面0.001为保证除数不为0
	if (debug) {
		std::cout << "block angle is " << 3.0 / (minAngleDiff + 0.01) << std::endl;
	}
}
/************************************************************************/
/*射门角度计算                                                          */
/************************************************************************/
inline float C_GetDirRange(const C_blockDirection& D) { return (D.leftDist <= D.rightDist ? D.leftDist : D.rightDist)*fabs(D.rightDir - D.leftDir); } //此处暂时加上fabs()，若程序没问题就去掉
inline float C_GetDirMiddle(const C_blockDirection& D) { return (D.leftDir + D.rightDir) / 2.0; }
//射门角度计算函数
C_shootParam CGPUBestAlgThread::C_shootRange_Geo(const CGeoPoint p) {
	C_shootParam sParam;
	CGeoPoint blockOpp[6];
	int blockOppNum = 0; //和C_theirPlayer的关系是i-1
						 //p点向左右门柱相关参数
	const CGeoPoint goalPoint(halfLength, 0);
	const CGeoPoint goalLeftPoint(halfLength, -halfGoalWidth);
	const CGeoPoint goalRightPoint(halfLength, halfGoalWidth);
	const CGeoLine goalLeftLine(p, goalLeftPoint);
	const CGeoLine goalRightLine(p, goalRightPoint);
	const float leftAngle = (goalLeftPoint - p).dir();
	const float rightAngle = (goalRightPoint - p).dir();
	//将处于阻挡范围的小车位置写入数组blockOpp[]
	//1
	CGeoPoint opp = C_theirPlayer_Geo[0];
	if (p.x() - 10 < opp.x() && Utils::dirDiff((opp - p), (goalPoint - p)) < PI * 2 / 3.0 && Utils::dirDiff((p - goalPoint), (opp - goalPoint)) < PI * 2 / 3.0) {
		blockOpp[blockOppNum] = opp;
		blockOppNum++;
	}
	//2
	opp = C_theirPlayer_Geo[1];
	if (p.x() - 10 < opp.x() && Utils::dirDiff((opp - p), (goalPoint - p)) < PI * 2 / 3.0 && Utils::dirDiff((p - goalPoint), (opp - goalPoint)) < PI * 2 / 3.0) {
		blockOpp[blockOppNum] = opp;
		blockOppNum++;
	}
	//3
	opp = C_theirPlayer_Geo[2];
	if (p.x() - 10 < opp.x() && Utils::dirDiff((opp - p), (goalPoint - p)) < PI * 2 / 3.0 && Utils::dirDiff((p - goalPoint), (opp - goalPoint)) < PI * 2 / 3.0) {
		blockOpp[blockOppNum] = opp;
		blockOppNum++;
	}
	//4
	opp = C_theirPlayer_Geo[3];
	if (p.x() - 10 < opp.x() && Utils::dirDiff((opp - p), (goalPoint - p)) < PI * 2 / 3.0 && Utils::dirDiff((p - goalPoint), (opp - goalPoint)) < PI * 2 / 3.0) {
		blockOpp[blockOppNum] = opp;
		blockOppNum++;
	}
	//5
	opp = C_theirPlayer_Geo[4];
	if (p.x() - 10 < opp.x() && Utils::dirDiff((opp - p), (goalPoint - p)) < PI * 2 / 3.0 && Utils::dirDiff((p - goalPoint), (opp - goalPoint)) < PI * 2 / 3.0) {
		blockOpp[blockOppNum] = opp;
		blockOppNum++;
	}
	//6
	opp = C_theirPlayer_Geo[5];
	if (p.x() - 10 < opp.x() && Utils::dirDiff((opp - p), (goalPoint - p)) < PI * 2 / 3.0 && Utils::dirDiff((p - goalPoint), (opp - goalPoint)) < PI * 2 / 3.0) {
		blockOpp[blockOppNum] = opp;
		blockOppNum++;
	}
	//有效边缘方向数组
	float activeEdge[14];
	float activeDist[14];
	memset(activeEdge, 0, 14 * sizeof(float));
	memset(activeDist, 0, 14 * sizeof(float));
	//for(int i=0;i<14;i++){
	//	activeedge[i]=0;
	//	activedist[i]=0;
	//}


	int activeEdgeCount = 0;
	//将有效边缘线加入到有效边缘方向数组
	//特例：左边线
	int leftCount = blockOppNum;
	for (; leftCount > 0; leftCount--) { //注意i对应blockOpp[i-1]
		if (Utils::pointToLineDist(blockOpp[leftCount - 1], goalLeftLine) < 9) {
			break;
		}
	}
	if (0 == leftCount) {
		activeEdge[activeEdgeCount] = leftAngle;
		activeDist[activeEdgeCount] = p.dist(goalLeftPoint);
		activeEdgeCount++;
	}
	//特例：右边线
	int rightCount = blockOppNum;
	for (; rightCount > 0; rightCount--) {
		if (Utils::pointToLineDist(blockOpp[rightCount - 1], goalRightLine) < 9) {
			break;
		}
	}
	if (0 == rightCount) {
		activeEdge[activeEdgeCount] = rightAngle;
		activeDist[activeEdgeCount] = p.dist(goalRightPoint);
		activeEdgeCount++;
	}
	int useBlockOppNum = blockOppNum;
	for (; useBlockOppNum > 0; useBlockOppNum--) { //第useBlockOppNum左右两边缘的判断
		CVector p2opp = blockOpp[useBlockOppNum - 1] - p;
		float maxSide = p2opp.mod();
		float derta = asinf(9.0 / maxSide);
		//左边缘
		float oppLeftBlockAngle = p2opp.dir() - derta;
		CGeoLine oppLeftBlockLine(p, oppLeftBlockAngle);
		int oppLeftCount = blockOppNum;
		for (; oppLeftCount > 0; oppLeftCount--) {
			if (oppLeftCount == useBlockOppNum) {
				continue;
			}
			if (Utils::pointToLineDist(blockOpp[oppLeftCount - 1], oppLeftBlockLine) < 9) {
				break;
			}
		}
		if (0 == oppLeftCount && true == Utils::CBetween(oppLeftBlockAngle, leftAngle, rightAngle)) {
			activeEdge[activeEdgeCount] = oppLeftBlockAngle;
			activeDist[activeEdgeCount] = p2opp.mod();
			activeEdgeCount++;
		}
		//右边缘
		float oppRightBlockAngle = p2opp.dir() + derta;
		CGeoLine oppRightBlockLine(p, oppRightBlockAngle);
		int oppRightCount = blockOppNum;
		for (; oppRightCount > 0; oppRightCount--) {
			if (oppRightCount == useBlockOppNum) {
				continue;
			}
			if (Utils::pointToLineDist(blockOpp[oppRightCount - 1], oppRightBlockLine) < 9) {
				break;
			}
		}
		if (0 == oppRightCount && true == Utils::CBetween(oppRightBlockAngle, leftAngle, rightAngle)) {
			activeEdge[activeEdgeCount] = oppRightBlockAngle;
			activeDist[activeEdgeCount] = p2opp.mod();
			activeEdgeCount++;
		}
	}
	//如果没有射门角度,直接返回
	if (0 == activeEdgeCount) {
		sParam.biggestAngle = 0;
		sParam.bestDir = 0;
		return sParam;
	}
	//有效边缘方向冒泡排序
	float temp;
	for (int i = activeEdgeCount - 1; i > 0; i--) {
		for (int j = 0; j < i; j++) {
			if (activeEdge[j] > activeEdge[j + 1]) {
				temp = activeEdge[j];
				activeEdge[j] = activeEdge[j + 1];
				activeEdge[j + 1] = temp;
				temp = activeDist[j];
				activeDist[j] = activeDist[j + 1];
				activeDist[j + 1] = temp;
			}
		}
	}
	//放入阻挡方向结构体
	C_blockDirection blockDir[7];
	memset(blockDir, 0, sizeof(C_blockDirection) * 7);
	int blockDirCount = activeEdgeCount / 2;
	for (int i = 0; i < blockDirCount; i++) {
		blockDir[i].leftDir = activeEdge[i * 2];
		blockDir[i].leftDist = activeDist[i * 2];
		blockDir[i].rightDir = activeEdge[i * 2 + 1];
		blockDir[i].rightDist = activeDist[i * 2 + 1];
	}
	//冒泡排序射门角度,将最大的排到最前
	C_blockDirection temp2;
	for (int i = blockDirCount - 1; i > 0; i--) {
		if (C_GetDirRange(blockDir[i]) > C_GetDirRange(blockDir[i - 1])) {
			temp2 = blockDir[i];
			blockDir[i] = blockDir[i - 1];
			blockDir[i - 1] = temp2;
		}
	}
	//取最大的射门角度及射门线
	sParam.biggestAngle = C_GetDirRange(blockDir[0]);
	sParam.bestDir = C_GetDirMiddle(blockDir[0]);
	if (debug) {
		std::cout << "the biggestAngle is" << sParam.biggestAngle << std::endl;
		CVector m(1000, blockDir[0].leftDir);
		GDebugEngine::Instance()->gui_debug_line(CGeoPoint(p.x(), p.y()), CGeoPoint(p.x(), p.y()) + CVector(m.x(), m.y()), COLOR_RED);
		m = CVector(1000, blockDir[0].rightDir);
		GDebugEngine::Instance()->gui_debug_line(CGeoPoint(p.x(), p.y()), CGeoPoint(p.x(), p.y()) + CVector(m.x(), m.y()), COLOR_RED);
	}
	return sParam;
}
/************************************************************************/
/* 评价函数：射门部分                                                   */
/************************************************************************/
void CGPUBestAlgThread::C_evaluateShoot_Geo(const CGeoPoint p, float& reverseShootP) {
	C_shootParam paramP = C_shootRange_Geo(p);
	//射门空挡
	reverseShootP += 200.0 / (paramP.biggestAngle + 0.1);// 系数0.1待调节,后面0.001为保证除数不为0
	if (debug) {
		std::cout << "kong dang is " << reverseShootP << std::endl;
	}
	//射门队员接球到射门的调整角度
	float shootDirDiff = fabs(Utils::Normalize(paramP.bestDir - (C_ball_Geo - p).dir()));
	reverseShootP += 1.5*shootDirDiff*shootDirDiff;
	if (debug) {
		std::cout << "tiaozhengjiao is " << 1.5 * shootDirDiff * shootDirDiff << std::endl << std::endl << std::endl;
	}
}

/************************************************************************/
/* 评价函数：其他部分                                                   */
/************************************************************************/
void CGPUBestAlgThread::C_evaluateOthers_Geo(const CGeoPoint p, float& reverseOtherP) {
	if (p.dist(C_ball_Geo) < 150) { // 距离开球点一定距离之外
		reverseOtherP += C_POINT_PASS_OFF;
	}
}
/************************************************************************/
/************************************************************************/
float CGPUBestAlgThread::getPointPotential(int x, int y) {
	//要评价的点
	CGeoPoint evaluatePoint_Geo(x, y);
	//初始化各类评价值	
	float reverseReceiveP_Geo = 0;
	float reverseShootP_Geo = 0;
	float reverseOtherP_Geo = 0;
	float reversePotential_Geo = 0;
	//评价函数
	C_evaluateReceive_Geo(evaluatePoint_Geo, reverseReceiveP_Geo);
	C_evaluateShoot_Geo(evaluatePoint_Geo, reverseShootP_Geo);
	C_evaluateOthers_Geo(evaluatePoint_Geo, reverseOtherP_Geo);
	reversePotential_Geo = reverseReceiveP_Geo + reverseShootP_Geo + reverseOtherP_Geo;
	return reversePotential_Geo;
}
/************************************************************************/
//计算定位球最佳射门点
/************************************************************************/
namespace {
	const int PointNum = 31;
	const int PointNum2 = 15;

}
CGeoPoint CGPUBestAlgThread::bestPointForCornerKicks(bool isChip) {
	int i, j, k;
	int numRecord;
	int getBallNum;
	int enemyNumRecord;
	int enemyAroundPointNum;
	int enemyAroundMeNum;;
	double dist;
	double dist2;
	double enemy2PointDist;
	double me2PointDist;
	double ball2PointDist;
	double ball2RealPointDist;
	double enemy2MeDist;
	double minDist;
	double crossNum;
	double currentValue;
	double minShootDir = 5 * Param::Math::PI / 180;
	double _raw_kick_dir;
	double determindir;
	int size;
	double sizeOfDir;
	bool Test = false;//测试模式
	bool isPassLineBlock;
	const CGeoPoint theirGoal(Param::Field::PITCH_LENGTH / 2, 0);
	CGeoPoint flatPointTmp[PointNum] = { CGeoPoint(285,0),CGeoPoint(295,-50),CGeoPoint(295,-70),CGeoPoint(295,-90),CGeoPoint(265,-50),CGeoPoint(265,-70),CGeoPoint(265,-90),CGeoPoint(235,-50),CGeoPoint(235,-70),CGeoPoint(235,-90),CGeoPoint(250,-25),CGeoPoint(0,-275),CGeoPoint(380,150),CGeoPoint(150,-70),CGeoPoint(370,130),CGeoPoint(240,140),CGeoPoint(100,-275),CGeoPoint(185,-275),CGeoPoint(220,70),CGeoPoint(225,-40),CGeoPoint(320,240),CGeoPoint(200,-170),CGeoPoint(275,-120),CGeoPoint(90,-130),CGeoPoint(270,70),CGeoPoint(110,0),CGeoPoint(340,100),CGeoPoint(245,25),CGeoPoint(190,0),CGeoPoint(345,105),CGeoPoint(225,-115) };
	vector <CGeoPoint> flatPoint(&flatPointTmp[0], &flatPointTmp[PointNum]);
	//CGeoPoint chipPointTmp[PointNum2] = { CGeoPoint(333,0),CGeoPoint(335,10),CGeoPoint(335,20),CGeoPoint(335,30),CGeoPoint(335,30),CGeoPoint(340,57),CGeoPoint(350,75),CGeoPoint(355,85),CGeoPoint(360,93),CGeoPoint(365,95),CGeoPoint(370,105),CGeoPoint(375,107),CGeoPoint(380,110),CGeoPoint(385,115),CGeoPoint(390,120) };
	//vector <CGeoPoint> chipPoint(&chipPointTmp[0], &chipPointTmp[PointNum2]);
	vector <CGeoPoint> chipPoint = flatPoint;
	if (!isChip) {
		size = flatPoint.size();
		determinValueForFlatKick = -999;
		for (i = 0; i < size; i++)
		{
			enemy2PointDist = 9999;
			me2PointDist = 9999;
			enemy2MeDist = 9999;
			minDist = 9999;
			crossNum = 0;
			enemyAroundPointNum = 0;
			enemyAroundMeNum = 0;
			isPassLineBlock = false;
			if (_pVision->Ball().Y() > 0) flatPoint[i].setY(-1 * flatPoint[i].y());
			CGeoSegment seg = CGeoSegment(_pVision->Ball().Pos(), flatPoint[i]);
			for (j = 1; j <= 6; j++) {
				/*if (seg.IsPointOnLineOnSegment(seg.projection(_pVision->OurPlayer(j).Pos()))) {
					dist = _pVision->OurPlayer(j).Pos().dist(seg.projection(_pVision->OurPlayer(j).Pos()));
					if (dist < 12 && flatPoint[i].x()<230) {
						isPassLineBlock = true;
					}`
				}*/
				dist = _pVision->OurPlayer(j).Pos().dist(flatPoint[i]);
				if (dist < me2PointDist && j!=number[4]) {
					me2PointDist = dist;
					getBallNum = j;
				}
			}
			CGeoSegment seg2 = CGeoSegment(_pVision->OurPlayer(getBallNum).Pos(), flatPoint[i]);
			for (k = 1; k <= 6; k++) {
				dist = _pVision->TheirPlayer(k).Pos().dist(flatPoint[i]);
				if (dist < determinDist1) enemyAroundPointNum++;
				if (dist < enemy2PointDist)  enemy2PointDist = dist;
				dist = _pVision->OurPlayer(getBallNum).Pos().dist(_pVision->TheirPlayer(k).Pos());
				if (dist < determinDist2) enemyAroundMeNum++;
				if (dist < enemy2MeDist) {
					enemy2MeDist = dist;
					enemyNumRecord = k;
				}
				dist = _pVision->TheirPlayer(k).Pos().dist(seg.projection(_pVision->TheirPlayer(k).Pos()));
				dist2 = _pVision->TheirPlayer(k).Pos().dist(seg2.projection(_pVision->TheirPlayer(k).Pos()));
				if (seg.IsPointOnLineOnSegment(seg.projection(_pVision->TheirPlayer(k).Pos())) || seg2.IsPointOnLineOnSegment(seg2.projection(_pVision->TheirPlayer(k).Pos()))) {
					if (dist < 12 || dist2 < 15) {
						isPassLineBlock = true;
						break;
					}
					else {
						if (dist < minDist) minDist = dist;
						crossNum++;
					}
				}
			}
			CShootRangeList shootRangeList(_pVision, getBallNum, flatPoint[i]);
			const CValueRange* bestRange = NULL;
			const CValueRangeList& shootRange = shootRangeList.getShootRange();
			if (shootRange.size() > 0) {
				bestRange = shootRange.getMaxRangeWidth();
				sizeOfDir = bestRange->getSize();
				if (bestRange && bestRange->getWidth() > 0) {	// 要求射门空档足够大
					_raw_kick_dir = bestRange->getMiddle();
				}
				double metoballdir = (_pVision->Ball().Pos() - flatPoint[i]).dir();
				determindir = fabs(Utils::Normalize(metoballdir - _raw_kick_dir));
			}
			else {
				sizeOfDir = 0;
			}
			ball2PointDist = _pVision->Ball().Pos().dist(flatPoint[i]);
			minDist = minDist > thresholdForFlatMinDist ? thresholdForFlatMinDist : minDist;
			enemy2MeDist = enemy2MeDist > thresholdForEnemy2MeFlatDist ? thresholdForEnemy2MeFlatDist : enemy2MeDist;
			enemy2PointDist = enemy2PointDist > thresholdForEnemy2PointDist ? thresholdForEnemy2PointDist : enemy2PointDist;
			if (isPassLineBlock || minDist < determinDist3 || enemy2PointDist < determinDist4 || ((enemy2MeDist < determinDist5) && (_pVision->TheirPlayer(enemyNumRecord).Pos().x() > _pVision->OurPlayer(getBallNum).Pos().x())) || sizeOfDir < minShootDir || ball2PointDist < 150 || determindir>85 * Param::Math::PI / 180) {
				currentValue = 0;
				if (Test) {
					if (isPassLineBlock) cout << i << " " << flatPoint[i].x() << " " << flatPoint[i].y() << " " << "block" << endl;
					if (minDist < determinDist3) cout << i << " " << flatPoint[i].x() << " " << flatPoint[i].y() << " " << "minDist < determinDist3" << endl;
					if (enemy2PointDist < determinDist4) cout << i << " " << flatPoint[i].x() << " " << flatPoint[i].y() << " " << "enemy2PointDist < determinDist4" << endl;
					if (((enemy2MeDist < determinDist5) && (_pVision->TheirPlayer(enemyNumRecord).Pos().x() > _pVision->OurPlayer(getBallNum).Pos().x()))) cout << i << " " << flatPoint[i].x() << " " << flatPoint[i].y() << " " << "enemy2MeDist < determinDist5" << endl;
					if (sizeOfDir < minShootDir) cout << i << " " << flatPoint[i].x() << " " << flatPoint[i].y() << " " << "sizeOfDir < minShootDir" << endl;
				}
			}
			else {
				currentValue = enemyAroundPointNum * weight1 + enemyAroundMeNum * 0 + minDist * weight3 + enemy2PointDist * weight4 + enemy2MeDist * 0 + crossNum * weight6 + sizeOfDir * weight7 + determindir * weight8 + ball2PointDist * weight9 + me2PointDist * weight10 + fixValueForFlatPass;
				if (Test) cout << i << " " << flatPoint[i].x() << " " << flatPoint[i].y() << " " << "ok" << endl;
			}
			if (currentValue > determinValueForFlatKick) {
				determinValueForFlatKick = currentValue;
				numRecord = i;
				//cout << determindir*180/3.1415926 << endl;
			}
			//权值输出
			out.open("weight.txt", std::ios::app);
			out << i << " " << getBallNum << "   " <<currentValue << " = " << enemyAroundPointNum * weight1 << " + " << enemyAroundMeNum * weight2 << " + " << minDist * weight3 << " + ";
			out << enemy2PointDist * weight4 << " + " << enemy2MeDist * weight5 << " + " << crossNum * weight6 << " + " << sizeOfDir * weight7 << " + ";
			out << determindir * weight8 << " + " << ball2PointDist * weight9 << " + " << me2PointDist * weight10 << " " << fixValueForFlatPass << endl;
			out << endl;
			out.close();
		}
	}
	else {
		size = chipPoint.size();
		determinValueForFlatKick = -999;
		determinValueForChipKick = -999;
		for (i = 0; i < size; i++) {
			enemy2PointDist = 999;
			me2PointDist = 999;
			enemy2MeDist = 999;
			enemyAroundPointNum = 0;
			enemyAroundMeNum = 0;
			if (_pVision->Ball().Y() > 0) chipPoint[i].setY(-1 * chipPoint[i].y());
			ball2PointDist = _pVision->Ball().Pos().dist(chipPoint[i]);
			for (j = 1; j <= 6; j++) {
				dist = _pVision->OurPlayer(j).Pos().dist(chipPoint[i]);
				if (dist < me2PointDist) {
					me2PointDist = dist;
					getBallNum = j;
				}
			}
			for (k = 1; k <= 6; k++) {
				if (_pVision->TheirPlayer(k).Valid()) {
					dist = _pVision->TheirPlayer(k).Pos().dist(chipPoint[i]);
					if (dist < determinDist1) enemyAroundPointNum++;
					if (dist < enemy2PointDist)  enemy2PointDist = dist;
					dist = _pVision->OurPlayer(getBallNum).Pos().dist(_pVision->TheirPlayer(k).Pos());
					if (dist < determinDist2) enemyAroundMeNum++;
					if (dist < enemy2MeDist) {
						enemy2MeDist = dist;
						enemyNumRecord = k;
					}
				}
			}

			CShootRangeList shootRangeList2(_pVision, getBallNum, chipPoint[i]);
			const CValueRange* bestRange2 = NULL;
			const CValueRangeList& shootRange2 = shootRangeList2.getShootRange();
			if (shootRange2.size() > 0) {
				bestRange2 = shootRange2.getMaxRangeWidth();
				sizeOfDir = bestRange2->getSize();
				if (bestRange2 && bestRange2->getWidth() > 0) {	// 要求射门空档足够大
					_raw_kick_dir = bestRange2->getMiddle();
				}
				double metoballdir = (_pVision->Ball().Pos() - chipPoint[i]).dir();
				determindir = fabs(Utils::Normalize(metoballdir - _raw_kick_dir));
			}
			else {
				sizeOfDir = 0;
			}
			enemy2PointDist = enemy2PointDist > thresholdForEnemy2PointDist ? thresholdForEnemy2PointDist : enemy2PointDist;
			if (enemy2PointDist < determinDist4 || sizeOfDir < minShootDir || ball2PointDist < 150 || determindir>85 * Param::Math::PI / 180)
				currentValue = 0;
			else
				currentValue = enemyAroundPointNum * weight1 + enemy2PointDist*2 + crossNum * weight6 + sizeOfDir * 400 + determindir*(-200) + ball2PointDist*(-0.1) + 5000;
			//cout << currentValue << " " << enemyAroundPointNum << " " << enemy2PointDist << " "<< sizeOfDir<<" "<< determindir<<" "<< ball2PointDist<<endl;
			if (currentValue > determinValueForChipKick) {
				determinValueForChipKick = currentValue;
				numRecord = i;
				ball2RealPointDist = _pVision->Ball().Pos().dist(chipPoint[numRecord]);
			}
		}
	}
	//cout << numRecord << endl;
	//cout << determindir << endl;
	//cout <<  determinvalue << endl;
	//发送udp



	//
	if (determinValueForFlatKick > 0) {
		out.open("weight.txt", std::ios::app);
		out << numRecord <<endl;
		out << endl;
		out.close();
		return flatPoint[numRecord];
	}
	else if (determinValueForChipKick > 0) {
		chipPower = ball2RealPointDist * 0.60;
		fixTime = -0.0000015498 * chipPower*chipPower + 0.0025344180 * chipPower + 0.2463515283;
		fixBuf = fixTime * 60;
		cout << chipPower << endl;
		return chipPoint[numRecord];
	}
	else {
		chipPower = 250;
		if (_pVision->Ball().Pos().y() > 0)
			return CGeoPoint(345, -75);
		else
			return CGeoPoint(345, 75);
	}
}

CGeoPoint CGPUBestAlgThread::bestPointForFrontKicks() {
	/*CGeoPoint targetPoint;
	vector <CGeoPoint> point;
	const MobileVisionT& ball = _pVision->Ball();
	const int AntiY = ball.Pos().y() > 0 ? -1 : 1;
	const double tmpY = 125;
	CGeoPoint p(450, tmpY*AntiY);
	const double min_Pos_X = 100;
	const double max_Pos_Y = 300;
	const double increaseStepForR = 15;
	const double increaseStepForDir = 10 * Param::Math::PI / 180;
	const double minDistance = 100;
	const double maxDistance = ball.Pos().dist(p);
	const double dir = (p - ball.Pos()).dir();
	const double limit = 200;
	double i, j;
	for (i = minDistance; i < maxDistance; i += increaseStepForR) {
	for (j = dir; (targetPoint.x() > min_Pos_X && fabs(targetPoint.y()) <= max_Pos_Y || j == dir); j += increaseStepForDir * AntiY) {
	targetPoint = ball.Pos() + Utils::Polar2Vector(i, j);
	if (!Utils::InTheirPenaltyArea(targetPoint,0) && fabs(targetPoint.y()) <= limit) point.push_back(targetPoint);
	}
	}
	//cout << point.size() << endl;*/
	return CGeoPoint(0, 0);

}

CGeoPoint CGPUBestAlgThread::bestPointForBackKicks() {
	const MobileVisionT& ball = _pVision->Ball();
	const int increaseStepForR = 5;
	const double increaseStepForDir = 1 * Param::Math::PI / 180;
	const double min_Pos_X = 50;
	const double max_Pos_Y = 300;
	const double x1 = 50;
	const double y1 = -30;
	const double x2 = 0;
	const double y2 = 30;
	const double limit = 250;
	const double minChipDistance = 150;
	const double maxChipDistance = 500;
	const double minthreshold = 60;
	const double maxthreshold = 120;
	const double mindir = 10 * Param::Math::PI / 180;
	double thresholdForEnemy2ChipPointDist;
	double ball2ChipPointDist;
	double i, j, k;
	int r = ball.Pos().dist(CGeoPoint(min_Pos_X, ball.Pos().y())) + 5;
	int AntiY = ball.Pos().y() > 0 ? -1 : 1;
	CGeoPoint targetPoint;
	CGeoPoint leftgoal(450, -30 * AntiY);
	CGeoPoint rightgoal(450, 30 * AntiY);
	vector <CGeoPoint> point;
	CGeoRectangle dangerArea(x1, y1, x2, y2);
	double dist;
	double me2ChipPointDist;
	double enemy2ChipPointDist;
	double enemy2MeDist;
	double pointToTheirGoalDist;
	double startdir, enddir;
	double currentValue;
	double slope;
	double b;
	int enemyAroundChipPointNum;
	int enemyAroundMeNum;
	int getBallNum;
	int enemyNumRecord;
	int numRecord;
	bool isRunLineBlock;
	determinValueForChipKick = -999;
	startdir = (leftgoal - ball.Pos()).dir();
	enddir = (rightgoal - ball.Pos()).dir();
	for (i = r; i < maxChipDistance; i += increaseStepForR) {
		for (j = startdir; (j <= enddir || j == startdir); j += increaseStepForDir * AntiY) {
			targetPoint = ball.Pos() + Utils::Polar2Vector(i, j);
			if (!dangerArea.HasPoint(targetPoint) && targetPoint.x() <= limit) point.push_back(targetPoint);
		}
	}
	//cout << point.size() << endl;
	for (i = 0; i < point.size(); i++)
	{
		me2ChipPointDist = enemy2ChipPointDist = enemy2MeDist = 999;
		enemyAroundChipPointNum = enemyAroundMeNum = 0;
		isRunLineBlock = false;
		for (j = 1; j <= 6; j++) {
			dist = point[i].dist(_pVision->OurPlayer(j).Pos());
			if (dist < me2ChipPointDist) {
				me2ChipPointDist = dist;
				getBallNum = j;
			}
		}
		for (k = 1; k <= 6; k++) {
			dist = _pVision->TheirPlayer(k).Pos().dist(point[i]);
			if (dist < eCAPdeterminDist) enemyAroundChipPointNum++;
			if (dist < enemy2ChipPointDist)  enemy2ChipPointDist = dist;
			dist = _pVision->OurPlayer(getBallNum).Pos().dist(_pVision->TheirPlayer(k).Pos());
			if (dist < eCAMdeterminDist) enemyAroundMeNum++;
			if (dist < enemy2MeDist) {
				enemy2MeDist = dist;
				enemyNumRecord = k;
			}
			CGeoSegment seg = CGeoSegment(_pVision->OurPlayer(getBallNum).Pos(), point[i]);
			dist = _pVision->TheirPlayer(k).Pos().dist(seg.projection(_pVision->TheirPlayer(k).Pos()));
			if (seg.IsPointOnLineOnSegment(seg.projection(_pVision->TheirPlayer(k).Pos()))) {
				if (dist < 15) {
					isRunLineBlock = true;
					break;
				}
			}
		}
		ball2ChipPointDist = _pVision->Ball().Pos().dist(point[i]);
		slope = (maxthreshold - minthreshold) / (maxChipDistance - minChipDistance);
		b = minthreshold - minChipDistance * slope;
		thresholdForEnemy2ChipPointDist = slope * ball2ChipPointDist + b;
		enemy2MeDist = enemy2MeDist > thresholdForEnemy2MeChipDist ? thresholdForEnemy2MeChipDist : enemy2MeDist;
		enemy2ChipPointDist = enemy2ChipPointDist > thresholdForEnemy2ChipPointDist ? thresholdForEnemy2ChipPointDist : enemy2ChipPointDist;
		pointToTheirGoalDist = point[i].dist(CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0));
		currentValue = enemyAroundChipPointNum * eCAPweight + enemyAroundMeNum * eCAMweight + enemy2ChipPointDist * eCPDweight + enemy2MeDist * eCweight3 + ball2ChipPointDist * bCPDweight + pointToTheirGoalDist * pTTGweight + fixValueForChipPass;
		if (isRunLineBlock) currentValue /= 2;
		if (currentValue > determinValueForChipKick) {
			determinValueForChipKick = currentValue;
			numRecord = i;
		}
	}
	//cout << enemyAroundChipPointNum << "  " << enemyAroundMeNum << "  " << enemy2ChipPointDist << "  " << enemy2MeDist << "  " << ball2ChipPointDist << endl;
	//cout << fixValueForChipPass <<" " <<currentValue<<endl;
	if (determinValueForChipKick > 0) {
		ball2ChipPointDist = _pVision->Ball().Pos().dist(point[numRecord]);
		chipPower = ball2ChipPointDist * 0.7;
		fixTime = -0.0000015498 * chipPower*chipPower + 0.0025344180 * chipPower + 0.2463515283;
		fixBuf = fixTime * 60;
		cout << point[numRecord].x() << " " <<point[numRecord].y() << endl;
		GDebugEngine::Instance()->gui_debug_line(_pVision->Ball().Pos(), point[numRecord]+Utils::Polar2Vector(1000, (point[numRecord]- _pVision->Ball().Pos()).dir()));
		return point[numRecord];
	}
	else {
		chipPower = ball2ChipPointDist * 0.7;
		fixTime = -0.0000015498 * chipPower*chipPower + 0.0025344180 * chipPower + 0.2463515283;
		fixBuf = fixTime * 60;
		return CGeoPoint(450,0);
	}
}

bool CGPUBestAlgThread::isChip() {
	return determinValueForFlatKick <= 0 ? true : false;
}

bool CGPUBestAlgThread::isChip(const int robotnum) {
	const double shoot_blocked_factor = 2.5;
	const double pass_blocked_factor = 5;
	bool shoot_blocked = WorldModel::Instance()->isShootLineBlocked(robotnum, 700, shoot_blocked_factor);
	bool pass_blocked = WorldModel::Instance()->isPassLineBlocked(robotnum, pass_blocked_factor);
	return shoot_blocked || pass_blocked;
}

bool CGPUBestAlgThread::isChip(const CGeoPoint passPoint) {
	const double shoot_blocked_factor = 1.5;
	const double pass_blocked_factor = 2.7;
	const double range = 700;
	bool passLineBlocked = false;
	const MobileVisionT& ball = _pVision->Ball();
	const CGeoPoint mePos = passPoint;
	const CGeoPoint ballPos = ball.Pos();
	CGeoSegment segment1 = CGeoSegment(mePos, ballPos);
	for (int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
		if (_pVision->TheirPlayer(i).Valid()) {
			CGeoPoint interPoint = segment1.projection(_pVision->TheirPlayer(i).Pos());
			if (interPoint.dist(_pVision->TheirPlayer(i).Pos()) < Param::Vehicle::V2::PLAYER_SIZE * pass_blocked_factor && segment1.IsPointOnLineOnSegment(interPoint)) {
				passLineBlocked = true;
				break;
			}
		}
	}
	cout << passLineBlocked << " passLineBlocked" << endl;
	if (passLineBlocked)
		return true;
	else
		return false;
	bool shootLineBlocked = false;
	CGeoPoint theirGoal = CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0);
	CVector pos2goal = theirGoal - passPoint;
	double dir = pos2goal.dir();
	CGeoSegment segment2 = CGeoSegment(passPoint, passPoint + Utils::Polar2Vector(range, dir));
	for (int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
		if (_pVision->TheirPlayer(i).Valid()) {
			CGeoPoint interPoint = segment2.projection(_pVision->TheirPlayer(i).Pos());
			if (interPoint.dist(_pVision->TheirPlayer(i).Pos()) < Param::Vehicle::V2::PLAYER_SIZE * shoot_blocked_factor && segment2.IsPointOnLineOnSegment(interPoint)) {
				shootLineBlocked = true;
				break;
			}
		}
	}
	cout << shootLineBlocked << " shootLineBlocked" << endl;
	return passLineBlocked || shootLineBlocked;
}
void CGPUBestAlgThread::detectModule(bool generatePos, double detectDist, int detectCycle, bool useGoalie) {  //重要模块！
	int i, j;
	int ramdomX, ramdomY;
	const MobileVisionT& ball = _pVision->Ball();
	const int antiY = ball.Y() > 0 ? -1 : 1;
	if (scanOverFlag) jamMode = false;//
	scanOverFlag = false;
	if (_pVision->Cycle() - _cycle1 > Param::Vision::FRAME_RATE * 0.1) {
		detectMarkedOverFlag = false;
		detectAgainFlag = false;
		fakePosComputeOverFlag = false;
		for (i = 0; i <= 5; i++) {
			buf[i] = 0;
			state[i] = 0;
		}
		maxbuf = 0;
		maxDefendKickbuf = 0;
		attempts = 0;
		isChipForDynamicKick = false;
		stay = true;//LUA 初始站位后检测
		allMarkedFlag = false;
		maxAttempts = 3;
		
		if (ball.X() > 360)
			indirectKickType = CornerKick;
		else if (ball.X() > 0)
			indirectKickType = FrontKick;
		else 
			indirectKickType = MiddleKick;
			
		writeOver = false;
		useOwnPos = false;
	}
	if (attempts >= maxAttempts)  detectMarkedOverFlag = true;
	if (generatePos) {
		int Num[11] = { 999,1,2,3,4,5,6,7,8,9,10 };
		if (useGoalie) {
			ramdomNumber[4] = rand() % 3 + 7;
			Num[ramdomNumber[4]] = 999;
		}
		for (i = 1; i <= 9; i++) {
			if (area[i].HasPoint(ball.Pos())) {
				cout << "haspoint" << i << endl;
				Num[i] = 999;
				if (i <= 6)Num[i + 3] = 999;
				break;
			}
		}
		if (detectAgainFlag) Num[10] = 999;
		detectAgainFlag = false;
		detectMarkedOverFlag = false;
		stay = false;
		maxbuf = -30;//拉扯一定距离后再开始检测
		for (i = 0; i <= 5; i++) {
			buf[i] = -30;
		}
		int areaNumber;
		for (i = 0; i <= 3; i++) {
			areaNumber = (rand() % (10)) + 1;
			while (Num[areaNumber] == 999) {
				areaNumber = (rand() % (10)) + 1;
			}
			ramdomNumber[i] = Num[areaNumber];
			Num[areaNumber] = 999;
		}
		//ramdomNumber[0] = 3;
		//ramdomNumber[1] = 7;
		//ramdomNumber[2] = 8;
		//ramdomNumber[3] = 9;
		const int attackValue = useGoalie ? 4 : 3;
		for (j = 0; j <= attackValue; j++) {
			if (ramdomNumber[j] < 10) {
				do {
					ramdomX = (rand() % (int(area[ramdomNumber[j]]._point[1].x() - area[ramdomNumber[j]]._point[2].x() + 1))) + int(area[ramdomNumber[j]]._point[2].x());
					ramdomY = (rand() % (int(area[ramdomNumber[j]]._point[1].y() - area[ramdomNumber[j]]._point[0].y() + 1))) + int(area[ramdomNumber[j]]._point[0].y());
					runPos[j] = CGeoPoint(ramdomX, ramdomY);
				} while (runPos[j].dist(ball.Pos()) < 70);
			}
			else {
				runPos[j] = wreckPos[(rand() % (10)) + 0];
				runPos[j].setY(runPos[j].y()*antiY);
			}
		}
		attempts++;
		cout << "attempt " << attempts << endl;
		//runPos[0] = CGeoPoint(375, -140);
		//runPos[1] = CGeoPoint(65, -160);
		//runPos[2] = CGeoPoint(115, 296);
		//runPos[3] = CGeoPoint(223, 17);
		//cout << attempts << endl;
		cout << ramdomNumber[0] << " " << ramdomNumber[1] << " " << ramdomNumber[2] << " " << ramdomNumber[3] << endl;
		//cout << runPos[0].x() << " " << runPos[0].y() << endl;
		//cout << runPos[1].x() << " " << runPos[1].y() << endl;
		//cout << runPos[2].x() << " " << runPos[2].y() << endl;
		//cout << runPos[3].x() << " " << runPos[3].y() << endl;

	}
	if (stay) {
		int i, j;
		for (i = 0; i <= 3; i++) {
			if (_pVision->OurPlayer(number[i]).Valid()) {
				runPos[i] = _pVision->OurPlayer(number[i]).Pos();
				for (j = 1; j <= 9; j++) {
					if (area[j].HasPoint(runPos[i])) {
						ramdomNumber[i] = j;
						break;
					}
				}
				if (j > 9 && runPos[i].x() > 0) {
					ramdomNumber[i] = 10;
				}
				//cout << "ramdomNumber[0] " << ramdomNumber[0] << " ramdomNumber[1] " << ramdomNumber[1]  << " ramdomNumber[2] " << ramdomNumber[2] << " ramdomNumber[3] " << ramdomNumber[3] << endl;
			}

		}
	}
	int sum = 0;
	if (detectCycle != 999)//不改变检测侦数时Lua会传来detectCycle=999
		determinCycle = detectCycle;
	if (determinCycle < 30) 
		determinCycle = 30;
	else if (determinCycle > 300) 
		determinCycle = 300;
	const int maxCycle = determinCycle*1.3;
	for (i = 0; i <= 3; i++) {
		if (isMarkedForDynamicKick(i, detectDist) || _pVision->OurPlayer(number[i]).X() < 0 || !number[i]) {
			if (buf[i]) buf[i] = 0;
			state[i] = 0;
		}
		else {
			buf[i]++;
			if (buf[i] > determinCycle) {
				defendedTpye[i] = Free;
				state[i] = 1;
			}
			sum += state[i];
		}
	}
	if (jamMode) {
		buf[1] = 0;//挡开球车模式Special执行挡车任务，不考虑传球给他
	}
	maxbuf++;
	if (sum && (maxbuf >= maxCycle)) detectMarkedOverFlag = true;
	if (maxbuf > maxCycle&&detectDefendKickedOverFlag) detectAgainFlag = true;
	_cycle1 = _pVision->Cycle();
	//cout << buf[0] << " " << maxbuf << endl;
	//cout << "maxbuf " << maxbuf << endl;
	//cout << "maxCycle " << maxCycle << endl;
	//cout << isChipForDynamicKick << endl;
	//cout << "number " << number[0] << " " << number[1] << " " << number[2] << " " << number[3] << endl;
	//cout << "state " << state[0] << " " << state[1] << " " << state[2] << " " << state[3] << endl;
	//cout << "type " << defendedTpye[0] << " " << defendedTpye[1] << " " << defendedTpye[2] << " " << defendedTpye[3] << endl;
	//UDPSendMessage();
}

void CGPUBestAlgThread::detectDefendKickedModule(int checkAttempts, int defendKickCycle) {
	const MobileVisionT& ball = _pVision->Ball();
	const PlayerVisionT& me = _pVision->OurPlayer(number[4]);
	const int antiY = _pVision->Ball().Y() > 0 ? -1 : 1;
	const int detectdefendKickCycle = defendKickCycle;
	const int maxdefendKickCycle = defendKickCycle*1.5;
	const double startDir = (CGeoPoint(0, -300 * antiY) - ball.Pos()).dir();
	const double endDir = (CGeoPoint(450, 0) - ball.Pos()).dir();
	if (_pVision->Cycle() - _cycle2 > Param::Vision::FRAME_RATE * 0.1) {
		//cout << startDir * 180 / Param::Math::PI << " " << endDir * 180 / Param::Math::PI<<endl;
		detectDefendKickAttempts = 0;
		buf[5] = 0;
		state[5] = 0;
		maxDefendKickbuf = 0;
		assisterPos = CGeoPoint(0, -300 * antiY);
		if (fabs(ball.Y()) < 200) {
			//checkAttempts = 0;
		}
		if (checkAttempts) {
			deltaAngle = fabs(Utils::Normalize(startDir - endDir)) / checkAttempts;
			if (fabs(Utils::Normalize(startDir - checkAttempts * deltaAngle * antiY) - endDir) > 5 * Param::Math::PI / 180)
				deltaAngle = (2* Param::Math::PI - fabs(Utils::Normalize(startDir - endDir))) / checkAttempts;
			deltaAngle = Utils::Normalize(deltaAngle);
		}
		else
			deltaAngle = 0;
		flatPassAngle.clear();
		flatPassAngleFlag.clear();
		flatPassAngle.push_back(startDir);
		detectDefendKickedOverFlag = false;
		flatPassAngleFlag.push_back(false);
	}
	if (!detectDefendKickedOverFlag) {
		if (isDefendKickedForDynamicKick(number[4])) {
			buf[5] = 0;
			flatPassAngleFlag[detectDefendKickAttempts] = false;
		}
		else {
			buf[5]++;
			if (buf[5] > detectdefendKickCycle) {
				flatPassAngleFlag[detectDefendKickAttempts] = true;
			}
		}
		maxDefendKickbuf++;
		//cout << maxDefendKickbuf << " " << maxdefendKickCycle << endl;
		if (maxDefendKickbuf >= maxdefendKickCycle) {
			detectDefendKickAttempts++;
			double tempDir;
			if (detectDefendKickAttempts > checkAttempts) {
				detectDefendKickedOverFlag = true;
				detectDefendKickAttempts = checkAttempts;
				int i, sum = 0;
				for (i = 0; i < flatPassAngleFlag.size(); i++) {
					sum += flatPassAngleFlag[i];
				}
				if (sum == flatPassAngleFlag.size()) {
					determinValueForFlatKick = 99;//配合lua在只检测防开球后时调用pos.isChip()
					state[5] = 1;
				}
				else {
					determinValueForFlatKick = -99;
					state[5] = 0;
				}
			}
			tempDir = Utils::Normalize(startDir - detectDefendKickAttempts * deltaAngle * antiY);
			if (!detectDefendKickedOverFlag) {
				flatPassAngleFlag.push_back(false);
				flatPassAngle.push_back(0);
				flatPassAngle[detectDefendKickAttempts] = tempDir;
			}
			//cout << detectDefendKickAttempts << " " << tempDir * 180 / Param::Math::PI << endl;
			maxDefendKickbuf = 0;
			assisterPos = ball.Pos() + Utils::Polar2Vector(120, tempDir);
			//GDebugEngine::Instance()->gui_debug_x(assisterPos);
		}
	}
	_cycle2 = _pVision->Cycle();
}

vector <CGeoPoint> CGPUBestAlgThread::generateShootPos() {
	const MobileVisionT& ball = _pVision->Ball();
	int i, j, k;
	int range = 999;
	double Dist, minDist;
	const double determinDist = 30;
	/*
	for (i = 0; i < flatPassAngleFlag.size(); i++) {
	cout << flatPassAngle[i] << " " << flatPassAngleFlag[i] << endl;
	}
	cout << "-------------------------------------" << endl;
	//double k, l;
	const double maxRadius = 50;
	const double minRadius = 0;
	const double increaseRadius = 25;
	const double startDir = -Param::Math::PI;
	const double endDir = Param::Math::PI- Param::Math::PI/180;
	const double increaseDir = 45 * Param::Math::PI / 180;
	*/
	CGeoPoint targetPoint;
	vector <CGeoPoint> point;
	CGeoRectangle enemyDefendArea = CGeoRectangle(CGeoPoint(Param::Field::PITCH_LENGTH / 2, Param::Field::PENALTY_AREA_WIDTH / 2 + 30),
		CGeoPoint(Param::Field::PITCH_LENGTH / 2 - Param::Field::PENALTY_AREA_DEPTH - 30, -Param::Field::PENALTY_AREA_WIDTH / 2 - 30));
	if (!useOwnPos) {
		for (i = 0; i <= 10; i++) {
			openArea[i] = false;
		}
		if (CornerKick == indirectKickType) {
			int prioritySequence[11] = { 999,7,2,3,5,4,8,6,9,10,1 };
			getBallNumber = 999;
			if (ball.Y() >= 0) {
				int temp;
				for (i = 1; i <= 7; i += 3) {
					temp = prioritySequence[i];
					prioritySequence[i] = prioritySequence[i + 2];
					prioritySequence[i + 2] = temp;
				}
			}
			for (i = 0; i <= 3; i++) {
				minDist = 999;
				for (j = 1; j <= 6; j++) {
					Dist = _pVision->OurPlayer(number[i]).Pos().dist(_pVision->TheirPlayer(j).Pos());
					if (Dist < minDist&&_pVision->TheirPlayer(j).X()>_pVision->OurPlayer(number[i]).X()) minDist = Dist;
				}
				if (minDist < determinDist) state[i] = 0;
				if (state[i]) {
					if (prioritySequence[ramdomNumber[i]] < range) {
						getBallNumber = i;
						range = prioritySequence[ramdomNumber[i]];
					}
					/*
					for (k = minRadius; k <=maxRadius; k += increaseRadius) {
					for (l = startDir; l <= endDir ; l += increaseDir) {
					targetPoint = _pVision->OurPlayer(number[i]).Pos() + Utils::Polar2Vector(k, l);
					if (!enemyDefendArea.HasPoint(targetPoint)&& !Utils::OutOfField(targetPoint)) point.push_back(targetPoint);
					}
					}
					*/
				}

			}
			if (getBallNumber == 999) {
				if (attempts < maxAttempts)
					detectAgainFlag = true;
				else
					allMarkedFlag = true;
				int maxbuf = 0;
				for (i = 0; i <= 3; i++) {
					if (buf[i] > maxbuf) {
						if (!jamMode || jamMode && i != 1) {
							maxbuf = buf[i];
							getBallNumber = i;
						}
					}
				}
				if (getBallNumber == 999) {
					minDist = 999;
					if (ball.Y() >= 0)
						point.push_back(CGeoPoint(380, -110));
					else
						point.push_back(CGeoPoint(380, 110));
					shootPos = point[0];
					for (i = 1; i <= 6; i++) {
						Dist = _pVision->OurPlayer(i).Pos().dist(point[0]);
						if (Dist < minDist) {
							if (i != number[4] && i != number[5]) {
								getBallNumber = i;
								minDist = Dist;
							}
						}
					}
					for (i = 0; i <= 3; i++) {
						if (getBallNumber == number[i]) {
							getBallNumber = i;
							break;
						}
					}
				}
			}
			else {
				int passArea = ramdomNumber[getBallNumber];
				if (ball.Y() >= 0) {
					if ((passArea - 1) % 3 == 0)
						passArea += 2;
					else if (passArea % 3 == 0)
						passArea -= 2;
				}
				if (passArea == 1) openArea[2] = openArea[4] = true;
				else if (passArea == 2) openArea[2] = true;
				else if (passArea == 3) openArea[2] = true;
				else if (passArea == 4) openArea[5] = true;
				else if (passArea == 5) openArea[2] = openArea[5] = true;
				else if (passArea == 6) openArea[3] = openArea[5] = true;
				else if (passArea == 7) openArea[5] = true;
				else if (passArea == 8) openArea[5] = true;
				else if (passArea == 9) openArea[5] = true;
				else openArea[10] = openArea[2] = true;
				//判断如果对方有门神，退后点进攻
				const PlayerVisionT& me = _pVision->OurPlayer(number[getBallNumber]);
				CGeoPoint theirGoal(450, 0);
				CGeoSegment shootLine(me.Pos(), theirGoal);
				bool theyHaveDefender = false;
				for (i = 1; i <= 6; i++) {
					const PlayerVisionT& anemy = _pVision->TheirPlayer(i);
					if (anemy.Valid()) {
						if (shootLine.IsPointOnLineOnSegment(shootLine.projection(anemy.Pos()))) {
							if (shootLine.projection(anemy.Pos()).dist(anemy.Pos()) < 12 && Utils::InTheirPenaltyArea(anemy.Pos(),30)) {
								theyHaveDefender = true;
								cout << "theyHaveDefender" << endl;
								break;
							}
						}
					}
				}
				if (theyHaveDefender) {
					if (openArea[10]) {
						openArea[10] = false;
						openArea[5] = true;
					}
					for (i = 1; i <= 3; i++) {
						if (openArea[i]) {
							openArea[i] = false;
							openArea[i + 3] = true;
						}
					}
				}
				//cout << point.size() << endl;
				//cout << number[getBallNumber] <<endl;
				//for (i = 0; i < point.size(); i++) {
				//GDebugEngine::Instance()->gui_debug_x(point[i], COLOR_BLACK);
				//}
			}
		}
		else if (FrontKick == indirectKickType) {
			cout << "frontkick" << endl;
				int prioritySequence[11] = { 999,6,7,2,8,3,4,5,9,10,1 };
			getBallNumber = 999;
			if (ball.Y() >= 0) {
				int temp;
				for (i = 1; i <= 7; i += 3) {
					temp = prioritySequence[i];
					prioritySequence[i] = prioritySequence[i + 2];
					prioritySequence[i + 2] = temp;
				}
			}
			for (i = 0; i <= 3; i++) {
				minDist = 999;
				for (j = 1; j <= 6; j++) {
					Dist = _pVision->OurPlayer(number[i]).Pos().dist(_pVision->TheirPlayer(j).Pos());
					if (Dist < minDist&&_pVision->TheirPlayer(j).X()>_pVision->OurPlayer(number[i]).X()) minDist = Dist;
				}
				if (minDist < determinDist) state[i] = 0;
				if (state[i]) {
					if (prioritySequence[ramdomNumber[i]] < range) {
						getBallNumber = i;
						range = prioritySequence[ramdomNumber[i]];
					}
				}

			}
			if (getBallNumber == 999) {
				if (attempts < maxAttempts && fabs(ball.Y() > 200)) {
					detectAgainFlag = true;
				}
				else
					allMarkedFlag = true;
				int maxbuf = 0;
				for (i = 0; i <= 3; i++) {
					if (buf[i] > maxbuf) {
						if (!jamMode || jamMode && i != 1) {
							maxbuf = buf[i];
							getBallNumber = i;
						}
					}
				}
				if (getBallNumber == 999) {
					minDist = 999;
					if (ball.Y() >= 0)
						point.push_back(CGeoPoint(380, -110));
					else
						point.push_back(CGeoPoint(380, 110));
					shootPos = point[0];
					for (i = 1; i <= 6; i++) {
						Dist = _pVision->OurPlayer(i).Pos().dist(point[0]);
						if (Dist < minDist) {
							if (i != number[4] && i != number[5]) {
								getBallNumber = i;
								minDist = Dist;
							}
						}
					}
					for (i = 0; i <= 3; i++) {
						if (getBallNumber == number[i]) {
							getBallNumber = i;
							break;
						}
					}
				}
			}
			else {
				int passArea = ramdomNumber[getBallNumber];
				if (ball.Y() >= 0) {
					if ((passArea - 1) % 3 == 0)
						passArea += 2;
					else if (passArea % 3 == 0)
						passArea -= 2;
				}
				if (passArea == 1) {
					openArea[3] = openArea[5] = true;
					if (ball.X() > startX + 2 * lengthstep) openArea[2] = true;
				}
				else if (passArea == 2) {
					openArea[3] = openArea[5] = openArea[10] = true;
					if (ball.X() > startX + 2 * lengthstep) openArea[2] = true;
				}
				else if (passArea == 3) {
					openArea[3] = openArea[5] = openArea[10] = true;
					if (ball.X() > startX + 2 * lengthstep) openArea[2] = true;
				}
				else if (passArea == 4) openArea[3] = openArea[5] = true;
				else if (passArea == 5) {
					openArea[3] = openArea[5] = openArea[6] = true;
					if (ball.X() > startX + 2 * lengthstep) openArea[2] = true;
				}
				else if (passArea == 6) openArea[3] = openArea[5] = openArea[6] = true;
				else if (passArea == 7) openArea[5] = openArea[3] = true;
				else if (passArea == 8) openArea[5] = openArea[6] = true;
				else if (passArea == 9) openArea[5] = openArea[6] = true;
				else openArea[10] = openArea[3] = true;
			}
		}
		if (ball.Pos().y() >= 0) {
			bool temp;
			for (i = 1; i <= 7; i += 3) {
				temp = openArea[i];
				openArea[i] = openArea[i + 2];
				openArea[i + 2] = temp;
			}
		}
		for (i = 1; i <= 9; i++) {
			if (openArea[i]) {
				for (j = area[i]._point[2].x() + 20; j < area[i]._point[1].x(); j += 40) {
					for (k = area[i]._point[0].y() + 20; k < area[i]._point[1].y(); k += 40) {
						targetPoint = CGeoPoint(j, k);
						if (!enemyDefendArea.HasPoint(targetPoint) && !Utils::OutOfField(targetPoint)) point.push_back(targetPoint);
					}
				}
			}
		}
		if (openArea[10]) {
			for (i = 0; i <= 9; i++) {
				targetPoint = wreckPos[i];
				if (ball.Y() > 0) targetPoint.setY(targetPoint.y()*-1);
				point.push_back(targetPoint);
			}
		}
	}
	else {
		point = luaPos;
		for (i = 0; i <= 3; i++) {
			if (number[i] == pickNumber) {
				getBallNumber = i;
				break;
			}
		}
	}
	return point;
}

void CGPUBestAlgThread::scanModule() {
	detectAgainFlag = false;
	const MobileVisionT& ball = _pVision->Ball();
	int i, j, k;
	double minDist;
	double minDistToShootLine;
	int numRecord;
	int enemyNumRecord;
	int enemyAroundPointNum;
	int enemyAroundMeNum;;
	double dist;
	double dist2;
	double enemy2PointDist;
	double me2PointDist;
	double ball2PointDist;
	double ball2PointDir;
	double ball2RealPointDist;
	double enemy2MeDist;
	double crossNum;
	double currentValue;
	const double minShootDir = 5 * Param::Math::PI / 180;
	double _raw_kick_dir;
	double determindir;
	double tempdir;
	int size;
	double sizeOfDir;
	double metoballdir;
	bool Test = false;//测试模式
	bool isPassLineBlock;
	bool canFlagPassTo = true;
	const CGeoPoint theirGoal(Param::Field::PITCH_LENGTH / 2, 0);
	double sum = 0;
	vector <CGeoPoint> point = generateShootPos();
	const PlayerVisionT& me = _pVision->OurPlayer(number[getBallNumber]);
	for (i = 0; i < flatPassAngleFlag.size(); i++) {
		sum += flatPassAngleFlag[i];
		if (!flatPassAngleFlag[i] && !jamMode)
			canFlagPassTo = false;
	}
	if (!sum && !jamMode) isChipForDynamicKick = true;
	//isChipForDynamicKick = true;
	if (!isChipForDynamicKick) {
		size = point.size();
		determinValueForFlatKick = -999;
		determinValueForChipKick = -999;
		for (i = 0; i < size; i++)
		{
			enemy2PointDist = 1100;
			me2PointDist = 1100;
			enemy2MeDist = 1100;
			minDist = 1100;
			minDistToShootLine = 1100;
			crossNum = 0;
			enemyAroundPointNum = 0;
			enemyAroundMeNum = 0;
			isPassLineBlock = false;
			me2PointDist = me.Pos().dist(point[i]);
			CShootRangeList shootRangeList(_pVision, number[getBallNumber], point[i]);
			const CValueRange* bestRange = NULL;
			const CValueRangeList& shootRange = shootRangeList.getShootRange();
			if (shootRange.size() > 0) {
				bestRange = shootRange.getMaxRangeWidth();
				sizeOfDir = bestRange->getSize();
				if (bestRange && bestRange->getWidth() > 0) {	// 要求射门空档足够大
					_raw_kick_dir = bestRange->getMiddle();
				}
				else {
					_raw_kick_dir = (CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0) - point[i]).dir();
				}
				double metoballdir = (ball.Pos() - point[i]).dir();
				determindir = fabs(Utils::Normalize(metoballdir - _raw_kick_dir));
			}
			else {
				_raw_kick_dir = (CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0) - point[i]).dir();
				sizeOfDir = 0;
			}
			CGeoLine shootLine(point[i], _raw_kick_dir);
			CGeoSegment seg = CGeoSegment(ball.Pos(), point[i]);
			CGeoSegment seg2 = CGeoSegment(me.Pos(), point[i]);
			for (k = 1; k <= 6; k++) {
				if (_pVision->TheirPlayer(k).Valid()) {
					dist = _pVision->TheirPlayer(k).Pos().dist(point[i]);
					if (dist < determinDist1) enemyAroundPointNum++;
					if (dist < enemy2PointDist)  enemy2PointDist = dist;
					dist = me.Pos().dist(_pVision->TheirPlayer(k).Pos());
					if (dist < determinDist2) enemyAroundMeNum++;
					if (dist < enemy2MeDist) {
						enemy2MeDist = dist;
						enemyNumRecord = k;
					}
					dist = _pVision->TheirPlayer(k).Pos().dist(shootLine.projection(_pVision->TheirPlayer(k).Pos()));
					if (dist < minDistToShootLine) {
						minDistToShootLine = dist;
					}
					dist = _pVision->TheirPlayer(k).Pos().dist(seg.projection(_pVision->TheirPlayer(k).Pos()));
					dist2 = _pVision->TheirPlayer(k).Pos().dist(seg2.projection(_pVision->TheirPlayer(k).Pos()));
					if (seg.IsPointOnLineOnSegment(seg.projection(_pVision->TheirPlayer(k).Pos())) || seg2.IsPointOnLineOnSegment(seg2.projection(_pVision->TheirPlayer(k).Pos()))) {
						if (dist < 12 || dist2 < 15) {
							isPassLineBlock = true;
							break;
						}
						else {
							if (dist < minDist) minDist = dist;
							crossNum++;
						}
					}
				}
			}
			if (jamMode) {
				dist = _pVision->OurPlayer(number[1]).Pos().dist(seg.projection(_pVision->OurPlayer(number[1]).Pos()));
				if (seg.IsPointOnLineOnSegment(seg.projection(_pVision->TheirPlayer(k).Pos())) && dist < 15)
					isPassLineBlock = true;
			}
			ball2PointDist = ball.Pos().dist(point[i]);
			ball2PointDir = (point[i] - ball.Pos()).dir();
			//
			for (j = 0; j < flatPassAngleFlag.size(); j++) {
				if (flatPassAngleFlag[j]) {
					if (j != flatPassAngleFlag.size() - 1 && flatPassAngleFlag[j + 1]) {
						if (flatPassAngle[j] > flatPassAngle[j + 1]) {
							if (fabs(flatPassAngle[j] - flatPassAngle[j + 1]) > Param::Math::PI) {
								if (ball2PointDir >= flatPassAngle[j] || ball2PointDir <= flatPassAngle[j + 1])
									canFlagPassTo = true;
							}
							else {
								if (ball2PointDir >= flatPassAngle[j + 1] && ball2PointDir <= flatPassAngle[j])
									canFlagPassTo = true;
							}
						}
						else {
							if (fabs(flatPassAngle[j] - flatPassAngle[j + 1]) > Param::Math::PI) {
								if (ball2PointDir >= flatPassAngle[j + 1] || ball2PointDir <= flatPassAngle[j])
									canFlagPassTo = true;
							}
							else {
								if (ball2PointDir >= flatPassAngle[j] && ball2PointDir <= flatPassAngle[j + 1])
									canFlagPassTo = true;
							}
						}
					}
					else {
						double left = Utils::Normalize(flatPassAngle[j] - 5 * Param::Math::PI / 180);
						double right = Utils::Normalize(flatPassAngle[j] + 5 * Param::Math::PI / 180);
						if (fabs(left - right) > Param::Math::PI) {
							if (left > right) {
								if (ball2PointDir >= left || ball2PointDir <= right) {
									canFlagPassTo = true;
								}
							}
							else {
								if (ball2PointDir >= right || ball2PointDir <= left) {
									canFlagPassTo = true;
								}
							}
						}
						else {
							if (left > right) {
								if (ball2PointDir >= right && ball2PointDir <= left) {
									canFlagPassTo = true;
								}
							}
							else {
								if (ball2PointDir >= left && ball2PointDir <= right) {
									canFlagPassTo = true;
								}
							}
						}
					}
				}

			}
			//cout << determinValueForFlatKick << endl;
			//
			//cout << ball2PointDir << " " << canFlagPassTo << endl;
			minDist = minDist > thresholdForFlatMinDist ? thresholdForFlatMinDist : minDist;
			enemy2MeDist = enemy2MeDist > thresholdForEnemy2MeFlatDist ? thresholdForEnemy2MeFlatDist : enemy2MeDist;
			enemy2PointDist = enemy2PointDist > thresholdForEnemy2PointDist ? thresholdForEnemy2PointDist : enemy2PointDist;
			if (isPassLineBlock || minDist < determinDist3 || enemy2PointDist < determinDist4 || ((enemy2MeDist < determinDist5) && (_pVision->TheirPlayer(enemyNumRecord).Pos().x() > _pVision->OurPlayer(number[getBallNumber]).Pos().x())) || sizeOfDir < minShootDir || ball2PointDist < 150 || determindir>75 * Param::Math::PI / 180 || !canFlagPassTo) {
				currentValue = 0;
				if (allMarkedFlag && canFlagPassTo && !jamMode)
					currentValue = 999;
				if (Test) {
					if (isPassLineBlock) cout << i << " " << point[i].x() << " " << point[i].y() << " " << "block" << endl;
					if (minDist < determinDist3) cout << i << " " << point[i].x() << " " << point[i].y() << " " << "minDist < determinDist3" << endl;
					if (enemy2PointDist < determinDist4) cout << i << " " << point[i].x() << " " << point[i].y() << " " << "enemy2PointDist < determinDist4" << endl;
					if (((enemy2MeDist < determinDist5) && (_pVision->TheirPlayer(enemyNumRecord).Pos().x() > _pVision->OurPlayer(number[getBallNumber]).Pos().x()))) cout << i << " " << point[i].x() << " " << point[i].y() << " " << "enemy2MeDist < determinDist5" << endl;
					if (sizeOfDir < minShootDir) cout << i << " " << point[i].x() << " " << point[i].y() << " " << "sizeOfDir < minShootDir" << endl;
					if (!canFlagPassTo) cout << "cannot flatpass" << endl;
				}
			}
			else {
				currentValue = enemyAroundPointNum * dWeight1 +  minDist * dWeight3 + enemy2PointDist * dWeight4 + enemy2MeDist * dWeight5 + crossNum * dWeight6 + sizeOfDir * dWeight7 + determindir * dWeight8 + ball2PointDist * dWeight9 + me2PointDist * dWeight10 + minDistToShootLine * dWeight11 +dFixValueForFlatPass;
				if (Test) cout << i << " " << point[i].x() << " " << point[i].y() << " " << "ok" << endl;
			}
			if (currentValue > determinValueForFlatKick) {
				tempdir = _raw_kick_dir;
				determinValueForFlatKick = currentValue;
				numRecord = i;
				//cout << determindir*180/3.1415926 << endl;
			}
		}
		GDebugEngine::Instance()->gui_debug_line(point[numRecord], point[numRecord] + Utils::Polar2Vector(1000, tempdir));
	}
	else {
		size = point.size();
		determinValueForFlatKick = 0;
		determinValueForChipKick = 0;
		for (i = 0; i < size; i++) {
			enemy2PointDist = 9999;
			me2PointDist = 9999;
			enemy2MeDist = 9999;
			enemyAroundPointNum = 0;
			enemyAroundMeNum = 0;
			me2PointDist = me.Pos().dist(point[i]);
			ball2PointDist = ball.Pos().dist(point[i]);
			for (k = 1; k <= 6; k++) {
				dist = _pVision->TheirPlayer(k).Pos().dist(point[i]);
				if (dist < determinDist1) enemyAroundPointNum++;
				if (dist < enemy2PointDist)  enemy2PointDist = dist;
				dist = me.Pos().dist(_pVision->TheirPlayer(k).Pos());
				if (dist < determinDist2) enemyAroundMeNum++;
				if (dist < enemy2MeDist) {
					enemy2MeDist = dist;
					enemyNumRecord = k;
				}
			}

			CShootRangeList shootRangeList2(_pVision, number[getBallNumber], point[i]);
			const CValueRange* bestRange2 = NULL;
			const CValueRangeList& shootRange2 = shootRangeList2.getShootRange();
			if (shootRange2.size() > 0) {
				bestRange2 = shootRange2.getMaxRangeWidth();
				sizeOfDir = bestRange2->getSize();
				if (bestRange2 && bestRange2->getWidth() > 0) {	// 要求射门空档足够大
					_raw_kick_dir = bestRange2->getMiddle();
				}
				metoballdir = (ball.Pos() - point[i]).dir();
				determindir = fabs(Utils::Normalize(metoballdir - _raw_kick_dir));
			}
			else {
				sizeOfDir = 0;
			}
			minDist = minDist > thresholdForFlatMinDist ? thresholdForFlatMinDist : minDist;
			enemy2MeDist = enemy2MeDist > thresholdForEnemy2MeFlatDist ? thresholdForEnemy2MeFlatDist : enemy2MeDist;
			enemy2PointDist = enemy2PointDist > 150 ? 150 : enemy2PointDist;
			if (sizeOfDir < minShootDir || determindir>70 * Param::Math::PI / 180 || ball2PointDist < 150)
				currentValue = 0;
			else
				currentValue = enemyAroundPointNum * weight1 + enemy2PointDist*1.2  + crossNum * weight6 + sizeOfDir * 200 + determindir*(-80) + ball2PointDist*(-0.5) + 5000;
			if (currentValue > determinValueForChipKick) {
				determinValueForChipKick = currentValue;
				numRecord = i;
				ball2RealPointDist = ball.Pos().dist(point[numRecord]);
				//cout << sizeOfDir * 180 / Param::Math::PI << " " << determindir * 180 / Param::Math::PI << endl;
			}
		}
	}
	//cout <<  determinvalue << endl;



	//
	if (determinValueForFlatKick == 0) {
		isChipForDynamicKick = true;
	}
	if (determinValueForFlatKick > 0) {
		shootPos = point[numRecord];
		scanOverFlag = true;
	}
	else if (determinValueForChipKick > 0) {
		shootPos = point[numRecord];
		chipPower = ball2RealPointDist * 0.65;
		fixTime = -0.0000023171*chipPower*chipPower + 0.0031183100*chipPower + 0.1231603074;
		fixBuf = fixTime * 60;
		scanOverFlag = true;
		cout << chipPower << " " << fixTime << " " << ball2RealPointDist << endl;
		//shootPos = CGeoPoint(345, -90);
	}
	else if (!determinValueForFlatKick && !determinValueForChipKick) {
		cout << "crash" << endl;
		if (ball.Pos().y() > 0)
			shootPos = CGeoPoint(345, -90);
		else
			shootPos = CGeoPoint(345, 90);
		ball2RealPointDist = ball.Pos().dist(shootPos);
		chipPower = ball2RealPointDist * 0.65;
		fixTime = -0.0000015498 * chipPower*chipPower + 0.0025344180 * chipPower + 0.2463515283;
		fixBuf = fixTime * 60;
		scanOverFlag = true;
		cout << chipPower << " " << fixTime << " " << ball2RealPointDist << endl;
	}
	if (scanOverFlag) {
		if (getBallNumber) {
			CGeoPoint tempPos;
			tempPos = _pVision->OurPlayer(number[getBallNumber]).Pos();
			runPos[getBallNumber] = _pVision->OurPlayer(number[0]).Pos();
			runPos[0] = tempPos;
		}
		jamMode = false;
	}
	cout << determinValueForFlatKick << " " << determinValueForChipKick << " " << isChipForDynamicKick << " " << number[getBallNumber] << endl;
	//cout << state[0] << " " << state[1] << " " << state[2] << " " << state[3] << endl;
	//cout << number[0] << " " << number[1] << " " << number[2] << " " << number[3] << endl;
	//cout << determinValueForFlatKick << " " << determinValueForChipKick << endl;
	//cout << buf[0] << " " << buf[1] << " " << buf[2] << " " << buf[3] << endl;
	//cout << determinValueForFlatKick <<" "<< determinValueForChipKick<<endl;
	//cout << "--------------------" << endl;
	//cout << number[getBallNumber] << " " << shootPos.x() << " " << shootPos.y() << endl;
	GDebugEngine::Instance()->gui_debug_x(shootPos, COLOR_YELLOW);
}

void CGPUBestAlgThread::fakePosComputeModule() {
	const PlayerVisionT& me = _pVision->OurPlayer(number[getBallNumber]);
	const MobileVisionT& ball = _pVision->Ball();
	int i;
	int ramdomX, ramdomY;
	int runArea[3];
	bool standAtWreckPos = false;
	const double antiY = _pVision->Ball().Y() > 0 ? -1 : 1;
	runPos[0] = shootPos;
	if (CornerKick == indirectKickType) {
		for (i = 0; i <= 3; i++) {
			if (ramdomNumber[i] == 10 && i != getBallNumber)
				standAtWreckPos = true;
		}
		if (area[3].HasPoint(shootPos)) {
			if (standAtWreckPos) {
				runArea[0] = 10;
			}
			else {
				runArea[0] = 5;
			}
			runArea[1] = 8;
			runArea[2] = 9;
		}
		else if (area[2].HasPoint(shootPos)) {
			if (standAtWreckPos) {
				runArea[0] = 10;
			}
			else {
				runArea[0] = 8;
			}
			runArea[1] = 4;
			runArea[2] = 9;
		}
		else if (area[5].HasPoint(shootPos)) {
			runArea[0] = 7;
			runArea[1] = 7;
			runArea[2] = 9;
		}
		else if (area[4].HasPoint(shootPos)) {
			runArea[0] = 8;
			runArea[1] = 8;
			runArea[2] = 9;
		}
		else if (area[7].HasPoint(shootPos)) {
			runArea[0] = 3;
			runArea[1] = 6;
			runArea[2] = 9;
		}
		else {
			runArea[0] = 7;
			runArea[1] = 8;
			runArea[2] = 9;
		}
	}
	else if (FrontKick == indirectKickType) {
		if (area[3].HasPoint(shootPos)) {
			runArea[0] = 8;
			runArea[1] = 8;
			runArea[2] = 9;
		}
		else if (area[5].HasPoint(shootPos)) {
			runArea[0] = 7;
			runArea[1] = 7;
			runArea[2] = 9;
		}
		else if (area[6].HasPoint(shootPos)) {
			runArea[0] = 8;
			runArea[1] = 8;
			runArea[2] = 8;
		}
		else if (area[10].HasPoint(shootPos)) {
			runArea[0] = 7;
			runArea[1] = 8;
			runArea[2] = 9;
		}
		else {
			runArea[0] = 8;
			runArea[1] = 8;
			runArea[2] = 9;
		}
	}
	
	for (i = 0; i <= 2; i++) {
		if (runArea[i] == 10) {
			runPos[i + 1] = wreckPos[(rand() % (10)) + 0];
			runPos[i + 1].setY(runPos[i + 1].y()*antiY);
		}
		else {
			ramdomX = (rand() % (int(area[runArea[i]]._point[1].x() - area[runArea[i]]._point[2].x() + 1))) + int(area[runArea[i]]._point[2].x());
			ramdomY = (rand() % (int(area[runArea[i]]._point[1].y() - area[runArea[i]]._point[0].y() + 1))) + int(area[runArea[i]]._point[0].y());
			runPos[i + 1] = CGeoPoint(ramdomX, ramdomY);
			runPos[i + 1].setY(runPos[i + 1].y()*antiY);
		}
	}
	//输出文件
	if (!writeOver) {
		int i;
		out.open("dynamicKickStatus.txt", std::ios::app);
		out << "ball.Pos()   " << _pVision->Ball().Pos() << endl;
		out << "indirectKickType " << indirectKickType << endl;
		out << "number       " << number[0] << " " << number[1] << " " << number[2] << " " << number[3] << endl;
		out << "areaNumber   " << ramdomNumber[0] << " " << ramdomNumber[1] << " " << ramdomNumber[2] << " " << ramdomNumber[3] << endl;
		out << "state        " << state[0] << " " << state[1] << " " << state[2] << " " << state[3] << endl;
		out << "runPos " << "CGeoPoint(" << runPos[0].x() << " , " << runPos[0].y() << ") ";
		out << "CGeoPoint(" << runPos[1].x() << " , " << runPos[1].y() << ") ";
		out << "CGeoPoint(" << runPos[2].x() << " , " << runPos[2].y() << ") ";
		out << "CGeoPoint(" << runPos[3].x() << " , " << runPos[3].y() << ") " << endl;
		out << "AssisterAngle  ";
		for (i = 0; i < flatPassAngle.size(); i++) {
			out << flatPassAngle[i] << " ";
		}
		out << endl;
		out << "notDefendKicked ";
		for (i = 0; i < flatPassAngleFlag.size(); i++) {
			out << flatPassAngleFlag[i] << " ";
		}
		out << endl;
		out << "pick " << number[getBallNumber] << endl;
		out << "openArea ";
		for (i = 0; i <= 10; i++) {
			if (openArea[i]) {
				out << i << " ";
			}
		}
		out << endl;
		out << "determinValueForFlatKick " << "determinValueForChipKick " << "isChipForDynamicKick " << "getballNumber " << endl;
		out << determinValueForFlatKick << " " << determinValueForChipKick << " " << isChipForDynamicKick << " " << number[getBallNumber] << endl;
		out << "shootPos " << "CGeoPoint(" << shootPos.x() << " , " << shootPos.y() << ")" << endl;
		out << "runArea " << "getBall " << runArea[0] << " " << runArea[1] << " " << runArea[2] << endl;
		out << endl;
		out.close();
		writeOver = true;
	}
}

void CGPUBestAlgThread::defendPosComputeModule() {
	const PlayerVisionT& me = _pVision->OurPlayer(number[0]);
	const MobileVisionT& ball = _pVision->Ball();
	CGeoSegment passSegment(ball.Pos(), shootPos);
	CGeoSegment runSegment(me.Pos(), shootPos);
	for (int i = 1; i <= 3; i++) {
		const PlayerVisionT& runBackVehicle = _pVision->OurPlayer(number[i]);
		for (int j = -Param::Math::PI / 2; Utils::Normalize(j) <= -Param::Math::PI / 2 || Utils::Normalize(j) >= Param::Math::PI / 2; j -= 15 * Param::Math::PI / 180) {
			CGeoLine runBackLine(runBackVehicle.Pos(), Utils::Normalize(j));
		}
	}



	//bool a = CGeoLineLineIntersection(passLine, Segment).Intersectant();

}

void CGPUBestAlgThread::numberReceive(int first, int second, int third, int fourth, int assister, int goalie) {
	number[0] = first;
	number[1] = second;
	number[2] = third;
	number[3] = fourth;
	number[4] = assister;
	number[5] = goalie;
	//cout << "number "<<number[0] << " " << number[1] << " " << number[2] << " " << number[3] << " " << number[4] << endl;
}

void CGPUBestAlgThread::receiveOwnPos(vector <CGeoPoint> myPos,  int number) {
	useOwnPos = true;
	luaPos = myPos;
	pickNumber = number;
}

bool CGPUBestAlgThread::isMarkedForDynamicKick(int num, double determindist) {
	const MobileVisionT& ball = _pVision->Ball();
	const PlayerVisionT& me = _pVision->OurPlayer(number[num]);
	double closestDist = 9999;
	int i;
	for (i = 1; i <= 6; i++) {
		if (_pVision->TheirPlayer(i).Valid()) {
			double dirMe2Goal = (CGeoPoint(Param::Field::PITCH_LENGTH / 2.0, 0) - me.Pos()).dir();
			double dirMe2Ball = (ball.Pos() - me.Pos()).dir();
			double dirMe2Enemy = (_pVision->TheirPlayer(i).Pos() - _pVision->OurPlayer(number[num]).Pos()).dir();
			double different1 = fabs(Utils::Normalize(dirMe2Goal - dirMe2Enemy));
			double different2 = fabs(Utils::Normalize(dirMe2Ball - dirMe2Enemy));
			closestDist = me.Pos().dist(_pVision->TheirPlayer(i).Pos());
			//cout<<closestDist<<endl;
			if (different1 <= Param::Math::PI / 2 && CornerKick == indirectKickType || different1 <= Param::Math::PI / 3) {
				//cout << dirMe2Goal << " " << dirMe2Enemy << " " << different << " " << closestDist <<" "<<number<< endl;
				if (closestDist < determindist) {
					defendedTpye[num] = ShotBlocked;
					return true;
				}
			}
			else if (different2 <= Param::Math::PI / 3 && (FrontKick == indirectKickType || MiddleKick == indirectKickType)) {
				if (closestDist < determindist) {
					defendedTpye[num] = PassBlocked;
					return true;
				}
			}
		}
	}
	return false;
}

bool CGPUBestAlgThread::isDefendKickedForDynamicKick(int number) {
	int i;
	CGeoPoint facePos = _pVision->OurPlayer(number).Pos() + Utils::Polar2Vector(120, _pVision->OurPlayer(number).Dir());
	CGeoSegment passLine = CGeoSegment(_pVision->Ball().Pos(), facePos);
	GDebugEngine::Instance()->gui_debug_line(_pVision->Ball().Pos(), facePos, COLOR_BLUE);
	for (i = 1; i <= 6; i++) {
		if (_pVision->TheirPlayer(i).Valid()) {
			double dist = passLine.projection(_pVision->TheirPlayer(i).Pos()).dist(_pVision->TheirPlayer(i).Pos());
			bool isprjon = false;
			if (passLine.IsPointOnLineOnSegment(passLine.projection(_pVision->TheirPlayer(i).Pos()))) isprjon = true;
			if (dist < 15 && isprjon) {
				if (_pVision->TheirPlayer(i).Pos().dist(_pVision->Ball().Pos()) < 70) {
					theirDefendKickerPos = _pVision->TheirPlayer(i).Pos();
					return true;
				}
			}
		}
	}
	return false;
}

CGeoPoint CGPUBestAlgThread::generateJamPos() {
	jamMode = true; //使用挡车战术
	return CGeoPoint(theirDefendKickerPos.x() - 20, theirDefendKickerPos.y());
}

bool CGPUBestAlgThread::isDefendedHead() {
	double maxPosX = 0;
	int theirDefenderNumber;
	bool enemyDefendHead;
	for (int i = 1; i <= 6; i++) {
		CGeoPoint enemyPos = _pVision->TheirPlayer(i).Pos();
		if (enemyPos.x() > maxPosX && !Utils::InTheirPenaltyArea(enemyPos, 0)) {
			maxPosX = enemyPos.x();
			theirDefenderNumber = i;
		}
	}
	if (_pVision->TheirPlayer(theirDefenderNumber).Y() > 0)
		enemyDefendHead = true;
	else
		enemyDefendHead = false;
	if (_pVision->Ball().Y() > 0)
		enemyDefendHead = !enemyDefendHead;
	return enemyDefendHead;
}

bool CGPUBestAlgThread::isMarked(int number) {
	for (int i = 0; i <= 3; i++) {
		if (this->number[i] == number) {
			cout << !state[i] << endl;
			return !state[i];
		}
	}
}

bool CGPUBestAlgThread::canPass() {
	const PlayerVisionT& me = _pVision->OurPlayer(number[0]);
	const PlayerVisionT& assister = _pVision->OurPlayer(number[4]);
	const CRobotCapability* robotCap = RobotCapFactory::Instance()->getRobotCap(_pVision->Side(), number[0]);
	const double MaxAcceleration = robotCap->maxAcceleration(CVector(0, 0), 0);
	const double MaxSpeed = robotCap->maxSpeed(0);
	const double CM_PREDICT_FACTOR = 1.5;
	const double ball2ShootPos = _pVision->Ball().Pos().dist(shootPos);
	double slack;
	double trobot = expectedCMPathTime(me, shootPos, 580, 350, CM_PREDICT_FACTOR);
	double tball;
	if (isChipForDynamicKick) {
		tball = fixTime * 1.55;
		trobot = trobot ;
	}
	else {
		double ballspeed = ball2ShootPos*1 + 100;
		if (ballspeed < 200) {
			ballspeed = 200;
		}
		else if (ballspeed > 600) {
			ballspeed = 600;
		}
		tball = (-ballspeed+sqrt(ballspeed*ballspeed-2*FRICTION*ball2ShootPos)) / (-FRICTION);
		cout << "trobot " << "tball" << endl;
		cout << trobot << " "<<tball<<endl;
		tball *= 0.95;
		//cout << tball << endl;
		//const double assister2ShootPosdifferent = fabs((Utils::Normalize(assister.Dir() - (shootPos - assister.Pos()).dir())));
		//cout << assister2ShootPosdifferent*180/ Param::Math::PI << endl;
		/*double k, b;
		double value1, value2, dir1, dir2;
		value1 = 0.2;
		dir1 = 10 * Param::Math::PI / 180;
		value2 = 1;
		dir2 = 100 * Param::Math::PI / 180;
		k = (value2 - value1) / (dir2 - dir1);
		b = value1 - k*dir1;
		slack = k*assister2ShootPosdifferent + b;*/
		tball += 1;
	}
	//cout << "trobot " << "tball" << endl;
	//cout << trobot << " "<<tball<<endl;
	//cout << number[0] << endl;

	//cout << "number[0] " << number[0] <<  " number[1] " << number[1] << " number[2] " << number[2] << " number[3] " << number[3] << " number[4] " << number[4] << " number[5] " << number[5] << endl;
	if (trobot < tball || _pVision->OurPlayer(number[0]).Pos().dist(shootPos) < 15)
		return true;
	else
		return false;
}

bool UDPSendMessage() {
	/*static UDPSocket* socket = NULL;
	if (socket == NULL) {
	socket = new UDPSocket();
	socket->init(222);
	}
	char data[] = "abcdefg123";
	socket->sendData(2222, data,sizeof(data),"127.0.0.1");*/
	return true;
}

const CGeoRectangle* CGPUBestAlgThread::getRunArea() {
	vector <CGeoRectangle> runArea;
	for (int i= 1; i <= size(area); i++) {
		runArea.push_back(area[i]);
	}
	return area;
}

CGeoPoint CGPUBestAlgThread::generateRunPos(int areaNumber) {
	CGeoPoint runPos;
	const int antiY = _pVision->Ball().Y() > 0 ? -1 : 1;
	if (areaNumber < 1 || areaNumber > 10) {
		areaNumber = rand() % (3) + 7;
	}
	if (areaNumber < 10) {
		do {
			const int ramdomX = (rand() % (int(area[areaNumber]._point[1].x() - area[areaNumber]._point[2].x() + 1))) + int(area[areaNumber]._point[2].x());
			const int ramdomY = (rand() % (int(area[areaNumber]._point[1].y() - area[areaNumber]._point[0].y() + 1))) + int(area[areaNumber]._point[0].y());
			runPos = CGeoPoint(ramdomX, ramdomY);
		} while (runPos.dist(_pVision->Ball().Pos()) < 70);
	}
	else {
		runPos = wreckPos[(rand() % (10)) + 0];
		runPos.setY(runPos.y()*antiY);
	}
	return runPos;
}

void CGPUBestAlgThread::changeLeaderOver(bool finishFlag) {
	changeLeaderOverFlag = finishFlag;
	changeLeaderFlag = false;
}