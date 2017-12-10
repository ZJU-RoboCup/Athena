#ifndef NETWORK_GDEBUG_ENGINE_H_
#define NETWORK_GDEBUG_ENGINE_H_

/**********************************************
GDebugEngine: 向图形界面输出消息的接口类
Version 1.0	beta
Author: qiuxingzhong
DATA: 2009.2.10
EMAIL: qxzzju@gmail.com
***********************************************/

#include <net_gui.h>
#include <singleton.h>
#include <geometry.h>
#include <queue>

class CGDebugEngine{
public:
	CGDebugEngine();
	~CGDebugEngine();
	net_gdebug get_queue_front();
	void pop_front();
	bool is_empty()const { return _debugs.empty(); }
	void gui_debug_x(const CGeoPoint& p, char debug_color = 1);
	void gui_debug_line(const CGeoPoint& p1, const CGeoPoint& p2, char debug_color = 1);
	void gui_debug_arc(const CGeoPoint& p, double r, double start_angle, double span_angle, char debug_color = 1);
	void gui_debug_triangle(const CGeoPoint& p1, const CGeoPoint& p2, const CGeoPoint& p3, char debug_color = 1);
	void gui_debug_robot(const CGeoPoint& p, double robot_dir);
	void gui_debug_msg(const CGeoPoint& p, const char* msgstr, char debug_color = 1);
	void gui_debug_curve(const double num, const double maxLimit, const double minLimit, char debug_color = 1);
private:
	void gui_debug_add(const net_gdebug& new_debug);

	std::queue< net_gdebug > _debugs;
};

typedef NormalSingleton< CGDebugEngine > GDebugEngine;

#endif