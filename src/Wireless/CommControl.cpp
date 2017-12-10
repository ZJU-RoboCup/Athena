/// FileName : 		CommControl.cpp
/// 				implementation file
/// Description :	It supports command send interface for ZJUNlict,
///	Keywords :		fitting, send, interface
/// Organization : 	ZJUNlict@Small Size League
/// Author : 		cliffyin
/// E-mail : 		cliffyin@zju.edu.cn
///					cliffyin007@gmail.com
/// Create Date : 	2011-07-25
/// Modified Date :	2011-07-25 
/// History :

#include "commcontrol.h"
#include <process.h>
#include "crc.h"
#include "RadioPackets.h"
#include <tinyxml/ParamReader.h>
#include <TimeCounter.h>
#include <fstream>
#include <CommandSender.h>
using namespace std;

#define UPDATECYCLE 1000

namespace {
	// ��ʱ��
	CUsecTimer _userTimer;
	const double ROBOT_LOST_TIME = 500.0;

	// �����ʣ��ϳ�Ϊ56000
	unsigned int COM_RATE = 115200;

	// ���ںţ�Ĭ��Ϊ1
	int COM_NUM = 1;

	// ��żУ��λ
	int PARITY = 0;

	// Э�����
	int PVERSION = 2008;
	int TIMERDELAY = 2;
	const int UPDATE_TimeOut = 50; 

	// �շ����
	int sendPacketNum[NUM_ROBOT];
	int receivePacketNum[NUM_ROBOT];

}

CCommControl::CCommControl(void)
{
	// ������־λ����
	bStart = false;
	bInit  = false;

	// û�������¼�
	hEventControlTimer   = NULL;
	hEventControlReceive = NULL;
	hEventControlCheck	 = NULL;
	hEventRequestInfo    = NULL;
	hEventNewCommand     = NULL;
	hEventRequestWait    = NULL;
	hEventNewCommandWait = NULL;
	hEventWatchDog       = NULL;

	// û�������߳�
	hThread = NULL;
	_threadID = 0;

	// 
	nTempNumSend = NUM_ROBOT;
	for (int i = 0; i <= NUM_ROBOT; i ++) {
		RobotPool[i].nRobotNum = i+1; 
		RobotPool[i].workingMode = 0;
		ClearRobotCmd(i);
		// �ײ��˶�ָ����¼��Ӷ�ʱ��
		RobotCmdTimer[i].start();
		RobotCmdLostTime[i] = 0.0;
	}

	// ��ʼ�����ջ�����
	for (int i = 0 ; i <= NUM_ROBOT; i ++) {
		// ���С���˶�ָ��ϴ�������Ϣ
		_updateValue[i] = 0;
		RevRobotPool[i].nRobotNum = i;
		RevRobotPool[i].fCurSpeed1 = -999;
		RevRobotPool[i].fCurSpeed2 = -999;
		RevRobotPool[i].fCurSpeed3 = -999;
		RevRobotPool[i].fCurSpeed4 = -999;		
	}

	memset(sendPacketNum, 0, sizeof(sendPacketNum));
	memset(receivePacketNum, 0, sizeof(receivePacketNum));

	// ���봮�����ò�����Э��汾
	DECLARE_PARAM_READER_BEGIN(CCommControl)
	READ_PARAM(COM_NUM)
	READ_PARAM(COM_RATE)
	READ_PARAM(PARITY)
	READ_PARAM(PVERSION)
	DECLARE_PARAM_READER_END

	switch (PVERSION)
	{
	case 2011:
		{
			pIRadioPacket = new RadioPacketV2011; 
			TIMERDELAY = 8;
			_nRecvPacketLen = 7;
			break;
		}
	case 2013:
		{
			pIRadioPacket = new RadioPacketV2013; 
			TIMERDELAY = 8;
			_nRecvPacketLen = 17;
			break;
		}
	case 2016:
		{
			pIRadioPacket = new RadioPacketV2016; 
			TIMERDELAY = 8;
			_nRecvPacketLen = 24;
			break;
		}

	default:
		{
			pIRadioPacket = new RadioPacketV2011;
			printf("No PVERSION!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			break;
		}
	}
}

CCommControl::~CCommControl(void)
{
	delete pIRadioPacket;
}

void CCommControl::Init()
{
	Com.pControl = this;

	// �򿪴���
	if (! Com.OpenPort(COM_NUM, COM_RATE, 8, 0, PARITY)) {
		std::cout<<"Comm Open Error!"<<std::endl;
		Error();
	}
	
	// �����˶�ָ��ڷ��͵Ĵ����¼�
	hEventControlTimer = CreateEvent(NULL,true,false,"COMM Control Timer");
	if (! hEventControlTimer) {
		std::cout<<"CommControl: Create Event Error!"<<std::endl;
		Error();
		CloseAll();
		return;
	}

	// ����С������״̬���صĴ����¼�
	hEventControlReceive = CreateEvent(NULL,true,false,"COMM Control Receive");
	if (! hEventControlReceive) {
		std::cout<<"CommControl: Create Event Error!"<<std::endl;
		Error();
		CloseAll();
		return;
	}

	// ����С�����˶�ָ����´����¼�
	hEventControlCheck = CreateEvent(NULL,true,false,"COMM Control Cmd Check");
	if (! hEventControlCheck) {
		std::cout<<"CommControl: Create Event Error!"<<std::endl;
		Error();
		CloseAll();
		return;
	}

	// ����С������״̬��ѯ�Ĵ����¼�
	hEventRequestInfo = CreateEvent(NULL,true,false,"COMM Control Request Info");
	if (! hEventRequestInfo) {
		std::cout<<"CommControl: Create Event Error!"<<std::endl;
		Error();
		CloseAll();
		return;
	}

	// ����С������״̬��ѯ�ĵȴ��¼�
	hEventRequestWait = CreateEvent(NULL,true,false,"COMM Control Request Info Wait");
	if (! hEventRequestWait) {
		std::cout<<"CommControl: Create Event Error!"<<std::endl;
		Error();
		CloseAll();
		return;
	}

	// �����µ�С������ָ��Ĵ����¼�
	hEventNewCommand = CreateEvent(NULL,true,false,"COMM Control New Command");
	if (! hEventNewCommand) {
		std::cout<<"CommControl: Create Event Error!"<<std::endl;
		Error();
		CloseAll();
		return;
	}

	// �����µ�С������ָ��ĵȴ��¼�
	hEventNewCommandWait = CreateEvent(NULL,true,false,"COMM Control New Command Wait");
	if (! hEventNewCommandWait) {
		std::cout<<"CommControl: Create Event Error!"<<std::endl;
		Error();
		CloseAll();
		return;
	}

	// �������Ź������¼�
	hEventWatchDog = CreateEvent(NULL, true, false, "Watch Dog");
	if (! hEventWatchDog) {
		std::cout<<"CommControl: Create Watch Dog Event Error!"<<std::endl;
		Error();
		CloseAll();
	}

	// ���ø߾��ȶ�ʱ���������Ϊ1ms���ж�
	MMRESULT mRes;
	mRes = timeBeginPeriod(1);
	if (TIMERR_NOCANDO == mRes) {
		std::cout<<"CommControl: Create Event Error!"<<std::endl;
		Error();
		CloseAll();
		return;
	}

	// ���������߳�
	bStart=true;
	hThread = (HANDLE)_beginthreadex( NULL, 0, &ControlThread, this, 0, &_threadID );
	hThread = (HANDLE)_beginthreadex( NULL, 0, &ReceiveThread, this, 0, &_threadID );

	// ע���жϵĻص�����
	// �ж�һ�������Զ�����λ��С�������˶�ָ����ڴ��������Ϊ TIMERDELAY
	mRes = timeSetEvent(TIMERDELAY,1,(LPTIMECALLBACK)hEventControlTimer,NULL,
						TIME_PERIODIC|TIME_CALLBACK_EVENT_SET);	
	// �ж϶��������Զ�����λ��С�������˶�ָ����ڴ��������Ϊ TIMERDELAY
	mRes = timeSetEvent(100,1,(LPTIMECALLBACK)hEventControlCheck,NULL,
						TIME_PERIODIC|TIME_CALLBACK_EVENT_SET);
	// �ж���������ϵͳ�ָ�ʹ�ã���ʱ����ûʹ�ã�һ�δ�������ʱΪ UPDATE_TimeOut
	watchDogID = timeSetEvent(UPDATE_TimeOut, 1, (LPTIMECALLBACK)hEventWatchDog, NULL,
						TIME_ONESHOT|TIME_CALLBACK_EVENT_SET);
	
	if (mRes) {
		uTimerID = mRes;
	} else {
		Destroy();
		Error();
		return;
	}

	bInit = true;
	_userTimer.start();

	return ;
}

void CCommControl::Destroy()
{
	if (bInit) {
		timeKillEvent(uTimerID);

		bStart = false;
		SetEvent(hEventControlTimer);
		WaitForSingleObject(hThread, INFINITE);

		timeEndPeriod(1);
		CloseAll();
		bInit = false;
	}

	return ;
}

void CCommControl::CloseAll()
{
	if (hEventControlReceive) {
		CloseHandle(hEventControlReceive);
	}

	if (hEventControlCheck) {
		CloseHandle(hEventControlCheck);
	}

	if (hEventControlTimer) {
		CloseHandle(hEventControlTimer);
	}

	if (hEventRequestInfo) {
		CloseHandle(hEventRequestInfo);
	}

	if (hEventRequestWait) {
		CloseHandle(hEventRequestWait);
	}

	if (hEventNewCommand) {
		CloseHandle(hEventNewCommand);
	}

	if (hEventNewCommandWait) {
		CloseHandle(hEventNewCommandWait);
	}

	if (hEventWatchDog) {
		CloseHandle(hEventWatchDog);
	}

	Com.ClosePort();

	hEventControlTimer		= NULL;
	hEventControlReceive	= NULL;
	hEventControlCheck		= NULL;
	hEventRequestInfo		= NULL;
	hEventNewCommand		= NULL;
	hEventRequestWait		= NULL;
	hEventNewCommandWait	= NULL;
	hEventWatchDog			= NULL;

	return ;
}

void CCommControl::Error()
{
	LPVOID lpMsgBuf;

	if (!FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL ))
	{
		return;
	}

	MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );

	LocalFree( lpMsgBuf );

	return ;
}

//CThreadCreator::CallBackReturnType THREAD_CALLBACK ControlThread(CThreadCreator::CallBackParamType p)
unsigned __stdcall ReceiveThread(void *p)
{
    CCommControl *pComm;
	pComm = (CCommControl*)p;
	HANDLE hreceivewait[2];
	hreceivewait[0] = pComm->hEventControlReceive;
	hreceivewait[1] = pComm->hEventRequestInfo;
	DWORD rcvRes;
	while (pComm->bStart)
	{
		rcvRes = WaitForMultipleObjects(2,hreceivewait,false,INFINITE);
		switch(rcvRes)
		{
		case WAIT_OBJECT_0:
			{
				// �ȴ��յ�����(˫��ͨѶ)
				pComm->m_mutex.lock();
				pComm->DecodePacket();
				ResetEvent(pComm->hEventControlReceive);
				pComm->m_mutex.unlock();
			}
			break;
		case WAIT_OBJECT_0 + 1:
			// �ײ����ݿ�����pTempInfo(������RequestSpeedInfo�����е�"info"ָ��)��ȥ�����ϲ����
			pComm->pTempInfo->nRobotNum			=	pComm->RevRobotPool[pComm->nTempNum].nRobotNum;
			
			pComm->RevRobotPool[pComm->nTempNum].ntof();
			pComm->pTempInfo->fCurSpeed1		=	pComm->RevRobotPool[pComm->nTempNum].fCurSpeed1;
			pComm->pTempInfo->fCurSpeed2		=	pComm->RevRobotPool[pComm->nTempNum].fCurSpeed2;
			pComm->pTempInfo->fCurSpeed3		=	pComm->RevRobotPool[pComm->nTempNum].fCurSpeed3;
			pComm->pTempInfo->fCurSpeed4		=	pComm->RevRobotPool[pComm->nTempNum].fCurSpeed4;

			pComm->pTempInfo->nKickInfo			=	pComm->RevRobotPool[pComm->nTempNum].kickInfo;		
			pComm->pTempInfo->bInfraredInfo		=	pComm->RevRobotPool[pComm->nTempNum].bInfraredInfo;

			pComm->pTempInfo->bControledInfo	=	pComm->RevRobotPool[pComm->nTempNum].bControledInfo;
			pComm->pTempInfo->changeNum			=	pComm->RevRobotPool[pComm->nTempNum].changeNum;
			pComm->pTempInfo->changeCountNum	=	pComm->RevRobotPool[pComm->nTempNum].changeCountNum;
			pComm->pTempInfo->battery = pComm->RevRobotPool[pComm->nTempNum].battery;
			pComm->pTempInfo->capacity = pComm->RevRobotPool[pComm->nTempNum].capacity;
			//if ( pComm->nTempNum == 3 || pComm->nTempNum == 2 ) printf("number = %d,buffer address = %p\n",pComm->pTempInfo->bInfraredInfo,pComm->pTempInfo);
			ResetEvent(pComm->hEventRequestInfo);
			SetEvent(pComm->hEventRequestWait);
			break;
		default:
			break;
		}
	}
	_endthreadex( 0 );
	return 0;
}

unsigned __stdcall ControlThread(void *p)
{
	CCommControl *pComm;
	pComm = (CCommControl*)p;

	const int EVENT_NUM = 4;
	HANDLE hWait[EVENT_NUM];
	hWait[0] = pComm->hEventControlTimer;		// �ȴ���ָ�����ɿɷ�����ʽ�����ݣ������ͳ�ȥ��event
	//hWait[1] = pComm->hEventControlReceive;		// �ȴ��յ�����
	//hWait[2] = pComm->hEventRequestInfo;	    // ���ٶ�ָ��ֽ��й�.����û���õ�
	hWait[1] = pComm->hEventNewCommand;			// �ȴ���ȡ�ϲ��ٶȿ����ָ�����ݣ�����ֵ��CCommControl�������Ӧ������ȥ��event��ע����hEventControlTimer������
	hWait[2] = pComm->hEventWatchDog;
	hWait[3] = pComm->hEventControlCheck;

	int i = 0;
	DWORD dRes;

	while (pComm->bStart)
	{
		// �ȴ����ձ��������¼�
		dRes = WaitForMultipleObjects(EVENT_NUM,hWait,false,50);

		switch (dRes)
		{
		case WAIT_OBJECT_0 :
			{
				// ���ֽ�ָ��д������,���ͳ�ȥ����������������Ҫ���ε��¼�����
				pComm->EncodePacket(i);

				// ĿǰΪ��������
				// i == 0 ��һ�������복��С��ǰ������
				// i == 0 �ڶ��������복�Ŵ�ĺ�������
				i++;
				if (i >= 2) {
					i = 0;
				}

				ResetEvent(pComm->hEventControlTimer);	// ���ͽ���,����ָ��ͱ�־
			}
			break;

		case WAIT_OBJECT_0 + 1:
			// ----> ROBOTCOMMAND --doWirelessModule().NewRobotCommand()--> CmdBuff[] --> RobotPool[]
			// �ȴ��ϲ㴫�����ٶȿ����ָ�����ݣ���ʱ�ѷŵ�CmdBuff��
			pComm->RobotPool[pComm->nTempNumSend].fSpeed1	=	pComm->CmdBuff.speed[0];
			pComm->RobotPool[pComm->nTempNumSend].fSpeed2	=	pComm->CmdBuff.speed[1];
			pComm->RobotPool[pComm->nTempNumSend].fSpeed3	=	pComm->CmdBuff.speed[2];
			pComm->RobotPool[pComm->nTempNumSend].fSpeed4	=	pComm->CmdBuff.speed[3];
			pComm->RobotPool[pComm->nTempNumSend].fton();

			// ����ʹ���ϲ��ٶȷֽ��Э��,������3�����ݽ��ò���
			pComm->RobotPool[pComm->nTempNumSend].workingMode = pComm->CmdBuff.mode;
			pComm->RobotPool[pComm->nTempNumSend].fX = pComm->CmdBuff.x;
			pComm->RobotPool[pComm->nTempNumSend].fY = pComm->CmdBuff.y;
			pComm->RobotPool[pComm->nTempNumSend].fRotate = pComm->CmdBuff.rotate;
			pComm->RobotPool[pComm->nTempNumSend].nControlBall = pComm->CmdBuff.cb;
			pComm->RobotPool[pComm->nTempNumSend].nKick = pComm->CmdBuff.shoot;

			// �����������ݴ���
			pComm->RobotPool[pComm->nTempNumSend].gyro = pComm->CmdBuff.gyro;
			pComm->RobotPool[pComm->nTempNumSend].angle = pComm->CmdBuff.angle;
			pComm->RobotPool[pComm->nTempNumSend].radius = pComm->CmdBuff.radius;
            
             pComm->RobotPool[pComm->nTempNumSend].stop = pComm->CmdBuff.stop;
			//PID Param
			if (pComm->CmdBuff.mode == MODE_DEBUG_PID_WRITE) {
				for (int ix = 0; ix < 4; ix++) {
					pComm->RobotPool[pComm->nTempNumSend].prop[ix] = pComm->CmdBuff.driver_p[ix];
					pComm->RobotPool[pComm->nTempNumSend].intg[ix] = pComm->CmdBuff.driver_i[ix];
					pComm->RobotPool[pComm->nTempNumSend].diff[ix] = pComm->CmdBuff.driver_d[ix];
				}
			}

			// ������ʧʱ��: �����·�������Ϊ0��ע���±� 0 ��ʼ
			pComm->RobotCmdLostTime[pComm->nTempNumSend] = 0.0;
			//cout << "pComm->nTempNumSend : " << pComm->nTempNumSend + 1 << endl;

			// �����ǰ���˶�ָ���·��������������ŵĻ�
			if (0 <= pComm->nTempNumSend && pComm->nTempNumSend < NUM_ROBOT) {
				
			}	

			// ����hEventNewCommand�¼�����ʾ���Խ�����һ���ڵ�ָ��ͣ�
			ResetEvent(pComm->hEventNewCommand);

			// ִ����ָ��ͺ�����event: hEventNewCommandWait�� 
			// ʹ����NewRobotCommand()�еĵȴ��߳̽����ȴ������Խ�����һ�������ݵ�����ĸ���
			// �������,�������event��ǩ,ʹ��NewCommand()����
			SetEvent(pComm->hEventNewCommandWait);	
			
			break;

		case WAIT_OBJECT_0 + 2:
			{
				pComm->RobotPool[pComm->nTempNumSend].fSpeed1 = 0;
				pComm->RobotPool[pComm->nTempNumSend].fSpeed2 = 0;
				pComm->RobotPool[pComm->nTempNumSend].fSpeed3 = 0;
				pComm->RobotPool[pComm->nTempNumSend].fSpeed4 = 0;
				pComm->RobotPool[pComm->nTempNumSend].fton();

				// ����ʹ���ϲ��ٶȷֽ��Э��,������3�����ݽ��ò���
				pComm->RobotPool[pComm->nTempNumSend].fX = 0;
				pComm->RobotPool[pComm->nTempNumSend].fY = 0;
				pComm->RobotPool[pComm->nTempNumSend].fRotate = 0;

				pComm->RobotPool[pComm->nTempNumSend].workingMode = 0;
				pComm->RobotPool[pComm->nTempNumSend].nControlBall=0;
				pComm->RobotPool[pComm->nTempNumSend].nKick = 0;
				ResetEvent(pComm->hEventWatchDog);
				break;
			}

		case WAIT_OBJECT_0 + 3:	// TODO : cliffyin
			{
				// ���¸���С����Ӧָ����ʧ��ʱ����ʱ��
				for (int i = 0; i <= NUM_ROBOT; i ++) {
					pComm->RobotCmdTimer[i].stop();
					double tmpLostTime = pComm->RobotCmdLostTime[i];
					pComm->RobotCmdLostTime[i] = tmpLostTime + pComm->RobotCmdTimer[i].time()/1000.0f;
					pComm->RobotCmdTimer[i].start();
				}

				// ����Ƿ���С��ָ����ʧ�������˶���Ҫ���ø�λ
				bool resetNow = false;
				for (int i = 0; i <= NUM_ROBOT; i ++) {
					if (pComm->RobotCmdLostTime[i] > ROBOT_LOST_TIME) {
						pComm->ClearRobotCmd(i);
						resetNow = true;
					}
					//cout <<i+1 <<  " with : *****************" << pComm->RobotCmdLostTime[i] << endl;
				}

				// �˶�ָ�����ø�λ���������ź�
				if (resetNow) {
					pComm->nTempNumSend = NUM_ROBOT;
					SetEvent(pComm->hEventNewCommand);
				}

				ResetEvent(pComm->hEventControlCheck);
			}
			break;

		case WAIT_TIMEOUT:
			break;

		default:
			break;
		}
	}

	_endthreadex( 0 );
	return 0;
}

int CCommControl::RequestSpeedInfo(int n,ROBOTINFO *info)
{
	if (n < 1 || n > NUM_ROBOT) {
		return -1;
	}

	pTempInfo = info;
	nTempNum = n;

	// �����ϲ����²�ȥС�����ݵ�event��ʹControlThread()��WaitForMultipleObjects()�����ܽ���׽����ִ����Ӧ�¼�(WAIT_OBJECT_0+2)��
	SetEvent(hEventRequestInfo);

    // ��WaitForSingleObject�����еȴ�hEventRequestWait�¼��ķ�������ControlThread()�е�setEvent(hEventRequestWait)
	WaitForSingleObject(hEventRequestWait,INFINITE);
	ResetEvent(hEventRequestWait);

	m_mutex2_list[n].lock();
	int temp = _updateValue[n];
	m_mutex2_list[n].unlock();

	return temp;
}

void CCommControl::ClearRobotCmd(int robotNum)
{
	RobotPool[robotNum].fSpeed1	=	0.0;
	RobotPool[robotNum].fSpeed2	=	0.0;
	RobotPool[robotNum].fSpeed3	=	0.0;
	RobotPool[robotNum].fSpeed4	=	0.0;

	// ����ʹ���ϲ��ٶȷֽ��Э��,������3�����ݽ��ò���
	RobotPool[robotNum].workingMode = 0;
	RobotPool[robotNum].fX = 0.0;
	RobotPool[robotNum].fY = 0.0;
	RobotPool[robotNum].fRotate = 0.0;
	RobotPool[robotNum].nControlBall = 0;
	RobotPool[robotNum].nKick = 0;

	// �����������ݴ���
	RobotPool[robotNum].gyro = false;
	RobotPool[robotNum].angle = 0.0;
	RobotPool[robotNum].radius = 0;

	return ;
}

void CCommControl::NewRobotCommand(int n, const ROBOTCOMMAND* command)
{
	// ��Ч�Լ��
	if (n < 0 || n >= NUM_ROBOT) {
		//cout<<"n:"<<n<<endl;
		return;
	}
	//cout<<"n:"<<n<<endl;
	// ��λ���Ź�
	timeKillEvent(watchDogID);

	// ����ָ���С����
	nTempNumSend = n;

	// ָ�����ݿ���
	memcpy(&CmdBuff,command,sizeof(ROBOTCOMMAND));
	

	// �趨����ָ��event�ı�־, �߳̿�ʼ�ȴ�����ָ��
	SetEvent(hEventNewCommand);

	// �ȴ��������event�ı�־, ������󷵻�;
	WaitForSingleObject(hEventNewCommandWait,INFINITE);	

	// ���÷������event��־
	ResetEvent(hEventNewCommandWait);	
	
	// �������Ź�
	watchDogID = timeSetEvent(UPDATE_TimeOut, 1, (LPTIMECALLBACK)hEventWatchDog, NULL,
								TIME_ONESHOT|TIME_CALLBACK_EVENT_SET);

	return ;
}

void CCommControl::EncodePacket(int pkgNum)
{
	// ----> ROBOTCOMMAND --doWirelessModule().NewRobotCommand()--> CmdBuff[] --> RobotPool[] --EncodePacket()--> TXBuff[]
	pIRadioPacket->setpkgnum(pkgNum);
	pIRadioPacket->encode(RobotPool, &Com);
}

void CCommControl::DecodePacket()
{
	int Num;
	if (pIRadioPacket->decode(RXBuff, RevRobotPool, Num)) {
		m_mutex2_list[Num].lock();
		_updateValue[Num] = (_updateValue[Num] + 1) % UPDATECYCLE;
		m_mutex2_list[Num].unlock();
	}
}
