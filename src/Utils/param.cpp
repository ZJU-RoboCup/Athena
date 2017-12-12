#include <param.h>

namespace Param{
	namespace Field{
		double PITCH_LENGTH = 900;                 // 场地长
		double PITCH_WIDTH = 600;                  // 场地宽
		double PITCH_MARGIN = 1;                   // 边界宽度
		double CENTER_CIRCLE_R = 50;               // 中圈半径
		double PENALTY_AREA_WIDTH = 250;           // 禁区宽度
		double PENALTY_AREA_DEPTH = 100;           // 禁区深度
		double PENALTY_AREA_R = 100;               // 两个圆弧
		double PENALTY_AREA_L = 50;                // 连接两个圆弧的线段
		double PENALTY_MARK_X = 350;               // 点球点的X坐标
		double OUTER_PENALTY_AREA_WIDTH = 175;     // 外围禁区宽度(界外开球时不能站在该线内)
		double FREE_KICK_AVOID_BALL_DIST = 50;     // 开任意球的时候,对方必须离球这么远
		double FIELD_WALL_DIST = 20;               // 场地护栏到边界的距离
		double GOAL_WIDTH = 100;
		double GOAL_DEPTH = 20;
		double RATIO = 1.5;
	}
	namespace Rule{
		const int Version = 2008; // 规则的版本/年份
		const double MaxDribbleDist = 50; // 最大带球距离, 0代表没有限制
	}
	
	namespace Vision{
		const double FRAME_RATE = 61; 
	}
	namespace Latency{ 
		// 下面分开的部分是给server作仿真用的
		float TOTAL_LATED_FRAME = 4.7f;
	}
	namespace AvoidDist{
		//避障所用参数
		//const double TEAMMATE_AVOID_DIST =  Param::Field::MAX_PLAYER_SIZE/2+15.0f; // 厘米 12
		const double TEAMMATE_AVOID_DIST = Param::Vehicle::V2::PLAYER_SIZE*3;
		const double OPP_AVOID_DIST = Param::Field::MAX_PLAYER_SIZE; // 厘米 18
		const double BALL_AVOID_DIST = Param::Field::BALL_SIZE/2+2.0f; // 厘米3
		const double DEFENDKICK_MARKING_DIST = 65;
	}
	namespace Vehicle{
		namespace V2{
			const double PLAYER_SIZE = 9;
			const double PLAYER_FRONT_TO_CENTER = 8.0;
			const double PLAYER_CENTER_TO_BALL_CENTER = 9.93;
			const double KICK_ANGLE = ::Param::Math::PI*30/180; // 可以击球的最大相对身体角度
			const double DRIBBLE_SIZE = PLAYER_FRONT_TO_CENTER + ::Param::Field::BALL_SIZE; // 带球时离球的距离
			const double DRIBBLE_ANGLE = ::Param::Math::PI*17/180; // 可以带球的最大相对身体角度
			const double HEAD_ANGLE = 57*Param::Math::PI/180; // 前面的开口角度
			//const double TOUCH_SHIFT_DIST = 10.06;		// Touch时后退的距离
			const double TOUCH_SHIFT_DIST = 9.96;
		}
	}
}