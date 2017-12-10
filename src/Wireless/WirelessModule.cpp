/// FileName : 		WirelessModule.cpp
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

#include ".\wirelessmodule.h"

CWirelessModule::CWirelessModule(void)
{
	pCommControl = NULL;
	pCommControl = new CCommControl();
	pCommControl->Init();
}

CWirelessModule::~CWirelessModule(void)
{
	pCommControl->Destroy();
}

void CWirelessModule::doWirelessModule(int index, const ROBOTCOMMAND& robotCommand)
{
	pCommControl->NewRobotCommand(index, &robotCommand);
}

CCommControl* CWirelessModule::getControlComm() const
{
	return pCommControl;
}