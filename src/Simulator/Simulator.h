/************************************************************************/
/* file created by shengyu, 2005.04.06                                  */
/************************************************************************/

#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__

#include <singleton.h>
#include <ServerInterface.h>
//#include "./CommandConvertor.h"

class CSimulator
{
private:
	CSimulator(const COptionModule *pOption);
	~CSimulator(void);
public:
	static CSimulator* instance(const COptionModule *pOption=0);
	static void destruct();
	inline void sendCommand(int num, short type, short value1, short value2, short value3)
	{_pServerIf->PlayerAct(num, type, value1, value2, value3);}
	inline bool getVisualInfo(CServerInterface::VisualInfo* info){return _pServerIf->GetVisualInfo(info);}
	inline bool MoveBall(double x, double y){return _pServerIf->MoveBall(x,y);}
private:
	static CSimulator* _instance;
	CServerInterface *_pServerIf;
};

#endif