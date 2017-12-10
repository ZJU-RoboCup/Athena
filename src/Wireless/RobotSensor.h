#ifndef _ROBOT_FILTERED_INFO_H_
#define _ROBOT_FILTERED_INFO_H_

#include <CommControl.h>
#include <singleton.h>
#include <param.h>

/*
* 小车本体感知信息接收处理 created by qxz
*/

/// 预定义的一些常量宏
#define MAX_STORE_SIZE 5  // 保留最近5帧的信息
#define NO_KICK 0
#define IS_FLAT_KICK_ON 1
#define IS_CHIP_KICK_ON 2
#define LOWEST_ENERGY 0
#define LOW_ENERGY 1
#define NORMAL_ENERGY 2
#define HIGH_ENERGY 3
#define HIGHEST_ENERGY 4

/// 包裹回传小车本体信息的结构
struct stLastResult{
	// 向下层索要小车状态数据时的结构信息
	ROBOTINFO info;
	int cycle;
	friend bool operator<(const stLastResult& a, const stLastResult& b){ return a.cycle<b.cycle; }
	friend bool operator==(const stLastResult& a, const stLastResult& b){ return a.cycle == b.cycle; }
	friend bool operator>(const stLastResult& a, const stLastResult& b){ return a.cycle>b.cycle; } 
};

struct robotMsg{
	int robotNum;
	int capacity;
	int battery;
	bool infare;
};

/// 机器人本体感知信息
class CRobotSensor{
public:
	// 构造函数
	CRobotSensor();
	// 析构函数
	~CRobotSensor() {}

	// 全局更新借口
	void Update(int cycle);

	// 小车双向通讯是否已经启动，数据是否是最新的
	bool IsInfoValid(int num)				{ return _isValid[num]; }

	// 小车是否产生红外信号，一般表示球在嘴前方
	bool IsInfraredOn(int num);
	// 重置小车的红外信号
	void ResetInraredOn(int num)			{ _lastInfraredInfo[num] = false;  robotInfoBuffer[num].bInfraredInfo = false; }

	// 小车有否启动平射或挑射的机构
	int IsKickerOn(int num)					{ return robotInfoBuffer[num].nKickInfo; }
	
	// 小车是否控制着球，要修改
	bool isBallControled(int num)			{ return robotInfoBuffer[num].bControledInfo; }
	// 重置小车的控球信号
	void ResetBallControled(int num)		{ _lastBallCtrlInfo[num] = false;  robotInfoBuffer[num].bControledInfo = false; }

	int GetChangeNum(int num)				{ return robotInfoBuffer[num].changeNum; }
	int GetChangeCountNum(int num)			{ return robotInfoBuffer[num].changeCountNum; }

	void setKickingChecked(int num, int cycle);

	//获得输出信息
	const char* getstopoutputmsg(int num) {return  outputmsg[num];}

private:
	// 更新的内部接口
	void UpdateBallDetected(int num);
	void UpdateKickerInfo(int num, int cycle);

	ROBOTINFO robotInfoBuffer[Param::Field::MAX_PLAYER+1];		// 存放过滤后的结果
	ROBOTINFO rawDataBuffer;

	bool _lastKickingChecked[Param::Field::MAX_PLAYER+1];
	int _lastCheckedKickingCycle[Param::Field::MAX_PLAYER+1];

	int _updateCycle[Param::Field::MAX_PLAYER+1];				// 最近一次更新时，策略的周期值
	int _timeIndex[Param::Field::MAX_PLAYER+1];					// 返回数据时间标示
	int _valid_count[Param::Field::MAX_PLAYER+1];                             
	bool _isValid[Param::Field::MAX_PLAYER+1];					// 当前数据是否是最新的

	bool _lastInfraredInfo[Param::Field::MAX_PLAYER+1];			// 历史的红外数据
	bool _lastBallCtrlInfo[Param::Field::MAX_PLAYER+1];			// 历史的红外数据

	char *outputmsg[Param::Field::MAX_PLAYER+1];

	robotMsg _robotMsg[Param::Field::MAX_PLAYER+1];
};

typedef NormalSingleton< CRobotSensor > RobotSensor;			// 全局访问接口
#endif