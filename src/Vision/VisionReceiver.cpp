/************************************************************************/
/* file created by shengyu, 2005.3.23                                   */
/************************************************************************/
#include "visionreceiver.h"
#include <Simulator.h>
#include <CtrlBreakHandler.h>
#include <OptionModule.h>
#include <RefereeBoxIf.h>
#include <playmode.h>
#include <tinyxml/ParamReader.h>
#include <TimeCounter.h>
#include <thread>
#include <mutex>
#include "MyThread.h"
#include "Semaphore.h"
CEvent *visionEvent = new CEvent;
VisionReceiver* VisionReceiver::_instance = 0;
Message* VisionReceiver::_visionMessage = 0;
CMutex* decisionMutex = new CMutex; ///<用来线程互斥，决策线程和图像获取后唤醒，同上 visionEvent 配合使用 
extern Semaphore vision_semaphore;
Semaphore vision_finish_semaphore(0);
namespace{
	CThreadCreator *_thread = 0;
	bool DEBUG_MODE = false;
	COptionModule *_pOption = 0;
	CCtrlBreakHandler *_breakHandler = 0;
	CRefereeBoxInterface *_referee = 0; //裁判盒
	CMutex* visionMutex = 0; ///<用来线程互斥，保证数据完整
	bool IS_SIMULATION = true; // 默认不是仿真
	bool WRITE_LOG = false;
	CUsecTimer _usecTimer;
	static const u_short VISION_PORT = 12345;
	const int _RecentLogLength =  30; //seconds
	VisionLog *_log;
}

VisionReceiver::VisionReceiver(COptionModule *pOption, CCtrlBreakHandler *breakHandler):_threadAlive(false)
{
	{
		DECLARE_PARAM_READER_BEGIN(General)
			READ_PARAM(IS_SIMULATION)
			READ_PARAM(WRITE_LOG)
		DECLARE_PARAM_READER_END
	}

	_visionMessage = new Message(pOption);
	_pOption = pOption;
	_breakHandler = breakHandler;
	_info.cycle = 0;

	_log = new VisionLog;

	///> 跑实物,启动UDPSocket
	if(!IS_SIMULATION){	
		//_socket = new UDPSocket;
		//_socket->init(VISION_PORT);
		//std::thread t2(&MyThread::run,mt);//start thread

	}

	///> Log文件指针初始化
	if (WRITE_LOG){
		_log->createFile();
		std::cout << "Log create"<< std::endl;
		_log->m_bIsLogging = true;
	}

	///> 启动裁判盒
	_referee = RefereeBoxInterface::Instance();
	_referee->start();

	///> 初始化互斥锁
	visionMutex = new CMutex; 

	///> 启动视觉接收线程
	_thread = new CThreadCreator(getMessage, 0);
	_threadAlive = true;
	_usecTimer.start();

}

VisionReceiver::~VisionReceiver(void)
{
	if(visionEvent)
		delete visionEvent;
	if(visionMutex)
		delete visionMutex;
	if(_visionMessage)
		delete _visionMessage;
	if(_referee)
		delete _referee;
	if(_thread)
		delete _thread;
	if(_socket) 
		delete _socket;

	if(_log && _log->m_bIsLogging)
	{
		std::cout << "Deleting Log" << std::endl;
		_log->delEmptyFile();//删除无效Log
		delete _log;
	}
}

VisionReceiver* VisionReceiver::instance(COptionModule *pOption, CCtrlBreakHandler *breakHandler)
{
	if(_instance == 0)
		_instance = new VisionReceiver(pOption, breakHandler);
	return _instance;
}

void VisionReceiver::destruct()
{
	if(_instance)
		delete _instance;
}

extern CCtrlBreakHandler breakHandler;

CThreadCreator::CallBackReturnType THREAD_CALLBACK VisionReceiver::getMessage(CThreadCreator::CallBackParamType lpParam)
{
	///> 接受图像周期
	static unsigned int cycle = 0;
		

	//g_semaphore.Wait();


	///> 图像信息接受，需要锁的配合，并进入循环
	while (_breakHandler->breaked() < 0) {
		//cout << "1 : receiver " << endl;
		CServerInterface::VisualInfo temp;
		RefRecvMsg tempRefMsg;
		if(!IS_SIMULATION){		// 实物
			//if (VisionReceiver::instance()->_socket->receiveData(reinterpret_cast<char*> (_visionMessage), sizeof(Message)) <= 0) {
			//	break;
			//}
			vision_semaphore.Wait();
			(VisionReceiver::_visionMessage)->message2VisionInfo(temp);	// 图像接受线程转换图像数据
			//cout << "has receive data" << endl;
			temp.mode = _referee->getPlayMode();		// 裁判盒命令接受线程得到裁判盒指令,并设置相应mode
			tempRefMsg.blueGoal = _referee->getBlueGoalNum();
			tempRefMsg.yellowGoal = _referee->getYellowGoalNum();
			tempRefMsg.timeRemain = _referee->getRemainTime();
			tempRefMsg.blueGoalie = _referee->getBlueGoalie();
			tempRefMsg.yellowGoalie = _referee->getYellowGoalie();
		} else {		// 仿真
			CSimulator *simulator = CSimulator::instance(_pOption);
			if (!simulator->getVisualInfo(&temp)) {
				break;
			}
			temp.mode = _referee->getPlayMode();
			tempRefMsg.blueGoal = _referee->getBlueGoalNum();
			tempRefMsg.yellowGoal = _referee->getYellowGoalNum();
			tempRefMsg.timeRemain = _referee->getRemainTime();
			tempRefMsg.blueGoalie = _referee->getBlueGoalie();
			tempRefMsg.yellowGoalie = _referee->getYellowGoalie();
		}
				
		decisionMutex->lock();		// 决策加锁

		visionMutex->lock();		// 图像加锁
		VisionReceiver::instance()->_info = temp;
		VisionReceiver::instance()->_recvMsg = tempRefMsg;
		VisionReceiver::instance()->_info.cycle = ++cycle;  // 暂时这么代替，fix me!!! 没有用到图像里面的cycle,而是策略自己记录cycle,收到一个图像便加1;

		// 两者图像接受间隔时间检测
		_usecTimer.stop();
		double delt_time = _usecTimer.time()/1000.0f;
		_usecTimer.start();
		
		if (_log->m_bIsLogging) {
			_log->writeLog(_log->m_pfLog,temp);			
		}

		visionMutex->unlock();		// 图像解锁
		vision_finish_semaphore.Signal();
		if (delt_time < 1000.0/Param::Vision::FRAME_RATE) {
			// 睡眠，去掉噪声图像，保证稳定的图像帧率
			Sleep(1000.0/Param::Vision::FRAME_RATE-delt_time);	
			//std::cout<<cycle<<'\t'<<1000.0/Param::Vision::FRAME_RATE-delt_time<<std::endl;
		}
		visionEvent->pulse();

		decisionMutex->unlock();	// 决策解锁
	}

	///> 退出
	VisionReceiver::instance()->_threadAlive = false;
	visionEvent->set();

	return 0;
}

bool VisionReceiver::getVisionInfo(CServerInterface::VisualInfo& info, RefRecvMsg& msg) const
{
	if(_threadAlive){
		visionMutex->lock();		// 图像加锁
		info = this->_info;			// 将得到的图像数据输出给info
		msg  = this->_recvMsg;
		visionMutex->unlock();		// 图像解锁
	}

	return _threadAlive;
}