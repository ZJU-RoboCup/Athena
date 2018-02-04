/**
* @file param.h
* 本文件定义程序中用到的各种参数.
* @date 10/1/2002
* @version 1.0
* @author peter@mail.ustc.edu.cn
*/
#ifndef _PARAM_H_
#define _PARAM_H_
#include <os_param.h> // 和系统有关的参数
/**
* 参数名字空间Param.
* 这个名字空间定义了程序中用到的所有的参数
*/
namespace Param{
	namespace Field{
		const int POS_SIDE_LEFT = 1;
		const int POS_SIDE_RIGHT = -1;
		const int MAX_PLAYER = 8; //2018.2.4

		/* 长度单位均为厘米,时间单位均为秒,质量单位为克,角度单位为弧度 */
		/* Ball */
		const double BALL_SIZE	= 2.15; // 半径
		const double BALL_DECAY = -0.8; // 阻力对球的加速度和速度成正比,单位为 /s
		/* Player */
		const double MAX_PLAYER_SIZE = 18;
		/* Field */
		extern double PITCH_LENGTH; // 场地长
		extern double PITCH_WIDTH; // 场地宽
		extern double PITCH_MARGIN; // 场地的边界宽度
		extern double CENTER_CIRCLE_R; // 中圈半径
		extern double PENALTY_AREA_WIDTH; // 禁区宽度
		extern double PENALTY_AREA_DEPTH; // 禁区深度
		extern double PENALTY_AREA_R; // 两个圆弧
		extern double PENALTY_AREA_L; // 连接两个圆弧的线段
		extern double PENALTY_MARK_X; // 点球点的X坐标
		extern double OUTER_PENALTY_AREA_WIDTH; // 外围禁区宽度(界外开球时不能站在该线内)
		extern double FIELD_WALL_DIST; // 场地护栏到边界的距离
		extern double GOAL_WIDTH; // 球门宽度
		extern double GOAL_DEPTH; // 球门深度
		extern double FREE_KICK_AVOID_BALL_DIST; // 开任意球的时候,对方必须离球这么远
		extern double RATIO;
	}
	namespace Math{
		const double PI = 3.14159265358979323846;
	}
	namespace Vehicle{
		namespace V2{
			extern const double PLAYER_SIZE;
			extern const double PLAYER_FRONT_TO_CENTER ;
			extern const double PLAYER_CENTER_TO_BALL_CENTER ;
			extern const double KICK_ANGLE; // 可以击球的最大相对身体角度
			extern const double DRIBBLE_SIZE; // 带球时离球的距离
			extern const double DRIBBLE_ANGLE; // 可以带球的最大相对身体角度
			extern const double HEAD_ANGLE; // 前面的开口角度
			extern const double TOUCH_SHIFT_DIST; //做Touch时后退的距离
		}
	}
	namespace AvoidDist{
		extern const double TEAMMATE_AVOID_DIST;
		extern const double OPP_AVOID_DIST;
		extern const double BALL_AVOID_DIST;
		extern const double DEFENDKICK_MARKING_DIST;
	}
	namespace Vision{
		extern const double FRAME_RATE; // 每秒桢数
	}
	namespace Latency{ 
        extern float TOTAL_LATED_FRAME; // 延迟的周期数,四舍五入
	}
	namespace Rule{
		extern const int Version; // 规则的版本
		extern const double MaxDribbleDist; // 最大带球距离
	}
}
#endif
