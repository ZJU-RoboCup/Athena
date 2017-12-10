#include "Socket.h"
#include "Vision\mediator\utils\winsocket\UDPSocket_vm.h"

#include <iostream>

namespace{
	static bool WinsockInitialized = false;
	//static const u_short PORT = 12345;
}

UDPSocket_vm::UDPSocket_vm()
{
	// Initialize WinSock
	if (!WinsockInitialized) {
		WSAData wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);

		WinsockInitialized = true;
	}

	theSocket = NULL;
}

UDPSocket_vm::~UDPSocket_vm(void)
{
	closesocket(theSocket);
}

bool UDPSocket_vm::init(u_short port)
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

	int res = bind(theSocket,(SOCKADDR*)&RecvAddr,sizeof(RecvAddr));
	if ( res == SOCKET_ERROR )
	{
		printf("error!\n");
		return false;
	}

	return true;
}


bool UDPSocket_vm::initWithMultiCast(u_short port)
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

	int res = bind(theSocket,(SOCKADDR*)&RecvAddr,sizeof(RecvAddr));

	if ( res == SOCKET_ERROR )
	{
		printf("error!\n");
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
		printf("udp error!\n");
		return false;
	}

	return true;

}

int UDPSocket_vm::receiveData(char* message, int size)
{
	sockaddr_in SenderAddr;
	int SenderAddrSize = sizeof(SenderAddr);
	int result = -1;
	
	result = recvfrom(theSocket, message, size, 0, (SOCKADDR *)&SenderAddr, &SenderAddrSize);
	//std::cerr << "result: " << result << std::endl;
	return result;
}

int UDPSocket_vm::sendData( char* message, int size )
{
	sockaddr_in SenderAddr;
	int SenderAddrSize = sizeof(SenderAddr);
	int result = -1;

	result = sendto(theSocket, message, size, 0, (SOCKADDR *)&SenderAddr, SenderAddrSize);
	//std::cerr << "result: " << result << std::endl;
	return result;
}