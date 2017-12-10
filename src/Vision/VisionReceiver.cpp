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
CMutex* decisionMutex = new CMutex; ///<�����̻߳��⣬�����̺߳�ͼ���ȡ���ѣ�ͬ�� visionEvent ���ʹ�� 
extern Semaphore vision_semaphore;
Semaphore vision_finish_semaphore(0);
namespace{
	CThreadCreator *_thread = 0;
	bool DEBUG_MODE = false;
	COptionModule *_pOption = 0;
	CCtrlBreakHandler *_breakHandler = 0;
	CRefereeBoxInterface *_referee = 0; //���к�
	CMutex* visionMutex = 0; ///<�����̻߳��⣬��֤��������
	bool IS_SIMULATION = true; // Ĭ�ϲ��Ƿ���
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

	///> ��ʵ��,����UDPSocket
	if(!IS_SIMULATION){	
		//_socket = new UDPSocket;
		//_socket->init(VISION_PORT);
		//std::thread t2(&MyThread::run,mt);//start thread

	}

	///> Log�ļ�ָ���ʼ��
	if (WRITE_LOG){
		_log->createFile();
		std::cout << "Log create"<< std::endl;
		_log->m_bIsLogging = true;
	}

	///> �������к�
	_referee = RefereeBoxInterface::Instance();
	_referee->start();

	///> ��ʼ��������
	visionMutex = new CMutex; 

	///> �����Ӿ������߳�
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
		_log->delEmptyFile();//ɾ����ЧLog
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
	///> ����ͼ������
	static unsigned int cycle = 0;
		

	//g_semaphore.Wait();


	///> ͼ����Ϣ���ܣ���Ҫ������ϣ�������ѭ��
	while (_breakHandler->breaked() < 0) {
		//cout << "1 : receiver " << endl;
		CServerInterface::VisualInfo temp;
		RefRecvMsg tempRefMsg;
		if(!IS_SIMULATION){		// ʵ��
			//if (VisionReceiver::instance()->_socket->receiveData(reinterpret_cast<char*> (_visionMessage), sizeof(Message)) <= 0) {
			//	break;
			//}
			vision_semaphore.Wait();
			(VisionReceiver::_visionMessage)->message2VisionInfo(temp);	// ͼ������߳�ת��ͼ������
			//cout << "has receive data" << endl;
			temp.mode = _referee->getPlayMode();		// ���к���������̵߳õ����к�ָ��,��������Ӧmode
			tempRefMsg.blueGoal = _referee->getBlueGoalNum();
			tempRefMsg.yellowGoal = _referee->getYellowGoalNum();
			tempRefMsg.timeRemain = _referee->getRemainTime();
			tempRefMsg.blueGoalie = _referee->getBlueGoalie();
			tempRefMsg.yellowGoalie = _referee->getYellowGoalie();
		} else {		// ����
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
				
		decisionMutex->lock();		// ���߼���

		visionMutex->lock();		// ͼ�����
		VisionReceiver::instance()->_info = temp;
		VisionReceiver::instance()->_recvMsg = tempRefMsg;
		VisionReceiver::instance()->_info.cycle = ++cycle;  // ��ʱ��ô���棬fix me!!! û���õ�ͼ�������cycle,���ǲ����Լ���¼cycle,�յ�һ��ͼ����1;

		// ����ͼ����ܼ��ʱ����
		_usecTimer.stop();
		double delt_time = _usecTimer.time()/1000.0f;
		_usecTimer.start();
		
		if (_log->m_bIsLogging) {
			_log->writeLog(_log->m_pfLog,temp);			
		}

		visionMutex->unlock();		// ͼ�����
		vision_finish_semaphore.Signal();
		if (delt_time < 1000.0/Param::Vision::FRAME_RATE) {
			// ˯�ߣ�ȥ������ͼ�񣬱�֤�ȶ���ͼ��֡��
			Sleep(1000.0/Param::Vision::FRAME_RATE-delt_time);	
			//std::cout<<cycle<<'\t'<<1000.0/Param::Vision::FRAME_RATE-delt_time<<std::endl;
		}
		visionEvent->pulse();

		decisionMutex->unlock();	// ���߽���
	}

	///> �˳�
	VisionReceiver::instance()->_threadAlive = false;
	visionEvent->set();

	return 0;
}

bool VisionReceiver::getVisionInfo(CServerInterface::VisualInfo& info, RefRecvMsg& msg) const
{
	if(_threadAlive){
		visionMutex->lock();		// ͼ�����
		info = this->_info;			// ���õ���ͼ�����������info
		msg  = this->_recvMsg;
		visionMutex->unlock();		// ͼ�����
	}

	return _threadAlive;
}