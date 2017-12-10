#ifndef _REFEREE_BOX_IF_H_
#define _REFEREE_BOX_IF_H_
#include "Network\UDPSocket.h"
#include <MultiThread.h>
#include <singleton.h>
/*
 *	CRefereeBoxInterface.
 *  和RefereeBox的接口
 */
class CSerialPort;
class CRefereeBoxInterface{
public:
	CRefereeBoxInterface();
	~CRefereeBoxInterface();
	void start();
	void stop();
	inline int getPlayMode(){return _playMode;}
	inline int getBlueGoalNum(){return _blueGoalNum;}
	inline int getYellowGoalNum(){return _yellowGoalNum;}
	inline int getRemainTime(){return _remainTime;}
	inline int getBlueGoalie(){return _blueGoalie;}
	inline int getYellowGoalie(){return _yellowGoalie;}
protected:
	static CThreadCreator::CallBackReturnType THREAD_CALLBACK listener(CThreadCreator::CallBackParamType refBoxInt);
	void receivingLoop();
private:
	UDPSocket* _UDPPort;
	bool _alive;
	// 现在的裁判盒指令
	int _playMode;
	// 蓝方的进球数
	int _blueGoalNum;
	// 黄方的进球数
	int _yellowGoalNum;
	// 这个半场所剩的时间(sec)
	int _remainTime;
	// 蓝方守门员车号
	int _blueGoalie;
	// 黄方守门员车号
	int _yellowGoalie;
};
typedef NormalSingleton<CRefereeBoxInterface> RefereeBoxInterface;
#endif