#ifndef NETWORK_UDP_SOCKET_H_
#define NETWORK_UDP_SOCKET_H_
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <vector>

class Socket
{
public:
	enum { NotConnected = -1,  // Never connected.
		Disconnected = 0,   // Connection was dropped.
		ConnectedAsClient       = 1,   // Connected as a client.
		ConnectedAsServer       = 2    // Connected as a server.
	};

	enum { UDP, TCP };

	Socket();
	~Socket();
	// UDP
	bool init_udp(u_short port, bool bind_needed = true);
	void close_udp();
	bool sendData_udp( u_short port,char* message, int size, char* hostname);
	int receiveData_udp( char* message, int size );
	// TCP
	// Client Function
	bool openConnection( char* hostname, int port ); 
	void setBuffer(int bufSize);
	bool isOpen();
	void close();
	bool sendData( char* message, int size );
	bool receiveData_tcp( char* message, int size );

	// Server Function
	bool openListener( int port );   
	bool readyForAccept();
	void acceptConnection();
	bool readyForSend();
	bool readyForRecv(); // 也可用于Client
private:
	void showError(int code);
	SOCKET _udpSocket;
	SOCKET _tcpSocket;
	bool _is_tcp_connected;
	int _status;
	int _protocol;

	std::vector< SOCKET > _tcp_clients;
};

#endif