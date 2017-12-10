#ifndef _COMPENSATE_
#define _COMPENSATE_

//���ڼ���һ��һ��ʱ�ĽǶȲ���
#include <geometry.h>
#include "utils.h"
#include "singleton.h"
//�������Ż���
#include "nlopt/nlopt.hpp"

#define COMPENSATE_FILE_NAME "data_Iran2017.txt"
#define COMPENSATE_RATION 1	

#define SHOOT_SPEED 450

#define INIT_CMP_PARAM 0	// ��ʼ������ʱ��Ϊ1
#define BAYESINIT 0			// �ñ�Ҷ˹����ʱ��Ϊ1
#define RANSACINIT 0		// ��RANSACʱ��Ϊ1
#define ISTESTRANSAC 0		// testģʽ��Ϊ1������txt�е����ݣ�


class CCompensate{
public:
	CCompensate(); 
	~CCompensate();
	double checkCompensate(int playerNum,double ballspeed,double rawdir);
	double getKickDir(int playerNum, CGeoPoint kickTarget);
	void setOurRobotIndex(unsigned char*_ourRobotIndex);
private:
	void readCompensateTable();
	double compensatevalue[100][50];
	unsigned char ourRobotIndex[6];
};

// new compensate

class CCompensateNew{
public:
	CCompensateNew(); 
	~CCompensateNew();
	double checkCompensate(int playerNum,double ballspeed,double rawdir, double outdir,  bool showFlag = 0);
	double getKickDir(int playerNum, CGeoPoint kickTarget); //  ֻ����kickTarget����vision�л�ȡball�ĸ�����Ϣ����
	void setOurRobotIndex(unsigned char*_ourRobotIndex);

	void getData(double shoot_vel, double shoot_dir, double out_dir);	// Total Interface: get data from lua and init/adjust
	void initParam(int num);											// initialize parameters
	void checkModel(int num);											// interface: check if parameters change

	// һЩ���ܺ���
	// nlopt func:
	void calcParamRANSAC();
	double calcObjFunc(const double *x);
	void calcInitParam();
	bool initConverge(); // �жϳ�ʼ���Ƿ�����

	// ���ջ������ݳ�������ƫ��ı����ֵ
	double adjustCmpParam(double v_out_dir, double player2pos, double shoot_vel, double shoot_dir);// ���ջ������ƹ��Ի��ڡ� A' = sin(compensate + error / 8) * v_out / (v_in * sin(theta_in));  // 8Ϊ�Լ��趨��ֵ

private:
	void readCompensateTable(); // ����Ҫ������ʵ�ǿ���ͨ�����ֱ����ġ�����һ���ٶ�
	double compensatevalue[100][50];
	unsigned char ourRobotIndex[6];

	// ģ�Ͳ���������
	// ���� theta_out = A * v_in * sin(theta_in) + B
	double A; 
	double B;  // ��ʱ����Ĳ���
	double A0;
	double B0;	// real params
	double sigmaInit;	// ��ʼ��ʱ�����ݲв�ƽ��
	double sigmaCmp;    // ���ϲ���ʱ���ݵĲв�ƽ��
	
	// ����
	double compensateData[50][3];	 // ���ģ�͵�ʱ�����������
	double initCmpData[100][3];		// ��ʼ��ʱ�������������
	bool isInterPoint[100];
	int cmpSampleNum;					// ���ʱ�ɼ������ݸ���
	int initSampleNum;				// ��ʼ��ʱ�ɼ������ݸ���
	double x_init[2];				// �����ʼֵ
	double lastA[2];
	double lastB[2];
	double adjustingData[50][2];    // �ⲹ���õ����ݣ� ��һ����v_out_dir, �ڶ����� player2pos
	int adjustNum;
	double adjustAngle;

};


typedef NormalSingleton< CCompensate > Compensate;
typedef NormalSingleton< CCompensateNew > CompensateNew;

#endif 