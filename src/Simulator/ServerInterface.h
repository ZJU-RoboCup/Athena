#ifndef _SERVER_INTERFACE_H_
#define _SERVER_INTERFACE_H_
#include <server.h>
/************************************************************************/
/*                      ��Server��ͨѶ�ӿ�                              */
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
	bool Pause(int status);///<��ͣ����
	bool PlayerAct(int num, short type, short value1, short value2, short value3);///<ִ�ж���
	bool GetVisualInfo(VisualInfoT* pinfo);///<��ȡ�Ӿ���Ϣ
	bool MoveBall(double x, double y);
};
#endif //_SERVER_INTERFACE_H_
