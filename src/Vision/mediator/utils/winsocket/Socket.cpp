#include "socket.h"
#include <iostream>

#ifdef _DEBUG
#define SOCKET_DEBUG
#endif

//#define DISABLE_NAGLE_ALGORITHM

#define USE_RECEIVE_TIMEOUT

namespace{
	static bool WinsockInitialized = false;
	//static const u_short PORT = 12345;
}

Socket::Socket()
{
	// Initialize WinSock
	if (!WinsockInitialized) {
		WSAData wsaData;
		WSAStartup(MAKEWORD(2, 0), &wsaData);

		WinsockInitialized = true;
	}

	_udpSocket = 0;
	_tcpSocket = 0;

	_is_tcp_connected = false;
	_status = NotConnected;
}

Socket::~Socket(void)
{
	close();
	if ( WinsockInitialized ){
		WSACleanup();
		WinsockInitialized = false;
	}
}

void Socket::showError(int code)
{
	switch (code)
	{
	case WSANOTINITIALISED:
		printf("A successful WSAStartup call must occur before using this function.\n");
		break;
	case WSAENETDOWN:
		printf("The network subsystem has failed.\n");
		break;
	case WSAEACCES:
		printf("Attempt to connect datagram socket to broadcast address failed\n");
		break;
	case WSAEADDRINUSE:
		printf("A process on the computer is already bound to the same fully-qualified address\n");
		break;
	case WSAEADDRNOTAVAIL:
		printf("The specified address is not a valid address for this computer.\n");
		break;
	case WSAEFAULT:
		printf("The name or namelen parameter is not a valid part of the user address space\n");
		break;
	case WSAEINPROGRESS:
		printf("A blocking Windows Sockets 1.1 call is in progress.\n");
		break;
	case WSAEINVAL:
		printf("The socket is already bound to an address.\n");
		break;
	case WSAENOBUFS:
		printf("Not enough buffers available, too many connections.\n");
		break;
	case WSAENOTSOCK:
		printf("The descriptor is not a socket.\n");
		break;
	default:
		printf("no erroe\n");
		break;
	}
}

bool Socket::init_udp(u_short port, bool bind_needed)
{
	_protocol = UDP;
	_udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if( _udpSocket == INVALID_SOCKET )
		return false;
	if ( bind_needed ){
		sockaddr_in RecvAddr;
		RecvAddr.sin_family = AF_INET;
		RecvAddr.sin_port = htons(port);
		RecvAddr.sin_addr.s_addr = INADDR_ANY;
		int res = bind(_udpSocket, (SOCKADDR *) &RecvAddr, sizeof(RecvAddr));

		if ( res == SOCKET_ERROR )
		{
			printf("error!\n");
			int err_code = WSAGetLastError();
			showError(err_code);
			return false;
		}

		return true;
	}
	else
		return true;
}
bool Socket::sendData_udp( u_short port,char* message, int size, char* hostname)
{
	sockaddr_in SenderAddr;
	SenderAddr.sin_family = AF_INET;
	SenderAddr.sin_port = htons(port);
	SenderAddr.sin_addr.s_addr = inet_addr(hostname);
	int SenderAddrSize = sizeof(SenderAddr);
	int result = SOCKET_ERROR;

	//std::cout << port << " " << hostname << std::endl;
	result = sendto(_udpSocket, message, size, 0, (SOCKADDR*)&SenderAddr, SenderAddrSize);
	if( result == SOCKET_ERROR )
	{
		//std::cerr << "Send Error! " << result << std::endl;
		wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
		return false;
	}
	//std::cout<<"byte: "<<result<<std::endl;
	return true;
}
int Socket::receiveData_udp(char* message, int size)
{
	sockaddr_in SenderAddr;
	int SenderAddrSize = sizeof(SenderAddr);
	int result = -1;
	
	result = recvfrom(_udpSocket, message, size, 0, (SOCKADDR *)&SenderAddr, &SenderAddrSize);
	//std::cerr << "result: " << result << std::endl;
	return result;
}
bool Socket::openConnection( char* hostname, int port )
{
	_protocol = TCP;
	printf("try to connect server: %s \n", hostname);
	// Find the server's address
	u_long nRemoteAddr = inet_addr(hostname);
	if (nRemoteAddr == INADDR_NONE) {
		// pcHost isn't a dotted IP, so resolve it through DNS
		hostent* pHE = gethostbyname(hostname);
		if (pHE == 0) {
			return false;
		}
		nRemoteAddr = *((u_long*)pHE->h_addr_list[0]);
	}
	if (nRemoteAddr == INADDR_NONE) return false;


	// Create address
	in_addr Address;
	memcpy(&Address, &nRemoteAddr, sizeof(u_long)); 

	// Create a stream theSocket
	_tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_tcpSocket != INVALID_SOCKET) {
		sockaddr_in sinRemote;
		sinRemote.sin_family = AF_INET;
		sinRemote.sin_addr.s_addr = nRemoteAddr;
		sinRemote.sin_port = htons(port);
		if ( connect(_tcpSocket, (sockaddr*)&sinRemote, sizeof(sockaddr_in)) == SOCKET_ERROR ) {
			perror("(connect TCP)");
			_tcpSocket = NULL;
			return false;
		}
	}

#ifdef DISABLE_NAGLE_ALGORITHM
	bool noDelay = true;
	setsockopt(_tcpSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&noDelay, sizeof(bool));
#endif

#ifdef USE_RECEIVE_TIMEOUT
	int timeout = 5000;
	setsockopt(_tcpSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(int));
#endif

	_status = ConnectedAsClient;
	_is_tcp_connected = true;
	return true;
}
bool Socket::openListener(int port)
{
	// Find the server's address
	_protocol = TCP;

	_tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_tcpSocket != INVALID_SOCKET) {

		sockaddr_in sinInterface;
		sinInterface.sin_family = AF_INET;
		sinInterface.sin_addr.s_addr =  htonl(INADDR_ANY);
		sinInterface.sin_port = htons(port);

		// Bind the theSocket and start listening
		if (bind(_tcpSocket, (sockaddr*)&sinInterface, sizeof(sockaddr_in)) != SOCKET_ERROR) {
			if ( listen( _tcpSocket, SOMAXCONN ) == SOCKET_ERROR )
				printf("Error listening on socket.\n");
			else{
				printf("connect socket as a server successfully.\n");
				_status = ConnectedAsServer;
				return true;
			}
		}
	}

	_tcpSocket = 0;
	return false;
}
void Socket::setBuffer(int bufSize)
{

}
bool Socket::isOpen()
{
	return ((_status == ConnectedAsClient) || (_status == ConnectedAsServer));
}
void Socket::close_udp()
{
	closesocket(_udpSocket);
}
void Socket::close()
{
	if ( _status > 0 ) {
		if ( _protocol == TCP && _status == ConnectedAsServer ) {
			for(std::vector<SOCKET>::iterator it = _tcp_clients.begin(); it != _tcp_clients.end(); it++){
				closesocket(*it);
			}
			if ( !_tcp_clients.empty() )
				_tcp_clients.clear();
		}

		closesocket(_tcpSocket);
		_status = Disconnected;
	}
}
bool Socket::sendData( char* message, int size )
{
	int flags = 0;  //MSG_OOB;
	if ( _status <= 0 ) return false;

	int rv = 0;
	// CLIENT SIDE
	// Just write to the fd.  If TCP then write message size first.
	if ( _status == ConnectedAsClient ) {
		send(_tcpSocket, (char*)&size, sizeof(int), flags);
		rv = send(_tcpSocket, message, size, flags);

		if (rv == SOCKET_ERROR) {
			printf("send() failed with error: %d\n", WSAGetLastError());
			return false;
		}
	} 

	// SERVER SIDE TCP
	// Write to the fd for each client, the message size then message.
	else {
		for(unsigned int i=0; i<_tcp_clients.size(); i++) {
			int temp_rv;
			send(_tcp_clients[i],(char*) &size, sizeof(int), flags);
			temp_rv = send(_tcp_clients[i], message, size, flags); 
			if ( rv > temp_rv )
				rv = temp_rv;
		}
		if (rv < 0) { perror("send (server, TCP)"); return false; }
	} 

	return true;
}
bool Socket::receiveData_tcp( char* message, int size )
{
	if ( _status == ConnectedAsServer ){
		// 先确定从那个套接字接收
		static fd_set fdvar;
		int max_fd = 0;
		SOCKET recv_socket;

		FD_ZERO(&fdvar);
		for(unsigned int i=0; i<_tcp_clients.size(); i++) {
			FD_SET(_tcp_clients[i], &fdvar);
			max_fd = max_fd > _tcp_clients[i] ? max_fd : _tcp_clients[i];
		}

		if ( select(max_fd+1, &fdvar, 0, 0, NULL) <= 0 ) return false;

		for(unsigned int i=0; i<_tcp_clients.size(); i++) {
			if ( FD_ISSET(_tcp_clients[i], &fdvar) ) {
				// printf("recv from %d\n",i);
				recv_socket = _tcp_clients[i];
				break;
			}
		}

		// 接收数据长度
		int nbytes;
		int rv;
		rv = recv(recv_socket, (char*)&nbytes, sizeof(int), 0);
		if(rv == 0)
			return false;

		if ( size > nbytes ) size = nbytes;
		rv = recv(recv_socket, message, size, 0);
		return true;
	}
	else{
		int nbytes;
		// 先接收数据长度
		recv(_tcpSocket, (char*)&nbytes, sizeof(int), 0);
		if ( size > nbytes ) size = nbytes;

		return ( recv(_tcpSocket, message, size, 0) > 0 );
	}

	return false;
}
bool Socket::readyForAccept()
{
	if ( _status == ConnectedAsServer && _protocol == TCP ){
		static timeval tv = {0,0};
		static fd_set fdvar;

		FD_ZERO(&fdvar);
		FD_SET(_tcpSocket, &fdvar);

		return ( select(_tcpSocket+1, &fdvar, 0, 0, &tv) > 0 );
	}
	else
		return true;
}
void Socket::acceptConnection()
{
	sockaddr_in sinRemote;
	int nAddrSize = sizeof(sinRemote);

	SOCKET newSocket = accept(_tcpSocket, (sockaddr*)&sinRemote, &nAddrSize);

	if (newSocket != INVALID_SOCKET) {
		_tcp_clients.push_back(newSocket);
		_is_tcp_connected = true;

		printf("accept a client's connection successfully\n");

#ifdef DISABLE_NAGLE_ALGORITHM
		bool noDelay = true;
		setsockopt(newSocket->theSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&noDelay, sizeof(bool));
#endif

	}
	else {
		newSocket = 0;
		_is_tcp_connected = false;
	}
}
bool Socket::readyForSend()
{
	if ( _status == ConnectedAsServer && _protocol == TCP ){
		if ( _tcp_clients.size() == 0 )
			return false;

		static timeval tv = {0,0};
		static fd_set fdvar;
		FD_ZERO(&fdvar);

		int maxfd = 0;
		for (unsigned int i=0; i<_tcp_clients.size(); i++ ){
			FD_SET(_tcp_clients[i], &fdvar);
			if ( _tcp_clients[i] > maxfd )
				maxfd = _tcp_clients[i];
		}

		return ( select(maxfd+1, 0, &fdvar, 0, &tv) == _tcp_clients.size() );
	}

	return true;
}
bool Socket::readyForRecv()
{
	// TODO
	if ( _status == ConnectedAsServer )
		return true;
	else{
		static timeval tv = {0,500};
		static fd_set fdvar;
		FD_ZERO(&fdvar);
		FD_SET(_tcpSocket, &fdvar);

		return (select(0, &fdvar, 0, 0, &tv) > 0);
	}
}