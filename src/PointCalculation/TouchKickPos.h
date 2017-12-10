#ifndef _TOUCH_KICK_POS_
#define _TOUCH_KICK_POS_

#include <VisionModule.h>

/************************************************************************/
/*     TouchKickPos 由小车射门角度及球运动轨迹线确定的小车位置调整点      */
/************************************************************************/

class CTouchKickPos {
public:
	///> 构造函数
	CTouchKickPos();

	///> 对外的调用接口
	void GenerateTouchKickPos(const CVisionModule *pVision, const int player, const double kickdir);

	///> 对外的获取接口
	bool getKickValid()	{ return _kick_valid; }
	CGeoPoint getKickPos() { return _kick_pos; }

	void setPassDir(int cycle, double dir) { _pass_set_cycle  = cycle; _ball_invalid_pass_dir = dir; }
private:
	///> 内部调用接口
	void GenerateValidTouchKickPos(const CVisionModule *pVision, const int player, const double kickdir);
	void GenerateInvalidTouchKickPos(const CVisionModule *pVision, const int player, const double kickdir);

private:
	///> 内部重置接口
	void reset();

	///> 内部相关变量
	int _pass_set_cycle;
	bool  _kick_valid;		//是否可以射门
	CGeoPoint _kick_pos;    //所求的踢球点
	double _ball_invalid_pass_dir; // Lua注册的开球时传球的朝向
};

typedef NormalSingleton< CTouchKickPos > TouchKickPos;

#endif // ~_TOUCH_KICK_POS_