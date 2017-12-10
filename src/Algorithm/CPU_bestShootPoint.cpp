#include <stdlib.h>
#include "GDebugEngine.h"
#include "geometry.h"
#include <iostream>

namespace{
	const bool debug = false;//ע����Ǻ���Ϥ��δ��룬���ܴ򿪿��أ��������������ϵͳ����
};

#define  C_POINT_PASS_OFF 200
#define  C_MAX_PLAYER		6
#define  PI         3.1415926

/************************************************************************/
/* ��ȫ����Ҫ�����ݷ��䵽�����Ĵ����У�Ҫ����      */
/* ���˺����и�ֵ                                  */
/************************************************************************/
CGeoPoint C_ourPlayer_Geo[6];
CGeoPoint C_theirPlayer_Geo[6];
CGeoPoint C_ball_Geo;
/************************************************************************/
/* ���ۺ�������������                              */
/************************************************************************/
void C_evaluateReceive_Geo(const CGeoPoint p, float& reverseReceiveP) {
	float dist_ball2p = p.dist(C_ball_Geo);
	float dir_ball2p = (p - C_ball_Geo).dir();
	float minAngleDiff = 1000;
	//1
	float dist_opp2p = (p - C_theirPlayer_Geo[0]).mod();
	CVector ball2opp = C_theirPlayer_Geo[0] - C_ball_Geo;
	float dist_ball2opp = ball2opp.mod();
	float dir_ball2opp = ball2opp.dir();			
	float absDirDiff = fabs(CNormalize(dir_ball2opp - dir_ball2p));
	if (absDirDiff < minAngleDiff && dist_ball2opp < dist_ball2p) {
		minAngleDiff = absDirDiff;// ���Դ��ĽǶȿյ����루��Ҫ���۲�����
	}
	reverseReceiveP += 50.0 / (dist_opp2p + 0.1);
	//2
	dist_opp2p = (p - C_theirPlayer_Geo[1]).mod();
	ball2opp = C_theirPlayer_Geo[1] - C_ball_Geo;
	dist_ball2opp = ball2opp.mod();
	dir_ball2opp = ball2opp.dir();			
	absDirDiff = fabs(CNormalize(dir_ball2opp - dir_ball2p));
	if (absDirDiff < minAngleDiff && dist_ball2opp < dist_ball2p) {
		minAngleDiff = absDirDiff;
	}
	reverseReceiveP += 50.0 / (dist_opp2p + 0.1);
	//3
	dist_opp2p = (p - C_theirPlayer_Geo[2]).mod();
	ball2opp = C_theirPlayer_Geo[2] - C_ball_Geo;
	dist_ball2opp = ball2opp.mod();
	dir_ball2opp = ball2opp.dir();			
	absDirDiff = fabs(CNormalize(dir_ball2opp - dir_ball2p));
	if (absDirDiff < minAngleDiff && dist_ball2opp < dist_ball2p) {
		minAngleDiff = absDirDiff;
	}
	reverseReceiveP += 50.0 / (dist_opp2p + 0.1);
	//4
	dist_opp2p = (p - C_theirPlayer_Geo[3]).mod();
	ball2opp = C_theirPlayer_Geo[3] - C_ball_Geo;
	dist_ball2opp = ball2opp.mod();
	dir_ball2opp = ball2opp.dir();			
	absDirDiff = fabs(CNormalize(dir_ball2opp - dir_ball2p));
	if (absDirDiff < minAngleDiff && dist_ball2opp < dist_ball2p) {
		minAngleDiff = absDirDiff;
	}
	reverseReceiveP += 50.0 / (dist_opp2p + 0.1);
	//5
	dist_opp2p = (p - C_theirPlayer_Geo[4]).mod();
	ball2opp = C_theirPlayer_Geo[4] - C_ball_Geo;
	dist_ball2opp = ball2opp.mod();
	dir_ball2opp = ball2opp.dir();			
	absDirDiff = fabs(CNormalize(dir_ball2opp - dir_ball2p));
	if (absDirDiff < minAngleDiff && dist_ball2opp < dist_ball2p) {
		minAngleDiff = absDirDiff;
	}
	reverseReceiveP += 50.0 / (dist_opp2p + 0.1);
	//6
	dist_opp2p = (p - C_theirPlayer_Geo[5]).mod();
	ball2opp = C_theirPlayer_Geo[5] - C_ball_Geo;
	dist_ball2opp = ball2opp.mod();
	dir_ball2opp = ball2opp.dir();			
	absDirDiff = fabs(CNormalize(dir_ball2opp - dir_ball2p));
	if (absDirDiff < minAngleDiff && dist_ball2opp < dist_ball2p) {
		minAngleDiff = absDirDiff;
	}
	reverseReceiveP += 50.0 / (dist_opp2p + 0.1);
	//��Ҫ�赲�Լ�������
	if (debug) {
		std::cout << "receive is " << reverseReceiveP << std::endl;
	}
	CGeoLine temp_line(C_ball_Geo, CGeoPoint(300, 0));
	float blockMyself = p.dist(temp_line.projection(p));
	if (blockMyself <= 15) {
		reverseReceiveP += C_POINT_PASS_OFF;
	} 
	else reverseReceiveP += 20.0 / (blockMyself + 0.1);
	if (debug) {
		std::cout << "block self angle is " << 20.0 / (blockMyself + 0.1) << std::endl;
	}
	reverseReceiveP += 3.0/(minAngleDiff + 0.01); // ������赲��,����0.001Ϊ��֤������Ϊ0
	if (debug) {
		std::cout << "block angle is " << 3.0/(minAngleDiff + 0.01) << std::endl;
	}
}
/************************************************************************/
/*���ŽǶȼ���                                     */
/************************************************************************/
//C_shootRange����ֵ�ṹ
typedef struct{
	float biggestAngle;
	float bestDir;
} C_shootParam;
//�赲����ṹ�壬���ڱ�������
typedef struct{
	float leftDir;
	float rightDir;
	float leftDist;
	float rightDist;
} C_blockDirection;
inline float C_GetDirRange(const C_blockDirection& D){return (D.leftDist <= D.rightDist ? D.leftDist : D.rightDist)*fabs(D.rightDir - D.leftDir);} //�˴���ʱ����fabs()��������û�����ȥ��
inline float C_GetDirMiddle(const C_blockDirection& D){return (D.leftDir + D.rightDir)/2.0;}
//���ŽǶȼ��㺯��
C_shootParam C_shootRange_Geo(const CGeoPoint p){
	C_shootParam sParam;	
	CGeoPoint blockOpp[6];
	int blockOppNum = 0; //��C_theirPlayer�Ĺ�ϵ��i-1
	//p��������������ز���
	const CGeoPoint goalPoint(300, 0);
	const CGeoPoint goalLeftPoint(300, -35);
	const CGeoPoint goalRightPoint(300, 35);
	const CGeoLine goalLeftLine(p, goalLeftPoint);
	const CGeoLine goalRightLine(p, goalRightPoint);
	const float leftAngle = (goalLeftPoint - p).dir();
	const float rightAngle = (goalRightPoint - p).dir();
	//�������赲��Χ��С��λ��д������blockOpp[]
	//1
	CGeoPoint opp = C_theirPlayer_Geo[0];
	if(p.x() - 10 < opp.x() && dirDiff((opp - p), (goalPoint - p)) < PI*2/3.0 && dirDiff((p - goalPoint), (opp - goalPoint)) < PI*2/3.0) {
		blockOpp[blockOppNum] = opp;
		blockOppNum++;
	}
	//2
	opp = C_theirPlayer_Geo[1];
	if(p.x() - 10 < opp.x() && dirDiff((opp - p), (goalPoint - p)) < PI*2/3.0 && dirDiff((p - goalPoint), (opp - goalPoint)) < PI*2/3.0) {
		blockOpp[blockOppNum] = opp;
		blockOppNum++;
	}
	//3
	opp = C_theirPlayer_Geo[2];
	if(p.x() - 10 < opp.x() && dirDiff((opp - p), (goalPoint - p)) < PI*2/3.0 && dirDiff((p - goalPoint), (opp - goalPoint)) < PI*2/3.0) {
		blockOpp[blockOppNum] = opp;
		blockOppNum++;
	}
	//4
	opp = C_theirPlayer_Geo[3];
	if(p.x() - 10 < opp.x() && dirDiff((opp - p), (goalPoint - p)) < PI*2/3.0 && dirDiff((p - goalPoint), (opp - goalPoint)) < PI*2/3.0) {
		blockOpp[blockOppNum] = opp;
		blockOppNum++;
	}
	//5
	opp = C_theirPlayer_Geo[4];
	if(p.x() - 10 < opp.x() && dirDiff((opp - p), (goalPoint - p)) < PI*2/3.0 && dirDiff((p - goalPoint), (opp - goalPoint)) < PI*2/3.0) {
		blockOpp[blockOppNum] = opp;
		blockOppNum++;
	}
	//6
	opp = C_theirPlayer_Geo[5];
	if(p.x() - 10 < opp.x() && dirDiff((opp - p), (goalPoint - p)) < PI*2/3.0 && dirDiff((p - goalPoint), (opp - goalPoint)) < PI*2/3.0) {
		blockOpp[blockOppNum] = opp;
		blockOppNum++;
	}
	//��Ч��Ե��������
	float activeEdge[14];
	float activeDist[14];
	int activeEdgeCount = 0;
	//����Ч��Ե�߼��뵽��Ч��Ե��������
	//�����������
	int leftCount = blockOppNum;
	for (;leftCount > 0;leftCount--) { //ע��i��ӦblockOpp[i-1]
		if (pointToLineDist(blockOpp[leftCount - 1], goalLeftLine) < 9) {
			break;
		}
	}
	if (0 == leftCount) {
		activeEdge[activeEdgeCount] = leftAngle;
		activeDist[activeEdgeCount] = p.dist(goalLeftPoint);
		activeEdgeCount++;
	}
	//�������ұ���
	int rightCount = blockOppNum;
	for (;rightCount > 0;rightCount--) {
		if (pointToLineDist(blockOpp[rightCount - 1], goalRightLine) < 9) {
			break;
		}
	}
	if (0 == rightCount) {
		activeEdge[activeEdgeCount] = rightAngle;
		activeDist[activeEdgeCount] = p.dist(goalRightPoint);
		activeEdgeCount++;
	}
	int useBlockOppNum = blockOppNum;
	for (;useBlockOppNum > 0;useBlockOppNum--) { //��useBlockOppNum��������Ե���ж�
		CVector p2opp = blockOpp[useBlockOppNum-1] - p;
		float maxSide = p2opp.mod();
		float derta = asinf(9.0/maxSide); 
		//���Ե
		float oppLeftBlockAngle = p2opp.dir() - derta;
		CGeoLine oppLeftBlockLine(p, oppLeftBlockAngle);
		int oppLeftCount = blockOppNum;
		for (;oppLeftCount > 0;oppLeftCount--) {
			if (oppLeftCount == useBlockOppNum) {
				continue;
			}
			if (pointToLineDist(blockOpp[oppLeftCount - 1], oppLeftBlockLine) < 9) {
				break;
			}
		}
		if (0 == oppLeftCount && true == CBetween(oppLeftBlockAngle,leftAngle,rightAngle)) {
			activeEdge[activeEdgeCount] = oppLeftBlockAngle;
			activeDist[activeEdgeCount] = p2opp.mod();
			activeEdgeCount++;
		}
		//�ұ�Ե
		float oppRightBlockAngle = p2opp.dir() + derta; 
		CGeoLine oppRightBlockLine(p,oppRightBlockAngle);
		int oppRightCount = blockOppNum;
		for (;oppRightCount > 0;oppRightCount--) {
			if (oppRightCount == useBlockOppNum) {
				continue;
			}
			if (pointToLineDist(blockOpp[oppRightCount-1], oppRightBlockLine) < 9) {
				break;
			}
		}
		if (0 == oppRightCount && true == CBetween(oppRightBlockAngle,leftAngle,rightAngle)) {
			activeEdge[activeEdgeCount] = oppRightBlockAngle;
			activeDist[activeEdgeCount] = p2opp.mod();
			activeEdgeCount++;
		}
	}
	//���û�����ŽǶ�,ֱ�ӷ���
	if (0 == activeEdgeCount) {
		sParam.biggestAngle = 0;
		sParam.bestDir = 0;
		return sParam;
	}
	//��Ч��Ե����ð������
	float temp;
	for(int i = activeEdgeCount - 1;i > 0;i--) {
		for(int j = 0;j < i;j++) {
			if(activeEdge[j] > activeEdge[j+1]) {
				temp = activeEdge[j];
				activeEdge[j] = activeEdge[j+1];
				activeEdge[j+1] = temp;
				temp = activeDist[j];
				activeDist[j] = activeDist[j+1];
				activeDist[j+1] = temp;
			}
		}
	}
	//�����赲����ṹ��
	C_blockDirection blockDir[7];
	int blockDirCount = activeEdgeCount/2;
	for (int i = 0;i < blockDirCount;i++) {
		blockDir[i].leftDir = activeEdge[i*2];
		blockDir[i].leftDist = activeDist[i*2];
		blockDir[i].rightDir = activeEdge[i*2+1];
		blockDir[i].rightDist = activeDist[i*2+1];
	}
	//ð���������ŽǶ�,�������ŵ���ǰ
	C_blockDirection temp2;
	for(int i = blockDirCount - 1;i > 0;i--) {
		if( C_GetDirRange(blockDir[i]) > C_GetDirRange(blockDir[i-1])) {
			temp2 = blockDir[i];
			blockDir[i] = blockDir[i-1];
			blockDir[i-1] = temp2;
		}
	}
	//ȡ�������ŽǶȼ�������
	sParam.biggestAngle = C_GetDirRange(blockDir[0]);
	sParam.bestDir = C_GetDirMiddle(blockDir[0]);
	if (debug) {
		std::cout << "the biggestAngle is" << sParam.biggestAngle << std::endl;
		CVector m(1000, blockDir[0].leftDir);
		GDebugEngine::Instance()->gui_debug_line(CGeoPoint(p.x(),p.y()),CGeoPoint(p.x(),p.y()) + CVector(m.x(),m.y()),COLOR_RED);
		m = CVector(1000, blockDir[0].rightDir);
		GDebugEngine::Instance()->gui_debug_line(CGeoPoint(p.x(),p.y()),CGeoPoint(p.x(),p.y()) + CVector(m.x(),m.y()),COLOR_RED);
	}
	return sParam;                                 
}
