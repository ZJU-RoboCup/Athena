/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team��		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  gpuBestAlgThread.h										*/
/* Func:	  ���������λ����̣߳�֧��CPUģʽ��GPUģʽ				    */
/* Author:	  ��Ⱥ 2012-08-18											*/
/*            Ҷ���� 2014-03-01                                         */
/* Refer:	  ###														*/
/* E-mail:	  wangqun1234@zju.edu.cn									*/
/* Version:	  0.0.1														*/
/************************************************************************/

#ifndef _GPU_BEST_ALG_THREAD_H
#define _GPU_BEST_ALG_THREAD_H

#include <fstream>
#include <MultiThread.h>
#include <singleton.h>
#include "VisionModule.h"
#include "geometry.h"

#define GRIDX 910
#define GRIDY 610
#define  C_POINT_PASS_OFF 200
#define  C_MAX_PLAYER		6

enum ReCalModeT {
	CIRCLE_MODE = 0,
	RECT_MODE,
};
/**
@brief    ���������λ����̣߳�֧��CPUģʽ��GPUģʽ
@details  �����Ƕ��������̵߳��㷨�̣߳��ṩȫ��������������λ��ļ��㡣ȫ�ֱ���USE_GPU
Ϊ1ʱʹ��GPU������λ���㣬Ϊ0ʱʹ��CPU������λ���㡣Ϊ����ÿ���������λ������ܹ������ȶ���
���һ���̶��Ļ��߱䶯��С��������Ҫ��ȡ��ʱע��һ���̶��������š�
@note	ע�⣬��ÿһ֡���У���ʹ�ö���ӿ�getBestPoint����ǰ��Ҫ����һ��setSendPoint�������趨
����ĵ㡣һ��˴�Ϊ��ǰ���ڵĵ�*/

//C_shootRange����ֵ�ṹ
typedef struct {
	float biggestAngle;
	float bestDir;
} C_shootParam;
//�赲����ṹ�壬���ڱ�������
typedef struct {
	float leftDir;
	float rightDir;
	float leftDist;
	float rightDist;
} C_blockDirection;

class CGPUBestAlgThread {
public:
	CGPUBestAlgThread();
	~CGPUBestAlgThread();

public:
	/**
	@brief	ע���Ӿ���Ϣ���ҿ����㷨�߳�*/
	void initialize(CVisionModule* _pVision);
	/**
	@brief	��ģ��Ķ���ӿ�
	@param	leftUp ȡ����������ϵ�(x�������ҷ����ֱ������ϵ��)
	@param	rightDown ȡ����������µ�(x�������ҷ����ֱ������ϵ��)
	@param	pointKeeperNum ȡ�㱣��ϵͳ�ı�ţ���ǰϵͳ��ʶ��������0~9
	@param	reCalMode ȡ������Ϊ���λ���Բ�Ρ���ǰϵͳֻ֧�־��Σ����˴�����RECT_MODE
	@param	forceReset ÿ�����¼��������λ�㣬Ĭ��Ϊfalse������true������ʹ��ȡ�㱣��ϵͳ*/
	CGeoPoint getBestPoint(const CGeoPoint leftUp = CGeoPoint(300, -200), const CGeoPoint rightDown = CGeoPoint(0, 200), const int pointKeeperNum = 0, const ReCalModeT& reCalMode = RECT_MODE, bool forceReset = false);//pointKeeper֧��ͬʱ�������ѵ�ʱ����ѵ㱣�ֹ���

																																																						 /**
																																																						 @brief ȫ�����е������ֵ������*/
	void generatePointValue();
	// ��� xx ��
	// ...

	/**
	@brief	�ж�ĳһ����ǰһ֡�������λ���Ƿ�����Ч*/
	bool isLastOneValid(const CGeoPoint& p);
	/**
	@brief	�趨�����
	@param	passPoint ��ǰ�����*/
	void setSendPoint(const CGeoPoint passPoint);
	/**
	@brief	ȡ�������ֵ�Ķ���ӿ�
	@param	p Ҫ��ȡ����ֵ�ĵ�*/
	double getPosPotential(const CGeoPoint p);

	// ����ΪһЩ���ۺ��� yys
	void C_evaluateReceive_Geo(const CGeoPoint p, float& reverseReceiveP);

	C_shootParam C_shootRange_Geo(const CGeoPoint p);

	void C_evaluateShoot_Geo(const CGeoPoint p, float& reverseShootP);

	void C_evaluateOthers_Geo(const CGeoPoint p, float& reverseOtherP);

	float getPointPotential(int x, int y);

public:
	//���㶨λ��������ŵ� jyl
	CGeoPoint bestPointForCornerKicks(bool isChip);

	CGeoPoint bestPointForFrontKicks();

	CGeoPoint bestPointForBackKicks();

	//��������������ŵ����������
	int getChipPower() { return chipPower; }
	int getFixBuf() { return fixBuf; }

	//�Д��Ƿ�����
	bool isChip();
	bool isChip(const int robotnum);
	bool isChip(const CGeoPoint passPoint);
	ofstream out;
	//�°涯̬���� 2017.5.18 jyl
	//��̽ģ��
	void detectModule(bool generatePos, double detectDist, int detectCycle, bool useGoalie);
	void detectDefendKickedModule(int checkAttepmts, int defendKickCycle);
	//ɨ��ģ��
	void scanModule();
	//��ַ�����ģ��
	void fakePosComputeModule();
	//�ط������ģ��
	void defendPosComputeModule();
	//�ж��Ƿ���Կ���
	bool canPass();
	//���㵲���
	bool isDefendedHead();
	//��ȡ��λ��
	CGeoPoint getPosForFirstVehicle() { return runPos[0]; }
	CGeoPoint getPosForSecondVehicle() { return runPos[1]; }
	CGeoPoint getPosForThirdVehicle() { return runPos[2]; }
	CGeoPoint getPosForFourthVehicle() { return runPos[3]; }
	CGeoPoint getPosForAssister() { return assisterPos; }
	CGeoPoint getPosForGoalie() { return runPos[4]; }
	CGeoPoint getPosForShoot() { return shootPos; }
	CGeoPoint generateJamPos();

	CGeoPoint confirmPosForFirstVehicle() { return confirmPos[0]; }
	CGeoPoint confirmPosForSecondVehicle() { return confirmPos[1]; }
	CGeoPoint confirmPosForThirdVehicle() { return confirmPos[2]; }
	CGeoPoint confirmPosForFourthVehicle() { return confirmPos[3]; }
	//
	bool detectMarkedOver() { return detectMarkedOverFlag; }
	bool detectDefendKickedOver() { return detectDefendKickedOverFlag; }
	bool detectAgain() { return detectAgainFlag; }
	bool scanOver() { return scanOverFlag; }
	bool allMarked() { return allMarkedFlag; }
	bool isMarked(int number);
	bool changeLeader(){ return changeLeaderFlag; }
	//
	bool isMarkedForDynamicKick(int num, double determindist);
	bool isDefendKickedForDynamicKick(int number);
	//
	void numberReceive(int first, int second, int third, int fourth, int assister, int goalie);
	void receiveOwnPos(vector <CGeoPoint> myPos  , int number);
	void changeLeaderOver(bool finishFlag);
	vector <CGeoPoint> generateShootPos();
	enum {
		CornerKick = 1,
		FrontKick,
		MiddleKick
	};
	enum {
		Free,
		ShotBlocked,
		PassBlocked
	};
	int number[6];
	CGeoPoint runPos[5];
	CGeoPoint confirmPos[5];
	CGeoPoint shootPos;
	const CGeoRectangle* getRunArea();
	CGeoPoint generateRunPos(const int areaNumber);
	bool changeLeaderOverFlag;
	bool changeLeaderFlag;
private:
	int chipPower;
	double fixTime;
	int fixBuf;
	double determinValueForFlatKick; //�Q���Ƿ���洫��Ĳο�ֵ
	double determinValueForChipKick;
	//�°涯̬����
	CGeoPoint assisterPos;
	CGeoPoint theirDefendKickerPos;
	int getBallNumber;
	int ramdomNumber[5];
	int buf[6];
	int maxDefendKickbuf;
	int maxbuf;
	int state[6];
	int attempts;
	int detectDefendKickAttempts;
	int indirectKickType;
	int defendedTpye[4];
	int maxAttempts;
	int determinCycle;
	int pickNumber;
	vector <bool> flatPassAngleFlag;
	vector <double> flatPassAngle;
	vector <CGeoPoint> luaPos;
	double deltaAngle;
	int _cycle1;
	int _cycle2;
	bool detectOverFlag;
	bool detectDefendKickedOverFlag;
	bool detectMarkedOverFlag;
	bool allMarkedFlag;
	bool detectAgainFlag;
	bool scanOverFlag;
	bool fakePosComputeOverFlag;
	bool openArea[11];
	bool isChipForDynamicKick;
	bool stay;
	bool jamMode;
	bool pickMode;
	bool writeOver;
	bool useOwnPos;
public:
	/**
	@brief	�����߳����õĺ���*/
	static CThreadCreator::CallBackReturnType THREAD_CALLBACK doBestCalculation(CThreadCreator::CallBackParamType lpParam);
private:
	float(*_PointPotential)[GRIDY];	///<����ֵ����
	CVisionModule* _pVision;			///<ͼ��ָ��
	CGeoPoint pointKeeper[10];			///<ȡ�㱣������
	int _lastCycle[10];					///<ȡ�㱣��ϵͳ�õ���ǰһ֡֡��

	CGeoPoint sendPoint;				///<����㣬һ��Ϊ�����ڵĵ�

	int halfLength; // ���ذ볤
	int halfWidth;  // ���ذ��
	int halfGoalWidth;
};

typedef NormalSingleton< CGPUBestAlgThread > GPUBestAlgThread;

struct {
	double ball_PosX;
	double ball_PosY;
	double ourPosX[6];
	double ourPosY[6];
	double theirPosX[6];
	double theirPosY[6];
	double projectPosX[31];
	double projectPosY[31];
	double bestPosX;
	double bestPosY;
	double bestPointNumber;
	double bestScore;
	double enemyAroundPointNum[31];
	double weight1;
	double value1[31];
	double enemyAroundMeNum[31];
	double weight2;
	double value2[31];
	double minDist[31];
	double weight3;
	double value3[31];
	double enemy2PointDist[31];
	double weight4;
	double value4[31];
	double enemy2MeDist[31];
	double weight5;
	double value5[31];
	double ShootDir[31];
	double weight6;
	double value6[31];
	double receiveShootDir[31];
	double weight7;
	double value7[31];
	double ball2PointDist[31];
	double weight8;
	double value8[31];
	double score[31];
	double fixvalue;
}fieldData;


#endif