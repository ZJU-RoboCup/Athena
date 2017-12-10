#pragma once

#include "proto/cpp/messages_robocup_ssl_detection.pb.h"
#include "proto/cpp/messages_robocup_ssl_geometry.pb.h"
#include "proto/cpp/messages_robocup_ssl_wrapper.pb.h"
#include "net/robocup_ssl_client.h"
#include "utils\winsocket\Socket.h"
#include "Transmit.h"
#include <MultiThread.h>
#include "UDPSocket.h"
class MyThread
{
public:
	MyThread();
	~MyThread();
	static CThreadCreator::CallBackReturnType THREAD_CALLBACK run(CThreadCreator::CallBackParamType lpParam);
	//void setCameraMode(int);
	void setBasicConfig();
	void setTransmitIPAndPort(std::string ip, u_short port)
	{
		iplist.push_back(ip);
		portlist.push_back(port);
	}
	void setMultiCastIPAndPort(string net_ref_address, int port) { client.setAddress(net_ref_address); client.setPort(port);}
	int openClient() { return client.open(true); }
	void setFirstStart() { transmit.setFirstStart(); }
	//void setPort(int port) { client.setPort(port); }
	//void MyThread::setOriginMsg();
private:
	static UDPSocket * _log_socket;
private:
	static void setCameraId(int);
	static bool doCameraUpdated();
	static void doTransmit();

private:
	static RoboCupSSLClient client;
	static SSL_WrapperPacket packet;
	static Socket server;
	static SendVisionMessage  transmit_msg;
	static Transmit transmit;
	static int newCameraID;
	static int cameraMode;
	static bool camReceive[4];
	static bool camOnline[4];
	static int RECEIVE_X_MIN;
	static int RECEIVE_X_MAX;
	static int RECEIVE_Y_MIN;
	static int RECEIVE_Y_MAX;
	static int MIN_ADD_FRAME;
	static int MIN_LOST_FRAME;
	static int _cycle;
	static int SLEEP_TIME;
	static int FOLLOW_CHECK_CYCLE;
	static bool M_SEND;
	static float maxBallDist;
	static float maxVehicleDist;
	static float CALI_ERROR;
	static string TRANSMIT_IP;
	static u_short TRANSMIT_PORT;
	static string MULTICAST_IP;
	static u_short MULTICAST_PORT;
	static bool LOG_MODE;
	static std::vector< std::string > iplist;
	static std::vector< u_short> portlist;
};

