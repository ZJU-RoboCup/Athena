#include <WinSock2.h>
#include <iostream>
#include <fstream>
#include <ServerInterface.h>
#include <weerror.h>
#include <WorldModel.h>
#include "DecisionModule.h"
#include "ActionModule.h"
#include <tinyxml/ParamReader.h>
#include <CtrlBreakHandler.h>
#include <VisionReceiver.h>
#include <TimeCounter.h>
#include <GDebugEngine.h>
#include "bayes/MatchState.h"
#include "gpuBestAlgThread.h"
#include "Global.h"
#include "DefendUtils.h"
#include "Compensate.h"
#include <mutex>
#include "MyThread.h"

/*! \mainpage Zeus - Run for number one
*
* \section Introduction
*
*	ZJUNlict is robot soccer team of Robocup in Small Size League.
*			
* \section Strategy
*	Frame : GameState -> Play -> Agent -> Skill -> Motion Control -> Wireless
*
* \subsection step1: GameState
* \subsection step2: Play
* \subsection step3: Agent
* \subsection step4: Skill
* \subsection step5: Motion Control : PathPlan and Trajectory Generation
* 
* etc...
*/

/// <summary> XML configure file for zues. </summary>
std::string CParamReader::_paramFileName = "zeus2005";

/// <summary> For Synchronize strategy. </summary>
extern CEvent *visionEvent;
extern CMutex *decisionMutex;
/// <summary> For GPU. </summary>
CMutex* _best_visiondata_copy_mutex = 0;
CMutex* _value_getter_mutex = 0;

CUsecTimer _usecTimer;
CUsecTimer _usecTime2;

using Param::Latency::TOTAL_LATED_FRAME;

namespace { 
	// ?¡Â??????????
	bool VERBOSE_MODE = true;
}

/// <summary> The graphical user interface socket </summary>
UDPSocket gui_socket;
UDPSocket gui_recordPos_socket;
int self_port = 20002;
int gui_port = 20001;
int RECORDPOS_PORT =0;
bool IS_SIMULATION = false;
bool gui_pause = false;
bool wireless_off = false;
bool send_debug_msg = false;
bool record_run_pos_on =false;
CThreadCreator* gui_recv_thread = 0;
CThreadCreator* gui_recordPos_thread = 0;
/// <summary>	Executes the graphical user interface send operation. </summary>
///
/// <remarks>	cliffyin, 2011/7/26. </remarks>
///
/// <param name="cycle">	The cycle. </param>

static void do_gui_send(int cycle);

/// <summary>	Executes the graphical user interface recv operation. </summary>
///
/// <remarks>	cliffyin, 2011/7/26. </remarks>

DWORD THREAD_CALLBACK do_gui_recv(LPVOID lpParam);

DWORD THREAD_CALLBACK do_recordPos_recv(LPVOID lpParam);

/**
@brief ???¨°??????????Main entry-point for this application.
@param argc ?¨¹??????????????Number of command-line arguments.
@param argv ?¨¹??????????Array of command-line argument strings.
@retval 0 ??????success!
*/

#ifndef RELEASE_LIB
int main(int argc, char* argv[])
{
	/************************************************************************/
	/* ?¡Â???¨¦??????????                                                     */
	/************************************************************************/

	// ??????????xml????????

	PARAM_READER->readParams();
	
	DECLARE_PARAM_READER_BEGIN(General)
	READ_PARAM(RECORDPOS_PORT)
	READ_PARAM(IS_SIMULATION)
		
	DECLARE_PARAM_READER_END
	
	// ???????¡Â????
	//for (int i = 0; i <= 1000; i++) {
	//	cout << "666" << endl;
	//}
	initializeSingleton();
	
	// ?¨°??????????????????¡¤?¡À??¨¦????????????????
	ofstream file;
	
	file.open("./frequence.txt",ios::out);
	
	// ?????????¨°¡À??????¡À?¨°???¨´??????????,??catch???¡Â??action->stopAll(),?¨´????ActionModule???¡§????¡¤???????
	// TODO ???????????¡Â??
	// ?¨¹?????????????¨¦
	CCtrlBreakHandler breakHandler;

	// ???¨°???????¨¦
	COptionModule option(argc,argv);

	// ???¨°???????¨¦
	vision->registerOption(&option);

	// ???¨°???????¨¦
	CDecisionModule decision(&option, vision);

	// ???¨°??¡Á¡Â???¨¦
	CActionModule action(&option, vision, &decision);

	// ????????¡Á??¨¢???????¨¦
	WORLD_MODEL->registerVision(vision);

	// ¡À?????¡À??¨¹?????????¡Â??????
	MATCH_STATE->initialize(&option,vision);

	/************************************************************************/
	/* ???? Main Loop  ¡À??¡¤????                                              */
	/************************************************************************/
	// ?????¡Â?¨°????¡¤?
	try {
		// ??????????????????
		_best_visiondata_copy_mutex = new CMutex; 
		_value_getter_mutex = new CMutex;
		GPUBestAlgThread::Instance()->initialize(VISION_MODULE);


		// ?????? ZjuNlictClient ?¡Â????¡ã??¡§???? Socket
		gui_socket.init(self_port);

		// GUI??????????
		gui_recv_thread = new CThreadCreator(do_gui_recv, 0);

		// DefendDebug????
		if (RECORDPOS_PORT)
		{
			gui_recordPos_socket.init(RECORDPOS_PORT);
			gui_recordPos_thread = new CThreadCreator(do_recordPos_recv, 0);
			
		}
	
		// ???????????¨¦
		const VisionReceiver *receiver = VisionReceiver::instance(&option, &breakHandler);
		if (!IS_SIMULATION) {
			const MyThread *mt = new MyThread();
		}
		// ???????????¨®
		CServerInterface::VisualInfo visionInfo;

		//if (!IS_SIMULATION) {
		//	std::thread t2(startMediator);//start thread
		//}

		// ¡ã¨¹??¡À?¡¤?????????????
		RefRecvMsg refRecvMsg;

		// ???????????¨¨??????????????
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

		// ???¨°????¡À¨º????
		bool stop = false;

		double usedtime = 0.0f;
		double totalusedtime = 0.0f;

		// ?¡Â???¡¤?????¨¬???¨¹????????¡Á¡Â

		while (breakHandler.breaked() < 1) {

			usedtime = 0.0f;
			totalusedtime = 0.0f;

			// ??¡¤???/????????????????		
			visionEvent->wait();
			// [2010-02-18 yph ????]
			decisionMutex->lock();

			// ????????????
			if (! receiver->getVisionInfo(visionInfo, refRecvMsg)) {
				std::cout << "no vision input" << std::endl;
				stop = true;
			}

			// ????????????????????¡¤?¡À??¨¦??
			if (VERBOSE_MODE) {
				if (visionInfo.cycle % 2 ==0) {
					_usecTime2.start();
				} else {
					_usecTime2.stop();					
					if (file.is_open()) {
						file<<" Frequence : "<<visionInfo.cycle<<" : "<<_usecTime2.time()/1000.0f<<"\n";
					}						
				}
				_usecTimer.start();
				vision->SetRefRecvMsg(refRecvMsg);
				vision->SetNewVision(visionInfo);
				//lua_getglobal(L, "execute");
				//tolua_pushusertype(L, this, "CVisionModule");
				//lua_call(L,1,0);
				_usecTimer.stop();

				if (visionInfo.cycle%10 == 0) {
					usedtime = _usecTimer.time()/1000.0f;
					totalusedtime += usedtime;
					//std::cout<<" VisionModule : "<<visionInfo.cycle<<" : "<<usedtime<<"\n";
				}
			} else {
				// ?¨¹?????????¨¦???????¨¨¡¤??????????????????¨¦¡Á?¡Á?¡À?
				vision->SetRefRecvMsg(refRecvMsg);
				vision->SetNewVision(visionInfo);
			}

			// ?¨¹???¨¢?????¨°???¨ª
			if (0 == breakHandler.breaked()) {
				stop = true;
			}


			// ?¡Â????¡ã????????¨ª
			if (breakHandler.halted() || gui_pause) {
				decision.DoDecision(true);
			} else {
				// ???????????????????¨¨¡¤???????????????????????????
				//cout << "2 : dodecision " << endl;
				decision.DoDecision(stop);
			}

			// ?????¨¹???¨²?????????¨¨¡¤??????????????????¨®???????¨²????¡¤???
			if (! wireless_off) {
				action.sendAction(visionInfo.ourRobotIndex);
			} else {
				action.sendNoAction(visionInfo.ourRobotIndex);
			}

			// [2010-02-18 yph ????]
			decisionMutex->unlock();	

			/************************************************************************/
			/* GUI ?¡Â???¡§??                                                         */
			/************************************************************************/
			do_gui_send(visionInfo.cycle);

			// ???¨°???????¨ª ??100ms?????????¨°???¡¤¡À?stop????¡¤???
			if (stop) {
				Sleep(100);
				VisionReceiver::destruct();
				break;
			}
		}
	} catch(WEError e) {
		// ???¨°?¨¬????????????
		action.stopAll();
		e.print();
		std::cout << "Usage : " << argv[0] << " [s(l|r)] [t(1|2)] [c(y|b)] [n(1..5)]" << Param::Output::NewLineCharacter;
		std::cout << "Press any key to exit." << Param::Output::NewLineCharacter;
		getchar();
	}

	/************************************************************************/
	/* ???¨°????                                                             */
	/************************************************************************/
	// ??¡À??¨°????????????????
	if (file.is_open()) {
		file.close();
	}

	// ????????
	action.stopAll();
	std::cout << "system exits, press any key to continue..." << std::endl;
	getchar();

	return 0;
}

#endif
/**
@brief ¡¤??????????¡Â??????????????¡ã??¡§???¡ì??ZjuNlictClient)
@param cycle ¡À¨º???¡À?¡ã???¡À??????
*/
static void do_gui_send(int cycle)
{
	// ?????¡ì?????????¡À?¨°??¡¤?????¡¤??¨°????¡¤???????
	if (send_debug_msg) {
		net_gdebugs new_msgs;
		new_msgs.totalnum = 0;
		new_msgs.totalsize = 0;
		memset(new_msgs.msg, 0, 500*sizeof(net_gdebug));

		// ?????¨®??UDP¡ã?¡À?????????¡ã¨¹
		while(!GDebugEngine::Instance()->is_empty()){
			net_gdebug new_msg = GDebugEngine::Instance()->get_queue_front();
			new_msgs.totalsize = vision->Cycle();
			new_msgs.msg[new_msgs.totalnum] = new_msg;
			GDebugEngine::Instance()->pop_front();
			new_msgs.totalnum++;
			if(new_msgs.totalnum >= 500){
				cout<<"!!!!!!!!!To Many Debug Message To Client!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
			}
		}
		gui_socket.sendData(gui_port, (char*)(&new_msgs), 8+sizeof(net_gdebug)*new_msgs.totalnum, "127.0.0.1");
	} else {
		while ( !GDebugEngine::Instance()->is_empty() ) {
			GDebugEngine::Instance()->pop_front();
		}
	}

	return;
}

/**
@brief ??????????¡ã????????¨¹???¡§???¡ì??ZjuNlictClient)
*/
DWORD THREAD_CALLBACK do_gui_recv(LPVOID lpParam)
{
	while (true) {
		char msg[net_gui_in_maxsize];
		if (gui_socket.receiveData(msg, net_gui_in_maxsize)) {
			net_gcommand* ngc = (net_gcommand*)msg;
			switch (ngc->msgtype) 
			{
			case NET_GUI_CONTROL:
				{
					if ( strcmp(ngc->cmd, "r") == 0 )
						gui_pause = false;
					else if ( strcmp(ngc->cmd, "p") == 0 )
						gui_pause = true;
					else if ( strcmp(ngc->cmd, "wireless_on") == 0 )
						wireless_off = false;
					else if ( strcmp(ngc->cmd, "wireless_off") == 0 )
						wireless_off = true;
					else if ( strcmp(ngc->cmd, "strategy_on") == 0 )
						send_debug_msg = true;
					else if ( strcmp(ngc->cmd, "strategy_off") == 0 )
						send_debug_msg = false;
					break;
				}
			default:
				break;
			}
		}
		Sleep(100);
	}
}


DWORD THREAD_CALLBACK do_recordPos_recv(LPVOID lpParam)
{
	while (true) {
		char msg[net_gui_in_maxsize];
		if (gui_recordPos_socket.receiveData(msg, net_gui_in_maxsize)) {
			net_gcommand* ngc = (net_gcommand*)msg;
			switch (ngc->msgtype) 
			{
			case NET_GUI_CONTROL:
				{
					if ( strcmp(ngc->cmd, "record_run_pos_on") == 0 )
						record_run_pos_on = true;
					else if ( strcmp(ngc->cmd, "record_run_pos_off") == 0 )
						record_run_pos_on = false;
					break;
				}
			default:
				break;
			}
		}
		Sleep(100);
	}
}