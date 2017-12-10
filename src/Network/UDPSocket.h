
#ifndef __UDP_SOCKET_H__
#define __UDP_SOCKET_H__


#define WIN32_LEAN_AND_MEAN //¹Ø¼ü
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

class UDPSocket
{
public:
	UDPSocket(void);
	~UDPSocket(void);
	bool init(u_short port);
	bool initWithMultiCast(u_short port);
	int receiveData( char* message, int size );
	void close();
	bool sendData( u_short port,char* message, int size, char* hostname);
	bool sendData2LastSockaddr(char* message, int size);
	bool sendData2OnlyAddr(char* message, int size, int port);
private:
	SOCKET theSocket;
	sockaddr_in m_last_sockaddr;
};

#endif