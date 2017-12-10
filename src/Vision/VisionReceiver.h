/************************************************************************/
/* file created by shengyu, 2005.3.23                                   */
/************************************************************************/
#ifndef __VISION_RECEIVER_H__
#define __VISION_RECEIVER_H__
#include "Network\UDPSocket.h"
#include "Network\Message.h"
#include <MultiThread.h>
#include <singleton.h>

#include <ServerInterface.h>
#include "VisionLog.h"
class COptionModule;
class CCtrlBreakHandler;
class VisionReceiver
{
public:
	static VisionReceiver* instance(COptionModule *pOption=NULL, CCtrlBreakHandler *breakHandler=NULL);	
	static void destruct();
	static Message* _visionMessage;
public:
	bool getVisionInfo(CServerInterface::VisualInfo& info, RefRecvMsg& msg) const;
	static CThreadCreator::CallBackReturnType THREAD_CALLBACK getMessage(CThreadCreator::CallBackParamType lpParam);
private:
	VisionReceiver(COptionModule *pOption, CCtrlBreakHandler *breakHandler);
	~VisionReceiver(void);

	static VisionReceiver* _instance;
	UDPSocket * _socket;
	CServerInterface::VisualInfo _info;
	RefRecvMsg _recvMsg;
	bool _threadAlive;
};

#endif