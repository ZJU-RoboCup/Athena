#ifndef _SERVER_INTERFACE_H_
#define _SERVER_INTERFACE_H_
#include <server.h>
/************************************************************************/
/*                      和Server的通讯接口                              */
/************************************************************************/
struct RefRecvMsg{
	int blueGoal;
	int yellowGoal;
	int timeRemain;
	int blueGoalie;
	int yellowGoalie;
};
class COptionModule;
class CServerInterface{
public:
	typedef VisualInfoT VisualInfo;
	CServerInterface(const COptionModule* pOption);
	~CServerInterface();
	bool Pause(int status);///<暂停更新
	bool PlayerAct(int num, short type, short value1, short value2, short value3);///<执行动作
	bool GetVisualInfo(VisualInfoT* pinfo);///<获取视觉信息
	bool MoveBall(double x, double y);
};
#endif //_SERVER_INTERFACE_H_
