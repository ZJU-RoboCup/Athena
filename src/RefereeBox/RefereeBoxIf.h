#ifndef _REFEREE_BOX_IF_H_
#define _REFEREE_BOX_IF_H_
#include "Network\UDPSocket.h"
#include <MultiThread.h>
#include <singleton.h>
/*
 *	CRefereeBoxInterface.
 *  ��RefereeBox�Ľӿ�
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
	// ���ڵĲ��к�ָ��
	int _playMode;
	// �����Ľ�����
	int _blueGoalNum;
	// �Ʒ��Ľ�����
	int _yellowGoalNum;
	// ����볡��ʣ��ʱ��(sec)
	int _remainTime;
	// ��������Ա����
	int _blueGoalie;
	// �Ʒ�����Ա����
	int _yellowGoalie;
};
typedef NormalSingleton<CRefereeBoxInterface> RefereeBoxInterface;
#endif