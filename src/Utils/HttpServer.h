#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

/**********************************************
HttpServer: 向脚本生成器发送图像信息
***********************************************/
#include "MultiThread.h"
#include <string>
#include "singleton.h"

struct to_web_data{
	std::string role[6];
	double x[6];
	double y[6];
	double dir[6];
	double ballx;
	double bally;
	bool end;
};

class CHttpServer{
public:
	CHttpServer();
	~CHttpServer();
	bool parseBallPos(char*, int);
	void setToWebData(char* r, int num);
	void getToWebData(to_web_data&);
	char* getCurPlay();
	void clearCurPlay();
public:
	to_web_data m_to_web_data;
	std::string m_cur_play;
	bool m_end;
};

typedef NormalSingleton< CHttpServer > HttpServer;

#endif