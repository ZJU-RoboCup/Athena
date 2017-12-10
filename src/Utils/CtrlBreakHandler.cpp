#include <CtrlBreakHandler.h>
#include <windows.h>
#include <iostream>
#include <param.h>
namespace{
	bool breakPressed = false;
	bool haltPressed = false;
	int countAfterBreak = 0;
	BOOL CtrlHandler(DWORD fdwCtrlType) 
	{ 
		switch (fdwCtrlType){ 
		case CTRL_C_EVENT:
			if(haltPressed){
				std::cout << "resume system" << std::endl;
				haltPressed = false;
			}
			else{
				std::cout << "halt system" << std::endl;
				haltPressed = true;
			}
			return TRUE;
		case CTRL_CLOSE_EVENT:
		case CTRL_BREAK_EVENT:
			breakPressed = true;
			return TRUE; 
		case CTRL_LOGOFF_EVENT: 
		case CTRL_SHUTDOWN_EVENT: 
		default: 
			return FALSE; 
		} 
	} 
}
CCtrlBreakHandler::CCtrlBreakHandler()
{
	if (!SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler,TRUE)){
		std::cout << "Could not set control handler" << Param::Output::NewLineCharacter;
	}
}
int CCtrlBreakHandler::breaked()
{
	if(!breakPressed && countAfterBreak==0)		// 未按下ctrl+Break
		return -1;
	else
		++countAfterBreak;
	if(countAfterBreak <= 1) // 按下ctrl+Break后第一次
		return 0;
	else
		return 1;
}

bool CCtrlBreakHandler::halted()
{
	return haltPressed;
}