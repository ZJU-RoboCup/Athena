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
	// 定时器
	CUsecTimer _userTimer;
	const double ROBOT_LOST_TIME = 500.0;

	// 波特率，老车为56000
	unsigned int COM_RATE = 115200;

	// 串口号，默认为1
	int COM_NUM = 1;

	// 奇偶校验位
	int PARITY = 0;

	// 协议相关
	int PVERSION = 2008;
	int TIMERDELAY = 2;
	const int UPDATE_TimeOut = 50; 

	// 收发相关
	int sendPacketNum[NUM_ROBOT];
	int receivePacketNum[NUM_ROBOT];

}

CCommControl::CCommControl(void)
{
	// 启动标志位清零
	bStart = false;
	bInit  = false;

	// 没有设置事件
	hEventControlTimer   = NULL;
	hEventControlReceive = NULL;
	hEventControlCheck	 = NULL;
	hEventRequestInfo    = NULL;
	hEventNewCommand     = NULL;
	hEventRequestWait    = NULL;
	hEventNewCommandWait = NULL;
	hEventWatchDog       = NULL;

	// 没有启动线程
	hThread = NULL;
	_threadID = 0;

	// 
	nTempNumSend = NUM_ROBOT;
	for (int i = 0; i <= NUM_ROBOT; i ++) {
		RobotPool[i].nRobotNum = i+1; 
		RobotPool[i].workingMode = 0;
		ClearRobotCmd(i);
		// 底层运动指令更新监视定时器
		RobotCmdTimer[i].start();
		RobotCmdLostTime[i] = 0.0;
	}

	// 初始化接收缓冲区
	for (int i = 0 ; i <= NUM_ROBOT; i ++) {
		// 清空小车运动指令：上传本体信息
		_updateValue[i] = 0;
		RevRobotPool[i].nRobotNum = i;
		RevRobotPool[i].fCurSpeed1 = -999;
		RevRobotPool[i].fCurSpeed2 = -999;
		RevRobotPool[i].fCurSpeed3 = -999;
		RevRobotPool[i].fCurSpeed4 = -999;		
	}

	memset(sendPacketNum, 0, sizeof(sendPacketNum));
	memset(receivePacketNum, 0, sizeof(receivePacketNum));

	// 读入串口设置参数及协议版本
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

	// 打开串口
	if (! Com.OpenPort(COM_NUM, COM_RATE, 8, 0, PARITY)) {
		std::cout<<"Comm Open Error!"<<std::endl;
		Error();
	}
	
	// 创建运动指令串口发送的触发事件
	hEventControlTimer = CreateEvent(NULL,true,false,"COMM Control Timer");
	if (! hEventControlTimer) {
		std::cout<<"CommControl: Create Event Error!"<<std::endl;
		Error();
		CloseAll();
		return;
	}

	// 创建小车本体状态返回的触发事件
	hEventControlReceive = CreateEvent(NULL,true,false,"COMM Control Receive");
	if (! hEventControlReceive) {
		std::cout<<"CommControl: Create Event Error!"<<std::endl;
		Error();
		CloseAll();
		return;
	}

	// 创建小车新运动指令更新触发事件
	hEventControlCheck = CreateEvent(NULL,true,false,"COMM Control Cmd Check");
	if (! hEventControlCheck) {
		std::cout<<"CommControl: Create Event Error!"<<std::endl;
		Error();
		CloseAll();
		return;
	}

	// 创建小车本体状态查询的触发事件
	hEventRequestInfo = CreateEvent(NULL,true,false,"COMM Control Request Info");
	if (! hEventRequestInfo) {
		std::cout<<"CommControl: Create Event Error!"<<std::endl;
		Error();
		CloseAll();
		return;
	}

	// 创建小车本体状态查询的等待事件
	hEventRequestWait = CreateEvent(NULL,true,false,"COMM Control Request Info Wait");
	if (! hEventRequestWait) {
		std::cout<<"CommControl: Create Event Error!"<<std::endl;
		Error();
		CloseAll();
		return;
	}

	// 创建新的小车控制指令的触发事件
	hEventNewCommand = CreateEvent(NULL,true,false,"COMM Control New Command");
	if (! hEventNewCommand) {
		std::cout<<"CommControl: Create Event Error!"<<std::endl;
		Error();
		CloseAll();
		return;
	}

	// 创建新的小车控制指令的等待事件
	hEventNewCommandWait = CreateEvent(NULL,true,false,"COMM Control New Command Wait");
	if (! hEventNewCommandWait) {
		std::cout<<"CommControl: Create Event Error!"<<std::endl;
		Error();
		CloseAll();
		return;
	}

	// 创建看门狗触发事件
	hEventWatchDog = CreateEvent(NULL, true, false, "Watch Dog");
	if (! hEventWatchDog) {
		std::cout<<"CommControl: Create Watch Dog Event Error!"<<std::endl;
		Error();
		CloseAll();
	}

	// 设置高精度定时，产生间隔为1ms的中断
	MMRESULT mRes;
	mRes = timeBeginPeriod(1);
	if (TIMERR_NOCANDO == mRes) {
		std::cout<<"CommControl: Create Event Error!"<<std::endl;
		Error();
		CloseAll();
		return;
	}

	// 开启控制线程
	bStart=true;
	hThread = (HANDLE)_beginthreadex( NULL, 0, &ControlThread, this, 0, &_threadID );
	hThread = (HANDLE)_beginthreadex( NULL, 0, &ReceiveThread, this, 0, &_threadID );

	// 注册中断的回调函数
	// 中断一：用于自动向下位机小车发送运动指令，周期触发，间隔为 TIMERDELAY
	mRes = timeSetEvent(TIMERDELAY,1,(LPTIMECALLBACK)hEventControlTimer,NULL,
						TIME_PERIODIC|TIME_CALLBACK_EVENT_SET);	
	// 中断二：用于自动向下位机小车发送运动指令，周期触发，间隔为 TIMERDELAY
	mRes = timeSetEvent(100,1,(LPTIMECALLBACK)hEventControlCheck,NULL,
						TIME_PERIODIC|TIME_CALLBACK_EVENT_SET);
	// 中断三：用于系统恢复使用，暂时好像没使用，一次触发，需时为 UPDATE_TimeOut
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
				// 等待收到数据(双向通讯)
				pComm->m_mutex.lock();
				pComm->DecodePacket();
				ResetEvent(pComm->hEventControlReceive);
				pComm->m_mutex.unlock();
			}
			break;
		case WAIT_OBJECT_0 + 1:
			// 底层数据拷贝到pTempInfo(即函数RequestSpeedInfo参数中的"info"指针)中去，供上层调用
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
	hWait[0] = pComm->hEventControlTimer;		// 等待将指令打包成可发送形式的数据，并发送出去的event
	//hWait[1] = pComm->hEventControlReceive;		// 等待收到数据
	//hWait[2] = pComm->hEventRequestInfo;	    // 与速度指令分解有关.现在没有用到
	hWait[1] = pComm->hEventNewCommand;			// 等待获取上层速度控球等指令数据，并赋值到CCommControl对象的相应变量中去的event（注意与hEventControlTimer的区别）
	hWait[2] = pComm->hEventWatchDog;
	hWait[3] = pComm->hEventControlCheck;

	int i = 0;
	DWORD dRes;

	while (pComm->bStart)
	{
		// 等待接收被触发的事件
		dRes = WaitForMultipleObjects(EVENT_NUM,hWait,false,50);

		switch (dRes)
		{
		case WAIT_OBJECT_0 :
			{
				// 将字节指令写到串口,发送出去，两包完整发送需要两次的事件触发
				pComm->EncodePacket(i);

				// 目前为两包发送
				// i == 0 第一包，编码车号小的前三辆车
				// i == 0 第二包，编码车号大的后两辆车
				i++;
				if (i >= 2) {
					i = 0;
				}

				ResetEvent(pComm->hEventControlTimer);	// 发送结束,重置指令发送标志
			}
			break;

		case WAIT_OBJECT_0 + 1:
			// ----> ROBOTCOMMAND --doWirelessModule().NewRobotCommand()--> CmdBuff[] --> RobotPool[]
			// 等待上层传下来速度控球等指令数据（此时已放到CmdBuff）
			pComm->RobotPool[pComm->nTempNumSend].fSpeed1	=	pComm->CmdBuff.speed[0];
			pComm->RobotPool[pComm->nTempNumSend].fSpeed2	=	pComm->CmdBuff.speed[1];
			pComm->RobotPool[pComm->nTempNumSend].fSpeed3	=	pComm->CmdBuff.speed[2];
			pComm->RobotPool[pComm->nTempNumSend].fSpeed4	=	pComm->CmdBuff.speed[3];
			pComm->RobotPool[pComm->nTempNumSend].fton();

			// 若是使用上层速度分解的协议,则下面3个数据将用不上
			pComm->RobotPool[pComm->nTempNumSend].workingMode = pComm->CmdBuff.mode;
			pComm->RobotPool[pComm->nTempNumSend].fX = pComm->CmdBuff.x;
			pComm->RobotPool[pComm->nTempNumSend].fY = pComm->CmdBuff.y;
			pComm->RobotPool[pComm->nTempNumSend].fRotate = pComm->CmdBuff.rotate;
			pComm->RobotPool[pComm->nTempNumSend].nControlBall = pComm->CmdBuff.cb;
			pComm->RobotPool[pComm->nTempNumSend].nKick = pComm->CmdBuff.shoot;

			// 将陀螺仪数据传入
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

			// 更新消失时间: 若有下发则重置为0，注意下标 0 开始
			pComm->RobotCmdLostTime[pComm->nTempNumSend] = 0.0;
			//cout << "pComm->nTempNumSend : " << pComm->nTempNumSend + 1 << endl;

			// 如果当前的运动指令下发车号是正常车号的话
			if (0 <= pComm->nTempNumSend && pComm->nTempNumSend < NUM_ROBOT) {
				
			}	

			// 重置hEventNewCommand事件，表示可以进行下一周期的指令发送；
			ResetEvent(pComm->hEventNewCommand);

			// 执行完指令发送后，设置event: hEventNewCommandWait； 
			// 使函数NewRobotCommand()中的等待线程结束等待，可以进行下一步的数据到缓存的复制
			// 发送完毕,设置完毕event标签,使能NewCommand()返回
			SetEvent(pComm->hEventNewCommandWait);	
			
			break;

		case WAIT_OBJECT_0 + 2:
			{
				pComm->RobotPool[pComm->nTempNumSend].fSpeed1 = 0;
				pComm->RobotPool[pComm->nTempNumSend].fSpeed2 = 0;
				pComm->RobotPool[pComm->nTempNumSend].fSpeed3 = 0;
				pComm->RobotPool[pComm->nTempNumSend].fSpeed4 = 0;
				pComm->RobotPool[pComm->nTempNumSend].fton();

				// 若是使用上层速度分解的协议,则下面3个数据将用不上
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
				// 更新各个小车对应指令消失计时器的时间
				for (int i = 0; i <= NUM_ROBOT; i ++) {
					pComm->RobotCmdTimer[i].stop();
					double tmpLostTime = pComm->RobotCmdLostTime[i];
					pComm->RobotCmdLostTime[i] = tmpLostTime + pComm->RobotCmdTimer[i].time()/1000.0f;
					pComm->RobotCmdTimer[i].start();
				}

				// 检测是否有小车指令消失过长，运动需要重置复位
				bool resetNow = false;
				for (int i = 0; i <= NUM_ROBOT; i ++) {
					if (pComm->RobotCmdLostTime[i] > ROBOT_LOST_TIME) {
						pComm->ClearRobotCmd(i);
						resetNow = true;
					}
					//cout <<i+1 <<  " with : *****************" << pComm->RobotCmdLostTime[i] << endl;
				}

				// 运动指令重置复位设置启动信号
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

	// 设置上层向下层去小车数据的event（使ControlThread()的WaitForMultipleObjects()函数能将捕捉到并执行相应事件(WAIT_OBJECT_0+2)）
	SetEvent(hEventRequestInfo);

    // 在WaitForSingleObject函数中等待hEventRequestWait事件的发生，即ControlThread()中的setEvent(hEventRequestWait)
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

	// 若是使用上层速度分解的协议,则下面3个数据将用不上
	RobotPool[robotNum].workingMode = 0;
	RobotPool[robotNum].fX = 0.0;
	RobotPool[robotNum].fY = 0.0;
	RobotPool[robotNum].fRotate = 0.0;
	RobotPool[robotNum].nControlBall = 0;
	RobotPool[robotNum].nKick = 0;

	// 将陀螺仪数据传入
	RobotPool[robotNum].gyro = false;
	RobotPool[robotNum].angle = 0.0;
	RobotPool[robotNum].radius = 0;

	return ;
}

void CCommControl::NewRobotCommand(int n, const ROBOTCOMMAND* command)
{
	// 有效性检查
	if (n < 0 || n >= NUM_ROBOT) {
		//cout<<"n:"<<n<<endl;
		return;
	}
	//cout<<"n:"<<n<<endl;
	// 复位看门狗
	timeKillEvent(watchDogID);

	// 发送指令的小车号
	nTempNumSend = n;

	// 指令内容拷贝
	memcpy(&CmdBuff,command,sizeof(ROBOTCOMMAND));
	

	// 设定发送指令event的标志, 线程开始等待发送指令
	SetEvent(hEventNewCommand);

	// 等待发送完毕event的标志, 发送完后返回;
	WaitForSingleObject(hEventNewCommandWait,INFINITE);	

	// 重置发送完毕event标志
	ResetEvent(hEventNewCommandWait);	
	
	// 重启看门狗
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
