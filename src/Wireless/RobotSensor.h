#ifndef _ROBOT_FILTERED_INFO_H_
#define _ROBOT_FILTERED_INFO_H_

#include <CommControl.h>
#include <singleton.h>
#include <param.h>

/*
* С�������֪��Ϣ���մ��� created by qxz
*/

/// Ԥ�����һЩ������
#define MAX_STORE_SIZE 5  // �������5֡����Ϣ
#define NO_KICK 0
#define IS_FLAT_KICK_ON 1
#define IS_CHIP_KICK_ON 2
#define LOWEST_ENERGY 0
#define LOW_ENERGY 1
#define NORMAL_ENERGY 2
#define HIGH_ENERGY 3
#define HIGHEST_ENERGY 4

/// �����ش�С��������Ϣ�Ľṹ
struct stLastResult{
	// ���²���ҪС��״̬����ʱ�Ľṹ��Ϣ
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

/// �����˱����֪��Ϣ
class CRobotSensor{
public:
	// ���캯��
	CRobotSensor();
	// ��������
	~CRobotSensor() {}

	// ȫ�ָ��½��
	void Update(int cycle);

	// С��˫��ͨѶ�Ƿ��Ѿ������������Ƿ������µ�
	bool IsInfoValid(int num)				{ return _isValid[num]; }

	// С���Ƿ���������źţ�һ���ʾ������ǰ��
	bool IsInfraredOn(int num);
	// ����С���ĺ����ź�
	void ResetInraredOn(int num)			{ _lastInfraredInfo[num] = false;  robotInfoBuffer[num].bInfraredInfo = false; }

	// С���з�����ƽ�������Ļ���
	int IsKickerOn(int num)					{ return robotInfoBuffer[num].nKickInfo; }
	
	// С���Ƿ��������Ҫ�޸�
	bool isBallControled(int num)			{ return robotInfoBuffer[num].bControledInfo; }
	// ����С���Ŀ����ź�
	void ResetBallControled(int num)		{ _lastBallCtrlInfo[num] = false;  robotInfoBuffer[num].bControledInfo = false; }

	int GetChangeNum(int num)				{ return robotInfoBuffer[num].changeNum; }
	int GetChangeCountNum(int num)			{ return robotInfoBuffer[num].changeCountNum; }

	void setKickingChecked(int num, int cycle);

	//��������Ϣ
	const char* getstopoutputmsg(int num) {return  outputmsg[num];}

private:
	// ���µ��ڲ��ӿ�
	void UpdateBallDetected(int num);
	void UpdateKickerInfo(int num, int cycle);

	ROBOTINFO robotInfoBuffer[Param::Field::MAX_PLAYER+1];		// ��Ź��˺�Ľ��
	ROBOTINFO rawDataBuffer;

	bool _lastKickingChecked[Param::Field::MAX_PLAYER+1];
	int _lastCheckedKickingCycle[Param::Field::MAX_PLAYER+1];

	int _updateCycle[Param::Field::MAX_PLAYER+1];				// ���һ�θ���ʱ�����Ե�����ֵ
	int _timeIndex[Param::Field::MAX_PLAYER+1];					// ��������ʱ���ʾ
	int _valid_count[Param::Field::MAX_PLAYER+1];                             
	bool _isValid[Param::Field::MAX_PLAYER+1];					// ��ǰ�����Ƿ������µ�

	bool _lastInfraredInfo[Param::Field::MAX_PLAYER+1];			// ��ʷ�ĺ�������
	bool _lastBallCtrlInfo[Param::Field::MAX_PLAYER+1];			// ��ʷ�ĺ�������

	char *outputmsg[Param::Field::MAX_PLAYER+1];

	robotMsg _robotMsg[Param::Field::MAX_PLAYER+1];
};

typedef NormalSingleton< CRobotSensor > RobotSensor;			// ȫ�ַ��ʽӿ�
#endif