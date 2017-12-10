#include "Network\UDPSocket.h"
#include "HttpServer.h"
#include <process.h>
#include "Simulator.h"
#include "Global.h"
#include <map>
#include <string>
namespace{
	CMutex buf_mut;
	CMutex play_mut;
	UDPSocket udp_sock;
	int recv_port = 41238;	//http发送socket端口
	int send_port = 41239;	
	volatile bool send_start = false;
	std::map<std::string, int> role2num;
}
 
unsigned __stdcall ReceiveWebDataThread(void *p)
{
	CHttpServer *pServer;
	pServer = (CHttpServer*)p;

	udp_sock.init(recv_port);

	char buff[5000];
	int nbuff;
	
	while(true) {
		memset(buff,0,4999);
		nbuff=udp_sock.receiveData(buff,4999);
		if(nbuff > 0){
			buff[nbuff]='/0';
			if(pServer->parseBallPos(buff,nbuff+1)){
				pServer->m_end = false;
				send_start = true;
			}
		}
		Sleep(10);
	}
	_endthreadex( 0 );
	return 0;
}

unsigned __stdcall SendWebDataThread(void *p)
{
	CHttpServer *pServer;
	pServer = (CHttpServer*)p;

	char content[5000];
	char contentFmt[] = "{\"ball\": {\"x\" : %f, \"y\" : %f},"
		"\"%s\": {\"x\" : %f, \"y\" : %f, \"r\" : %f},"
		"\"%s\": {\"x\" : %f, \"y\" : %f, \"r\" : %f},"
		"\"%s\": {\"x\" : %f, \"y\" : %f, \"r\" : %f},"
		"\"%s\": {\"x\" : %f, \"y\" : %f, \"r\" : %f},"
		"\"%s\": {\"x\" : %f, \"y\" : %f, \"r\" : %f},"
		"\"%s\": {\"x\" : %f, \"y\" : %f, \"r\" : %f},"
		"\"End\" : %d}";

	while(true) {
		to_web_data send_data;
		pServer->getToWebData(send_data);

		sprintf(	content,contentFmt,
			send_data.ballx, send_data.bally,
			send_data.role[0].c_str(), send_data.x[0], send_data.y[0],send_data.dir[0]*180/3.1415926 + 360,
			send_data.role[1].c_str(), send_data.x[1], send_data.y[1],send_data.dir[1]*180/3.1415926 + 360,
			send_data.role[2].c_str(), send_data.x[2], send_data.y[2],send_data.dir[2]*180/3.1415926 + 360,
			send_data.role[3].c_str(), send_data.x[3], send_data.y[3],send_data.dir[3]*180/3.1415926 + 360,
			send_data.role[4].c_str(), send_data.x[4], send_data.y[4],send_data.dir[4]*180/3.1415926 + 360,
			send_data.role[5].c_str(), send_data.x[5], send_data.y[5],send_data.dir[5]*180/3.1415926 + 360,
			pServer->m_end);
		if(send_start){
			udp_sock.sendData2OnlyAddr(content,strlen(content),send_port);
		}
		Sleep(30);
	}
	_endthreadex( 0 );
	return 0;
}


void CHttpServer::setToWebData(char* r, int num)
{
	if(num > 0){
		buf_mut.lock();
		m_to_web_data.x[role2num[r]] = vision->OurPlayer(num).Pos().x();
		m_to_web_data.y[role2num[r]] = vision->OurPlayer(num).Pos().y();
		m_to_web_data.dir[role2num[r]] = vision->OurPlayer(num).Dir();
		m_to_web_data.role[role2num[r]] = r;
		m_to_web_data.ballx = vision->Ball().X();
		m_to_web_data.bally = vision->Ball().Y();
		buf_mut.unlock();
	}
}

void CHttpServer::getToWebData(to_web_data& d)
{
	buf_mut.lock();
	d = m_to_web_data;
	buf_mut.unlock();
}

bool CHttpServer::parseBallPos(char* buf, int len)
{
	char* px = strstr(buf,"ballx=") + 6;
	char* py = strstr(buf,"&bally=");
	char* pname = strstr(buf,"&name=");
	char* pz = strstr(buf,"&ballz=");
	if(px == NULL || py == NULL){
		return false;
	} else{
		double x, y;
		char x_value[10];
		char y_value[10];
		memcpy(x_value,px,py-px);
		x = atof(x_value);
		memcpy(y_value,py+7,pname-py-7);
		y = atof(y_value);
		char curplay[30];
		memcpy(curplay,pname+6,pz-pname-6);
		play_mut.lock();
		m_cur_play = curplay;
		play_mut.unlock();
		std::cout<<x<<" "<<y<<std::endl;
		CSimulator::instance()->MoveBall(x,y);
		return true;
	}
}
//ballx=280&bally=150&name=Ref_CornerKickV100&ballz=423
char* CHttpServer::getCurPlay()
{
	string p;
	p.clear();
	play_mut.lock();
	p = m_cur_play;
	play_mut.unlock();
	return (char*)m_cur_play.c_str();
}

void CHttpServer::clearCurPlay()
{
	play_mut.lock();
	m_cur_play.clear();
	play_mut.unlock();
	m_end = true;
}

CHttpServer::CHttpServer()
{
	role2num.insert(std::make_pair("Leader",0));
	role2num.insert(std::make_pair("Assister",1));
	role2num.insert(std::make_pair("Special",2));
	role2num.insert(std::make_pair("Defender",3));
	role2num.insert(std::make_pair("Middle",4));
	role2num.insert(std::make_pair("Goalie",5));

	m_to_web_data.role[0] = "Leader";
	m_to_web_data.role[1] = "Assister";
	m_to_web_data.role[2] = "Special";
	m_to_web_data.role[3] = "Defender";
	m_to_web_data.role[4] = "Middle";
	m_to_web_data.role[5] = "Goalie";

	for(int i = 0; i < 6; i++){
		m_to_web_data.x[i] = 0;
		m_to_web_data.y[i] = 0;
		m_to_web_data.dir[i] = 0;
		m_to_web_data.ballx = 0;
		m_to_web_data.bally = 0;
		m_to_web_data.end = false;
	}
	m_end = false;
	unsigned int _threadID1,_threadID2;
	m_cur_play.clear();
	HANDLE hThread1 = (HANDLE)_beginthreadex( NULL, 0, &ReceiveWebDataThread, this, 0, &_threadID1 );
	HANDLE hThread2 = (HANDLE)_beginthreadex( NULL, 0, &SendWebDataThread, this, 0, &_threadID2 );

}

CHttpServer::~CHttpServer()
{

}