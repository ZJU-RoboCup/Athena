/************************************************************************/
/* file created by shengyu, 2005.3.25                                   */
/************************************************************************/
#ifndef __UDP_SOCKET_H__
#define __UDP_SOCKET_H__

// #include <winsock.h>
//#include <winsock2.h>
#include <windows.h>
//#include <WS2tcpip.h>

class UDPSocket_vm
{
public:
	UDPSocket_vm(void);
	~UDPSocket_vm(void);
	bool init(u_short port);
	bool initWithMultiCast(u_short port);
	int receiveData( char* message, int size );
	int sendData( char* message, int size );
private:
	SOCKET theSocket;
};

#endif