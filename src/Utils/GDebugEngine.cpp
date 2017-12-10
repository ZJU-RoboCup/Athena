#include "GDebugEngine.h"
#include <param.h>
#include <cstring>
#include <WorldModel.h>

CGDebugEngine::CGDebugEngine()
{
	while ( !_debugs.empty() )
		_debugs.pop();
}
CGDebugEngine::~CGDebugEngine()
{
	 while ( !_debugs.empty() )
		 _debugs.pop();
}
void CGDebugEngine::gui_debug_x(const CGeoPoint& p, char debug_color)
{
	CGeoPoint basePos = p;

	const CGeoPoint p1 = basePos + CVector(5, 5);
	const CGeoPoint p2 = basePos + CVector(-5,-5);
	gui_debug_line(p1, p2, debug_color);

	const CGeoPoint p3 = basePos + CVector(5,-5);
	const CGeoPoint p4 = basePos + CVector(-5, 5);
	gui_debug_line(p3,p4, debug_color);
}
void CGDebugEngine::gui_debug_line(const CGeoPoint& p1, const CGeoPoint& p2, char debug_color)
{
	PosT pos1;
	pos1.valid  = 1;
	if ( WorldModel::Instance()->option()->MySide() == Param::Field::POS_SIDE_RIGHT ){
		pos1.x = -p1.x();
		pos1.y = -p1.y();
	}
	else{
		pos1.x = p1.x();
		pos1.y = p1.y();
	}

	PosT pos2;
	pos2.valid = 1;
	if ( WorldModel::Instance()->option()->MySide() == Param::Field::POS_SIDE_RIGHT ){
		pos2.x = -p2.x();
		pos2.y = -p2.y();
	}
	else{
		pos2.x = p2.x();
		pos2.y = p2.y();
	}

	net_gdebug the_debug = {NET_GUI_DEBUG_LINE, 0, 0, debug_color, 1};
	the_debug.info.line.p[0] = pos1;
	the_debug.info.line.p[1] = pos2;
	the_debug.info.line.flags = 0;

	gui_debug_add(the_debug);
}
void CGDebugEngine::gui_debug_arc(const CGeoPoint& p, double r, double start_angle, double span_angle, char debug_color)
{
	PosT center;
	center.valid = 1;
	if ( WorldModel::Instance()->option()->MySide() == Param::Field::POS_SIDE_RIGHT ){
		center.x = -p.x();
		center.y = -p.y();
	}
	else{
		center.x = p.x();
		center.y = p.y();
	}
	
	net_gdebug the_debug = {NET_GUI_DEBUG_ARC, 0, 0, debug_color, 1};
	the_debug.info.arc.center = center;
	the_debug.info.arc.r = r;
	the_debug.info.arc.a1 = start_angle;
	the_debug.info.arc.a2 = span_angle;
	the_debug.info.arc.flags = 0;

	gui_debug_add(the_debug);
}
void CGDebugEngine::gui_debug_triangle(const CGeoPoint& p1, const CGeoPoint& p2, const CGeoPoint& p3, char debug_color)
{
	PosT pos1,pos2,pos3;
	pos1.valid = pos2.valid = pos3.valid = 1;
	if ( WorldModel::Instance()->option()->MySide() == Param::Field::POS_SIDE_RIGHT ){
		pos1.x = -p1.x();
		pos1.y = -p1.y();

		pos2.x = -p2.x();
		pos2.y = -p2.y();

		pos3.x = -p3.x();
		pos3.y = -p3.y();
	}
	else{
		pos1.x = p1.x();
		pos1.y = p1.y();

		pos2.x = p2.x();
		pos2.y = p2.y();

		pos3.x = p3.x();
		pos3.y = p3.y();
	}
	

	net_gdebug the_debug = {NET_GUI_DEBUG_TRIANGLE, 0, 0, debug_color, 1};
	the_debug.info.tri.p[0] = pos1;
	the_debug.info.tri.p[1] = pos2;
	the_debug.info.tri.p[2]	= pos3;

	gui_debug_add(the_debug);
}
void CGDebugEngine::gui_debug_robot(const CGeoPoint& p, double robot_dir)
{
	PosT robot_pos;
	robot_pos.valid = 1;
	if ( WorldModel::Instance()->option()->MySide() == Param::Field::POS_SIDE_RIGHT ){
		robot_pos.x = -p.x();
		robot_pos.y = -p.y();
	}
	else{
		robot_pos.x = p.x();
		robot_pos.y = p.y();
	}
	
	float rdir;
	if ( WorldModel::Instance()->option()->MySide() == Param::Field::POS_SIDE_RIGHT ){
		robot_dir = Utils::Normalize(robot_dir + Param::Math::PI);
	}
	rdir = robot_dir*180/Param::Math::PI; // ×ª³É½Ç¶È
	

	net_gdebug the_debug = {NET_GUI_DEBUG_ROBOT, 0, 0, 0, 1};
	the_debug.info.car.p = robot_pos;
	the_debug.info.car.dir = rdir;

	gui_debug_add(the_debug);
}
void CGDebugEngine::gui_debug_msg(const CGeoPoint& p, const char* msgstr, char debug_color)
{
	PosT center;
	center.valid = 1;
	if ( WorldModel::Instance()->option()->MySide() == Param::Field::POS_SIDE_RIGHT ){
		center.x = -p.x();
		center.y = -p.y();
	}
	else{
		center.x = p.x();
		center.y = p.y();
	}

	net_gdebug the_debug = {NET_GUI_DEBUG_FIELD_MSG, 0, 0, debug_color, 1};
	strcpy(the_debug.info.msg.msg, msgstr);
	the_debug.info.msg.p = center;

	gui_debug_add(the_debug);
}
void CGDebugEngine::gui_debug_curve(const double num, const double maxLimit, const double minLimit, char debug_color)
{
	net_gdebug the_debug = {NET_GUI_DEBUG_CURVE, 0, 0, debug_color, 1};
	the_debug.info.curve.num = num;
	the_debug.info.curve.maxLimit = maxLimit;
	the_debug.info.curve.minLimit = minLimit;

	gui_debug_add(the_debug);
}
void CGDebugEngine::gui_debug_add(const net_gdebug& new_debug)
{
	_debugs.push(new_debug);
}
net_gdebug CGDebugEngine::get_queue_front()
{
	return _debugs.front();
}
void CGDebugEngine::pop_front()
{
	_debugs.pop();
}
