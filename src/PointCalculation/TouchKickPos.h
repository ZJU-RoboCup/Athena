#ifndef _TOUCH_KICK_POS_
#define _TOUCH_KICK_POS_

#include <VisionModule.h>

/************************************************************************/
/*     TouchKickPos ��С�����ŽǶȼ����˶��켣��ȷ����С��λ�õ�����      */
/************************************************************************/

class CTouchKickPos {
public:
	///> ���캯��
	CTouchKickPos();

	///> ����ĵ��ýӿ�
	void GenerateTouchKickPos(const CVisionModule *pVision, const int player, const double kickdir);

	///> ����Ļ�ȡ�ӿ�
	bool getKickValid()	{ return _kick_valid; }
	CGeoPoint getKickPos() { return _kick_pos; }

	void setPassDir(int cycle, double dir) { _pass_set_cycle  = cycle; _ball_invalid_pass_dir = dir; }
private:
	///> �ڲ����ýӿ�
	void GenerateValidTouchKickPos(const CVisionModule *pVision, const int player, const double kickdir);
	void GenerateInvalidTouchKickPos(const CVisionModule *pVision, const int player, const double kickdir);

private:
	///> �ڲ����ýӿ�
	void reset();

	///> �ڲ���ر���
	int _pass_set_cycle;
	bool  _kick_valid;		//�Ƿ��������
	CGeoPoint _kick_pos;    //����������
	double _ball_invalid_pass_dir; // Luaע��Ŀ���ʱ����ĳ���
};

typedef NormalSingleton< CTouchKickPos > TouchKickPos;

#endif // ~_TOUCH_KICK_POS_