#include ".\simulator.h"
#include <ServerInterface.h>
#include <OptionModule.h>

CSimulator* CSimulator::_instance = 0;

CSimulator::CSimulator(const COptionModule *pOption)
{
	_pServerIf = new CServerInterface(pOption);
}

CSimulator::~CSimulator(void)
{
	delete _pServerIf;
}

CSimulator* CSimulator::instance(const COptionModule *pOption)
{
	if(_instance == 0)
		_instance = new CSimulator(pOption);
	return _instance;
}

void CSimulator::destruct()
{
	if(_instance)
		delete _instance;
}