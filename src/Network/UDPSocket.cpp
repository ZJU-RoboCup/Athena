#include ".\udpsocket.h"
#include <iostream>
namespace{
	bool WinsockInitialized = false;
}

UDPSocket::UDPSocket()
{
	// Initialize WinSock
	if (!WinsockInitialized) {
		WSAData wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);

		WinsockInitialized = true;
	}

	theSocket = NULL;
}

UDPSocket::~UDPSocket(void)
{
	closesocket(theSocket);
	WSACleanup();
}

bool UDPSocket::init(u_short port)
{
//  	const char* multiCastAddr = "224.5.23.1";
//  	const int multiCastPort = 10001;
//  	sockaddr_in target;
// 	SOCKET multiSock; 
// 
// 	theSocket = WSASocket(AF_INET,SOCK_DGRAM,0,NULL,0,WSA_FLAG_MULTIPOINT_D_LEAF | WSA_FLAG_MULTIPOINT_C_LEAF
// 		| WSA_FLAG_OVERLAPPED);

	sockaddr_in RecvAddr;
	RecvAddr.sin_family = AF_INET;
	RecvAddr.sin_port = htons(port);
	RecvAddr.sin_addr.s_addr = (INADDR_ANY);
	theSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	linger InternalLinger;
	InternalLinger.l_onoff=1;
	InternalLinger.l_linger=0;
	setsockopt(theSocket,SOL_SOCKET,SO_LINGER,(const char*)&InternalLinger,sizeof(linger));

	int res = bind(theSocket,(SOCKADDR*)&RecvAddr,sizeof(RecvAddr));
	if ( res == SOCKET_ERROR )
	{
		printf("socket error!\n");
		return false;
	}

	return true;
}


bool UDPSocket::initWithMultiCast(u_short port)
{
	//将windows.h中#inclue<winsock.h>改为#include<winsock2.h>
	const char* multiCastAddr = "224.5.23.1";
	const u_short multiCastPort = port;
	sockaddr_in target;
	SOCKET multiSock; 

	theSocket = WSASocket(AF_INET,SOCK_DGRAM,0,NULL,0,WSA_FLAG_MULTIPOINT_D_LEAF | 
		WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_OVERLAPPED);

	sockaddr_in RecvAddr;
	RecvAddr.sin_family = AF_INET;
	RecvAddr.sin_port = htons(multiCastPort);
	RecvAddr.sin_addr.s_addr = (INADDR_ANY);

	linger InternalLinger;
	InternalLinger.l_onoff=1;
	InternalLinger.l_linger=0;
	setsockopt(theSocket,SOL_SOCKET,SO_LINGER,(const char*)&InternalLinger,sizeof(linger));

	int res = bind(theSocket,(SOCKADDR*)&RecvAddr,sizeof(RecvAddr));

	if ( res == SOCKET_ERROR )
	{
		printf("multi cast error!\n");
		return false;
	}
	target.sin_family = AF_INET;
	target.sin_port = multiCastPort;
	target.sin_addr.s_addr = inet_addr(multiCastAddr);

	printf("port:%d\n",port);

	multiSock = WSAJoinLeaf(theSocket,(SOCKADDR*)&target,sizeof(target),NULL
		,NULL,NULL,NULL,JL_BOTH);

	if ( multiSock == INVALID_SOCKET )
	{
		printf("udp multi cast error!\n");
		return false;
	}

	return true;
}

int UDPSocket::receiveData(char* message, int size)
{
	sockaddr_in SenderAddr;
	int SenderAddrSize = sizeof(SenderAddr);
	int result = -1;
	
	result = recvfrom(theSocket, message, size, 0, (SOCKADDR *)&SenderAddr, &SenderAddrSize);
	m_last_sockaddr = SenderAddr;
	return result;
}

bool UDPSocket::sendData2LastSockaddr(char* message, int size)
{
	sockaddr_in SenderAddr;
	SenderAddr.sin_family = AF_INET;
	SenderAddr.sin_port = m_last_sockaddr.sin_port;
	SenderAddr.sin_addr.s_addr = m_last_sockaddr.sin_addr.s_addr;
	int SenderAddrSize = sizeof(SenderAddr);
	int result = -1;
	result = sendto(theSocket, message, size, 0, (SOCKADDR*)&SenderAddr, SenderAddrSize);
	if( result == -1 ) {
		std::cerr << "Send Error! " << result << std::endl;
		return false;
	}
	return true;
}

bool UDPSocket::sendData2OnlyAddr(char* message, int size, int port)
{
	sockaddr_in SenderAddr;
	SenderAddr.sin_family = AF_INET;
	SenderAddr.sin_port = htons(port);
	SenderAddr.sin_addr.s_addr = m_last_sockaddr.sin_addr.s_addr;
	int SenderAddrSize = sizeof(SenderAddr);
	int result = -1;
	result = sendto(theSocket, message, size, 0, (SOCKADDR*)&SenderAddr, SenderAddrSize);
	if( result == -1 ) {
		std::cerr << "Send Error! " << result << std::endl;
		return false;
	}
	return true;
}

bool UDPSocket::sendData( u_short port,char* message, int size, char* hostname)
{
	sockaddr_in SenderAddr;
	SenderAddr.sin_family = AF_INET;
	SenderAddr.sin_port = htons(port);
	SenderAddr.sin_addr.s_addr = inet_addr(hostname);
	int SenderAddrSize = sizeof(SenderAddr);
	int result = -1;

	result = sendto(theSocket, message, size, 0, (SOCKADDR*)&SenderAddr, SenderAddrSize);
	if( result == -1 )
	{
		std::cerr << "Send Error! " << result << std::endl;
		return false;
	}
	//std::cout<<"byte: "<<result<<std::endl;
	return true;
}

void UDPSocket::close()
{
	closesocket(theSocket);
}