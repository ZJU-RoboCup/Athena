#ifndef _COMPENSATE_
#define _COMPENSATE_

//用于计算一传一射时的角度补偿
#include <geometry.h>
#include "utils.h"
#include "singleton.h"
//非线性优化库
#include "nlopt/nlopt.hpp"

#define COMPENSATE_FILE_NAME "data_Iran2017.txt"
#define COMPENSATE_RATION 1	

#define SHOOT_SPEED 450

#define INIT_CMP_PARAM 0	// 初始化参数时设为1
#define BAYESINIT 0			// 用贝叶斯方法时设为1
#define RANSACINIT 0		// 用RANSAC时设为1
#define ISTESTRANSAC 0		// test模式设为1（读入txt中的数据）


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
	double getKickDir(int playerNum, CGeoPoint kickTarget); //  只输入kickTarget，从vision中获取ball的各种信息就行
	void setOurRobotIndex(unsigned char*_ourRobotIndex);

	void getData(double shoot_vel, double shoot_dir, double out_dir);	// Total Interface: get data from lua and init/adjust
	void initParam(int num);											// initialize parameters
	void checkModel(int num);											// interface: check if parameters change

	// 一些功能函数
	// nlopt func:
	void calcParamRANSAC();
	double calcObjFunc(const double *x);
	void calcInitParam();
	bool initConverge(); // 判断初始化是否收敛

	// 外层闭环，根据场上射门偏差改变参数值
	double adjustCmpParam(double v_out_dir, double player2pos, double shoot_vel, double shoot_dir);// 外层闭环，类似惯性环节。 A' = sin(compensate + error / 8) * v_out / (v_in * sin(theta_in));  // 8为自己设定的值

private:
	void readCompensateTable(); // 不需要读表，其实是可以通过拟合直接算的。（加一个速度
	double compensatevalue[100][50];
	unsigned char ourRobotIndex[6];

	// 模型参数和数据
	// 参数 theta_out = A * v_in * sin(theta_in) + B
	double A; 
	double B;  // 临时储存的参数
	double A0;
	double B0;	// real params
	double sigmaInit;	// 初始化时的数据残差平方
	double sigmaCmp;    // 场上测量时数据的残差平方
	
	// 数据
	double compensateData[50][3];	 // 检查模型的时候的所有数据
	double initCmpData[100][3];		// 初始化时储存的所有数据
	bool isInterPoint[100];
	int cmpSampleNum;					// 检查时采集的数据个数
	int initSampleNum;				// 初始化时采集的数据个数
	double x_init[2];				// 结果初始值
	double lastA[2];
	double lastB[2];
	double adjustingData[50][2];    // 测补偿用的数据， 第一列是v_out_dir, 第二列是 player2pos
	int adjustNum;
	double adjustAngle;

};


typedef NormalSingleton< CCompensate > Compensate;
typedef NormalSingleton< CCompensateNew > CompensateNew;

#endif 