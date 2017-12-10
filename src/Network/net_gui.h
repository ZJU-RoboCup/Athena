#ifndef NET_GUI_H_
#define NET_GUI_H_

// 消息类型
#define NET_GUI_DEBUG_LINE  1
#define NET_GUI_DEBUG_ARC   2
#define NET_GUI_DEBUG_BALL  3
#define NET_GUI_DEBUG_TIME  4
#define NET_GUI_DEBUG_FIELD_MSG  5
#define NET_GUI_DEBUG_NORMAL_MSG 6
#define NET_GUI_DEBUG_TRIANGLE 7
#define NET_GUI_DEBUG_ROBOT 8
#define NET_GUI_DEBUG_CURVE 9

#include "server.h"
#include <cstring>

// 线段箭头的方向
#define G_ARROW_FORW    (1 << 1)
#define G_ARROW_BACK    (1 << 2)
#define G_ARROW_BOTH    (G_ARROW_FORW | G_ARROW_BACK)

struct gdebug_line {
	PosT p[2];  // Two points determine a line
	char flags;     // Flags... see above
};

struct gdebug_triangle {
	PosT p[3];
};

struct gdebug_robot{
	PosT p;
	float dir;
};

// 是否填充
#define G_ARC_FILL      (1 << 1)

struct gdebug_arc {
	PosT center;
	float r; 
	short a1, a2;
	char flags;     // Flags... see above
};

#define G_MSG_MAXLENGTH 64
#define G_MAX_NET_GDEBUG 500

struct gdebug_msg {
	PosT p;
	char msg[G_MSG_MAXLENGTH];
};

struct gdebug_curve {
	double num;
	double maxLimit;
	double minLimit;
};

// 颜色定义
#define COLOR_WHITE 0
#define COLOR_RED 1
#define COLOR_ORANGE 2
#define COLOR_YELLOW 3
#define COLOR_GREEN 4
#define COLOR_CYAN 5
#define COLOR_BLUE 6
#define COLOR_PURPLE 7
#define COLOR_GRAY 8
#define COLOR_BLACK 9

struct net_gdebug{
	char msgtype; // = NET_GUI_DEBUG_*
	char team;
	char robot_num;
	char color;  // = COLOR_*
	int timestamp; // 周期

	union{
		gdebug_line line;
		gdebug_arc arc;
		gdebug_triangle tri;
		gdebug_robot car;
		gdebug_msg msg;
		gdebug_curve curve;
	} info;

	int size() const;
};

struct net_gdebugs{
	int totalnum;
	int totalsize;
	net_gdebug msg[G_MAX_NET_GDEBUG];
};

inline int net_gdebug::size() const {
	return sizeof(net_gdebug) - 
		(msgtype != NET_GUI_DEBUG_FIELD_MSG ? 0 : (G_MSG_MAXLENGTH - (strlen(info.msg.msg) + 1)));
}

const int net_gui_out_maxsize = sizeof(net_gdebug);

#define NET_GUI_AUTO 100
#define NET_GUI_PLAY 101
#define NET_GUI_CONTROL 102

#define G_COMMAND_MAXLENGTH 256

struct net_gcommand{
	char msgtype; // = NET_GUI_PLAY
	char cmd[G_COMMAND_MAXLENGTH];

	int size() const;
};

inline int net_gcommand::size() const {
	return sizeof(net_gcommand) - (G_COMMAND_MAXLENGTH - (strlen(cmd) + 1));
}

const int net_gui_in_maxsize = sizeof(net_gcommand);

#endif