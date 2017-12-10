#include "MyThread.h"
#include "net\message.h"
#include "iostream"
#include <windows.h>
#include "VisionReceiver.h"
#include <tinyxml/ParamReader.h>
#include "Semaphore.h"
Semaphore vision_semaphore(0);
std::string CParamReader::_paramFileName = "zeus2005";

RoboCupSSLClient MyThread::client;
SSL_WrapperPacket MyThread::packet;
Socket MyThread::server;
SendVisionMessage MyThread::transmit_msg;
Transmit MyThread::transmit;
int MyThread::newCameraID;
int MyThread::cameraMode;
bool MyThread::camReceive[4];
bool MyThread::camOnline[4];
int MyThread::MIN_ADD_FRAME;
int MyThread::MIN_LOST_FRAME;
int MyThread::_cycle;
int MyThread::SLEEP_TIME;
int MyThread::FOLLOW_CHECK_CYCLE;
int MyThread::RECEIVE_X_MIN;
int MyThread::RECEIVE_X_MAX;
int MyThread::RECEIVE_Y_MIN;
int MyThread::RECEIVE_Y_MAX;
bool MyThread::M_SEND;
bool MyThread::LOG_MODE;
float MyThread::maxBallDist;
float MyThread::maxVehicleDist;
float MyThread::CALI_ERROR;
std::string MyThread::TRANSMIT_IP;
u_short MyThread::TRANSMIT_PORT;
std::string MyThread::MULTICAST_IP;
u_short MyThread::MULTICAST_PORT;
std::vector< std::string > MyThread::iplist;
std::vector< u_short> MyThread::portlist;
UDPSocket * MyThread::_log_socket = nullptr;
namespace {
	CThreadCreator *_thread = 0;
}

MyThread::MyThread(){
	server.init_udp(54320);
	for (int i = 0; i < 4; i++) {
		camReceive[i] = false;
		camOnline[i] = false;
	}

	_cycle = 0;
	setBasicConfig();
	if (LOG_MODE) {
		_log_socket = new UDPSocket;
		_log_socket->init(12345);
	}
	switch (openClient()) {
	case 0:
		cout << "Error! 不能打开UDP端口" << endl;
		break;
	case 1:
		cout << "Error! 不能建立UDP多播" << endl;
		break;
	default:
		break;
	}
	setFirstStart();
	cout << "Start Transmitting" << endl;
	_thread = new CThreadCreator(run, 0);
}


MyThread::~MyThread()
{
	client.close();
	cout << "stop transmitting" << endl;
	if (_thread)
		delete _thread;
	if (LOG_MODE && _log_socket)
		delete _log_socket;
}

CThreadCreator::CallBackReturnType THREAD_CALLBACK MyThread::run(CThreadCreator::CallBackParamType lpParam)
{
	int i;
	if (LOG_MODE) {
		cout << "\n---------------------------\nLOG_MODE = 1 !!!!!!!!!!!!!!\n---------------------------\n\n" << endl;
		while (true) {
			if (MyThread::_log_socket->receiveData(reinterpret_cast<char*> (&transmit_msg), sizeof(Message)) <= 0) {
				break;
			}
			vision_semaphore.Signal();
			memcpy(VisionReceiver::instance()->_visionMessage, &transmit_msg, sizeof(SendVisionMessage));
			Sleep(SLEEP_TIME);
		}
	}
	else {
		while (true) {
			if (client.receive(packet)) {
				if (packet.has_detection()) {
					auto& detection = packet.detection();
					setCameraId(detection.camera_id());
					int balls_n = detection.balls_size();
					int robots_blue_n = detection.robots_blue_size();
					int robots_yellow_n = detection.robots_yellow_size();
					//Ball info:
					//SSL-Vision可以设置最多发几个球，需保持一致
					int count = 0;
					for (i = 0; i < balls_n; i++) {
						auto& ball = detection.balls(i);
						if (ball.x() >= RECEIVE_X_MIN && ball.x() <= RECEIVE_X_MAX && ball.y() >= RECEIVE_Y_MIN && ball.y() <= RECEIVE_Y_MAX) {
							GlobalData::Instance()->setBall(newCameraID, count, ball.x(), ball.y(), ball.confidence());
							count++;
						}
					}

					//Blue robot info
					count = 0;
					//cout << "cameraID:" << detection.camera_id() << endl;
					for (i = 0; i < robots_blue_n; i++) {
						auto& robot = detection.robots_blue(i);
						if (robot.x() >= RECEIVE_X_MIN && robot.x() <= RECEIVE_X_MAX && robot.y() >= RECEIVE_Y_MIN && robot.y() <= RECEIVE_Y_MAX) {
							GlobalData::Instance()->setBlueRobot(newCameraID, count, robot.robot_id(), robot.x(), robot.y(), robot.orientation(), robot.confidence());
							count++;
						}
					}
					count = 0;
					for (i = 0; i < robots_yellow_n; i++) {
						auto& robot = detection.robots_yellow(i);
						if (robot.x() >= RECEIVE_X_MIN && robot.x() <= RECEIVE_X_MAX && robot.y() >= RECEIVE_Y_MIN && robot.y() <= RECEIVE_Y_MAX) {
							GlobalData::Instance()->setYellowRobot(newCameraID, i, robot.robot_id(), robot.x(), robot.y(), robot.orientation(), robot.confidence());
							count++;
						}
					}
					if (doCameraUpdated())
					{
						if (_cycle > 32766)
							_cycle = 1;
						transmit.setCycle(FOLLOW_CHECK_CYCLE, _cycle++);
						transmit_msg = transmit.smsgUpdate(6, MIN_ADD_FRAME, MIN_LOST_FRAME, maxVehicleDist + CALI_ERROR, M_SEND);
						memcpy(VisionReceiver::instance()->_visionMessage, &transmit_msg, sizeof(SendVisionMessage));
						vision_semaphore.Signal();//
						doTransmit();
						Sleep(SLEEP_TIME);//原来是msleep
					}
				}
			}
		}
	}
	return 0;
}
bool MyThread::doCameraUpdated()
{
	return ((!camOnline[0] ||camReceive[0]) && (!camOnline[1] || camReceive[1]) &&
		   (!camOnline[2] || camReceive[2]) && (!camOnline[3] || camReceive[3]));
}

void MyThread::doTransmit()
{
	for (std::vector< std::string >::const_iterator it1 = iplist.begin();
	it1 != iplist.end(); it1++)
	{
		bool succeed = server.sendData_udp(portlist[it1 - iplist.begin()], (char*)&transmit_msg, sizeof(transmit_msg), (char *)it1->c_str());
	}
	for (int i = 0; i < 4; i++)
		camReceive[i] = false;
}

void MyThread::setCameraId(int cameraID)
{
	//这里ssl发过来的实际id要和场地对应起来
	newCameraID = cameraID;
	camReceive[newCameraID] = true;
}

void MyThread::setBasicConfig()
{
	int MAX_BALL_SPEED;
	int MAX_ROBOT_SPEED;
	int CAMERA_0_ENABLE, CAMERA_1_ENABLE, CAMERA_2_ENABLE, CAMERA_3_ENABLE;
		DECLARE_PARAM_READER_BEGIN(VisionMediator)
		READ_PARAM(MIN_ADD_FRAME)
		READ_PARAM(MIN_LOST_FRAME)
		READ_PARAM(SLEEP_TIME)
		READ_PARAM(FOLLOW_CHECK_CYCLE)
		READ_PARAM(MAX_BALL_SPEED)
		READ_PARAM(MAX_ROBOT_SPEED)
		READ_PARAM(CALI_ERROR)
		READ_PARAM(M_SEND)
		READ_PARAM(RECEIVE_X_MIN)
		READ_PARAM(RECEIVE_X_MAX)
		READ_PARAM(RECEIVE_Y_MIN)
		READ_PARAM(RECEIVE_Y_MAX)
		READ_PARAM(CAMERA_0_ENABLE)
		READ_PARAM(CAMERA_1_ENABLE)
		READ_PARAM(CAMERA_2_ENABLE)
		READ_PARAM(CAMERA_3_ENABLE)
		READ_PARAM(TRANSMIT_IP)
		READ_PARAM(TRANSMIT_PORT)
		READ_PARAM(MULTICAST_IP)
		READ_PARAM(MULTICAST_PORT)
	DECLARE_PARAM_READER_END
	{
		DECLARE_PARAM_READER_BEGIN(General)
			READ_PARAM(LOG_MODE)
		DECLARE_PARAM_READER_END
	}

	maxBallDist = MAX_BALL_SPEED / 60.0 * 1000.0;
	maxVehicleDist = MAX_ROBOT_SPEED / 60.0 * 1000.0;

	camOnline[0] = CAMERA_0_ENABLE;
	camOnline[1] = CAMERA_1_ENABLE;
	camOnline[2] = CAMERA_2_ENABLE;
	camOnline[3] = CAMERA_3_ENABLE;
	iplist.clear();
	portlist.clear();
	setTransmitIPAndPort(TRANSMIT_IP, TRANSMIT_PORT);
	setMultiCastIPAndPort(MULTICAST_IP, MULTICAST_PORT);
}