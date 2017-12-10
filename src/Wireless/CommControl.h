/// FileName : 		CommControl.h
/// 				declaration file
/// Description :	It supports command send interface for ZJUNlict,
///	Keywords :		fitting, send, interface
/// Organization : 	ZJUNlict@Small Size League
/// Author : 		cliffyin
/// E-mail : 		cliffyin@zju.edu.cn
///					cliffyin007@gmail.com
/// Create Date : 	2011-07-25
/// Modified Date :	2011-07-25 
/// History :

#ifndef __COMM_CONTROL_H__
#define __COMM_CONTROL_H__

#include "SerialPort.h"
#include <MultiThread.h>
#include "./RobotCommand.h"
#include "TimeCounter.h"

#define NUM_ROBOT  12

/// <summary>	Robot control.  </summary>
///
/// <remarks>	ZjuNlict, 2011-7-26. </remarks>

class CRobotControl
{
public:
	CRobotControl(void)
	{ 
		nSpeed1	= 0; 
		nSpeed2	= 0; 
		nSpeed3	= 0; 
		nSpeed4	= 0; 
		nKick	= 0;
		nControlBall = 0; 
		fX = 0; 
		fY = 0; 
		posX = 0;
		posY = 0;
		posW = 0;
		fRotate	= 0;
		gyro = false;
		stop=false;
		angle = 0;
		radius = 0;
	}

	~CRobotControl(void){};

	// ����
	int nRobotNum;

	// ����ģʽ
	int workingMode;

	// send��Ϣ: ����ȥ���ĸ����ӵ��ٶ�
	// �ϲ�ָ������ٶȷֽ��ĸ���ֵ
	float fSpeed1,fSpeed2,fSpeed3,fSpeed4;
	// ���͵��ײ�ʱ����ת��������ֵ
	short nSpeed1,nSpeed2,nSpeed3,nSpeed4;

	// send��Ϣ:������ָ��x����,y�����ٶ�,�Լ�ת��
	float fX, fY, fRotate;

	// send��Ϣ��С����Ŀ��λ�úͷ���
	float posX, posY, posW;

	// send��Ϣ:��������
	unsigned int nKick;

	// send��Ϣ:�����
	char nControlBall;

	// send��Ϣ��������
	bool gyro;
	float angle;
	unsigned int radius;
	bool stop;

	// send: PID Param
	unsigned int prop[4];
	unsigned int intg[4];
	unsigned int diff[4];

	// receive��Ϣ: ���������ĸ������ٶ�
	// ˫��ͨѶ�յ���ֻ������������
	short nCurSpeed1,nCurSpeed2,nCurSpeed3,nCurSpeed4;           
	// �ϲ�ȡ�õĸ�������
	float fCurSpeed1,fCurSpeed2,fCurSpeed3,fCurSpeed4;

	// receive��Ϣ: �������ź�
	bool bInfraredInfo;
	
	// receive��Ϣ: �������
	bool bControledInfo;

	// receive��Ϣ���ϴ��ı����
	int changeNum;

	// receive��Ϣ���ϴ����θı��ѷ��˼�֡
	int changeCountNum;

	//receive��Ϣ����ص�ѹ 
	int battery;

	//receive��Ϣ�����ݵ��
	int capacity;

	//receive��Ϣ: x,y������ٶ�,���ٶȣ��Ƕ�
	float accX, accY, vW, angel;

	//receive��Ϣ: 1,2,3,4��ռ�ձ�
	float duty1, duty2, duty3, duty4;

	// receive��Ϣ: �����⣨Ԥ������δ�õ���
	short kickInfo; // 0-û����, 1-ƽ��, 2-����

	unsigned char kickId, kickCount;
	

	void fton()
	{
		nSpeed1=(int)fSpeed1;
		nSpeed2=(int)fSpeed2;
		nSpeed3=(int)fSpeed3;
		nSpeed4=(int)fSpeed4;
	};

	void ntof()
	{
		fCurSpeed1=(float)nCurSpeed1;
		fCurSpeed2=(float)nCurSpeed2;
		fCurSpeed3=(float)nCurSpeed3;
		fCurSpeed4=(float)nCurSpeed4;
	};
};

/// <summary>	Robotinfo.  </summary>
/// �����ϲ����²���ҪС��״̬����ʱ�Ľṹ��Ϣ
/// <remarks>	ZjuNlict, 2011-7-26. </remarks>

struct ROBOTINFO
{
	int nRobotNum;

	float fCurSpeed1;
	float fCurSpeed2;
	float fCurSpeed3;
	float fCurSpeed4;

	bool bInfraredInfo;
	bool bControledInfo;
	short nKickInfo;
	unsigned char kickId;
	unsigned char kickCount;
	int changeNum;
	int changeCountNum;
	int battery;
	int capacity;
};

/// <summary>	Communications control, abstract interface.  </summary>
///
/// <remarks>	Zju Nlict, 2011-7-26. </remarks>
class AbstractRadioPackets;

/// <summary>	Communications control, main module.  </summary>
///
/// <remarks>	ZjuNlict, 2011-7-26. </remarks>

class CCommControl
{
public:

	/// <summary>	Default constructor. </summary>
	///
	/// <remarks>	ZjuNlict, 2011-7-26. </remarks>

	CCommControl(void);

	/// <summary>	Finaliser. </summary>
	///
	/// <remarks>	ZjuNlict, 2011-7-26. </remarks>

	~CCommControl(void);

	/// <summary>	Initialize interface, creates multi-threads. </summary>
	///
	/// <remarks>	ZjuNlict, 2011-7-26. </remarks>
	void Init();

	/// <summary>	Destroys interface, close threads. </summary>
	///
	/// <remarks>	ZjuNlict, 2011-7-26. </remarks>

	void Destroy();

	/// <summary>	Request robot information. </summary>
	/// ����ֵ��ʾ˫��ͨѶ���ݵ�ʱ�����
	/// <remarks>	ZjuNlict, 2011-7-26. </remarks>
	///
	/// <param name="n">   	The specified vehilce num. </param>
	/// <param name="info">	[in,out] If non-null, the information. </param>
	///
	/// <returns>	. </returns>

	int RequestSpeedInfo(int n,ROBOTINFO *info); 

	/// <summary>	Creates new robot command. </summary>
	///
	/// <remarks>	ZjuNlict, 2011-7-26. </remarks>
	///
	/// <param name="n">	  	The specified vehilce num. </param>
	/// <param name="command">	The corresponding command. </param>

	void NewRobotCommand(int n, const ROBOTCOMMAND* command);

	// added by cliffyin [6/11/2012 ZJUNlict]
	void ClearRobotCmd(int robotNum);

	/// <summary> The serial port </summary>
	friend class CSerialPort;

	/// <summary>	Control thread, main loop thread. </summary>
	///
	/// <remarks>	ZjuNlict, 2011-7-26. </remarks>
	///
	/// <param name="p">	[in,out] If non-null, the. </param>
	///
	/// <returns>	. </returns>

	friend unsigned __stdcall ControlThread(void *p);
	friend unsigned __stdcall ReceiveThread(void *p);
	//friend CThreadCreator::CallBackReturnType THREAD_CALLBACK ControlThread(CThreadCreator::CallBackParamType p);	

private:
	void CloseAll();
	void Error();

	void EncodePacket(int pkgNum);
	void DecodePacket();
	int recv_packet_len() const { return _nRecvPacketLen; }

	bool bStart;                    // �Ƿ����ڿ�ʼ����
	bool bCrcRight;					// ˫��ͨѶ�����ڽ��ն�CRCУ�飬���ƽ������ݵ���Ч��

	HANDLE hThread;
	unsigned _threadID;

	HANDLE hEventControlTimer;      // ���ٶȵ�ָ����һ��ʱ�䴥�������ͳ�ȥ��event
	HANDLE hEventControlReceive;    // �յ�С�����ݵ�event
	HANDLE hEventControlCheck;
	HANDLE hEventRequestInfo;       // �ϲ����²���Ҫ�³����ݵ�event
	HANDLE hEventRequestWait;       
	HANDLE hEventNewCommand;        // �ȴ���ȡ�ϲ��ٶȿ����ָ�����ݣ�����ֵ��CCommControl�������Ӧ������ȥ��event��ע����hEventControlTimer������
	HANDLE hEventNewCommandWait;
	HANDLE hEventWatchDog;          // ���Ź��¼�(��ʱû�����ݸ�����λ)

	UINT uTimerID;                  // ����ָ���ʱ�䴥���ļ�ʱ��
	UINT watchDogID;

	BYTE TXBuff[24];                // ���ݷ��͵Ļ����ַ,���ڷ���x,y�ٶ�,ת��,����,���������
	BYTE RXBuff[24];                // ���ݽ��յĻ����ַ,�����յ�С����Ӧ����

    // �²�->�ϲ�
	ROBOTINFO *pTempInfo;           // �ϲ����²���ҪС��״̬���ݣ����²㽫���ݴ����pTempInfo��
	int nTempNum;                   // �ϲ���Ҫ����ʱ�ĳ���
	int _nRecvPacketLen;

	// �ϲ�->�²�
	int nTempNumSend;               // �ϲ����²�д�������ָ��ʱ�ĳ���
	ROBOTCOMMAND CmdBuff;           // �ϲ������������ʱ�ռ�(����hEventNewCommand�¼��У���WAIT_OBJECT_0+3���������Ƶ�RobotPool[]��ӦС����)

	CMutex m_mutex;
	CMutex m_mutex2_list[NUM_ROBOT+1];
	CSerialPort Com;
	bool bInit;

	// �洢������
	CUsecTimer RobotCmdTimer[NUM_ROBOT + 1];	// ��ʱ�������ϲ����˶�ָ�����¸���
	double RobotCmdLostTime[NUM_ROBOT + 1];		// �ϲ��˶�ָ����ʧʱ��
	CRobotControl RobotPool[NUM_ROBOT + 1];		// ����ϲ����²㷢��ָ��ʱ��������
	CRobotControl RevRobotPool[NUM_ROBOT+1];	// ���˫��ͨѶ�յ�����ʱ,��ŵ�������
	int _updateValue[NUM_ROBOT+1];              // ��ʾ����˫��ͨѶ���ݷ��ص�ʱ��

	AbstractRadioPackets* pIRadioPacket;		// �������ĳ���ӿ�
};

unsigned __stdcall ControlThread(void *p);
unsigned __stdcall ReceiveThread(void *p);
//CThreadCreator::CallBackReturnType THREAD_CALLBACK ControlThread(CThreadCreator::CallBackParamType p);

#define WM_ROBOTFEEDBACK WM_USER+0x0100

#endif // ~__COMM_CONTROL_H__