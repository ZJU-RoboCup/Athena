#ifndef _COMMAND_FACTORY_H_
#define _COMMAND_FACTORY_H_
#include "PlayerCommandV2.h"
#include <singleton.h>
/**
* CCommandFactory类.
* 用来存放各种Command的实例
*/
class CCommandFactory{
public:
	template < class T >
	T* newCommand(const T& cmd)
	{
		static T* pCmds[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		if( pCmds[cmd.number()] == 0 ){
			pCmds[cmd.number()] = new T(cmd);
		}else{
			*pCmds[cmd.number()] = cmd;
		}
		return pCmds[cmd.number()];
	}
};
typedef NormalSingleton< CCommandFactory > CmdFactory;
#endif
