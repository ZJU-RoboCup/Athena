/**
* @file param.h
* ���ļ�����������õ��ĸ��ֲ���.
* @date 10/1/2002
* @version 1.0
* @author peter@mail.ustc.edu.cn
*/
#ifndef _PARAM_H_
#define _PARAM_H_
#include <os_param.h> // ��ϵͳ�йصĲ���
/**
* �������ֿռ�Param.
* ������ֿռ䶨���˳������õ������еĲ���
*/
namespace Param{
	namespace Field{
		const int POS_SIDE_LEFT = 1;
		const int POS_SIDE_RIGHT = -1;
		const int MAX_PLAYER = 8; //2018.2.4

		/* ���ȵ�λ��Ϊ����,ʱ�䵥λ��Ϊ��,������λΪ��,�Ƕȵ�λΪ���� */
		/* Ball */
		const double BALL_SIZE	= 2.15; // �뾶
		const double BALL_DECAY = -0.8; // ��������ļ��ٶȺ��ٶȳ�����,��λΪ /s
		/* Player */
		const double MAX_PLAYER_SIZE = 18;
		/* Field */
		extern double PITCH_LENGTH; // ���س�
		extern double PITCH_WIDTH; // ���ؿ�
		extern double PITCH_MARGIN; // ���صı߽���
		extern double CENTER_CIRCLE_R; // ��Ȧ�뾶
		extern double PENALTY_AREA_WIDTH; // �������
		extern double PENALTY_AREA_DEPTH; // �������
		extern double PENALTY_AREA_R; // ����Բ��
		extern double PENALTY_AREA_L; // ��������Բ�����߶�
		extern double PENALTY_MARK_X; // ������X����
		extern double OUTER_PENALTY_AREA_WIDTH; // ��Χ�������(���⿪��ʱ����վ�ڸ�����)
		extern double FIELD_WALL_DIST; // ���ػ������߽�ľ���
		extern double GOAL_WIDTH; // ���ſ��
		extern double GOAL_DEPTH; // �������
		extern double FREE_KICK_AVOID_BALL_DIST; // ���������ʱ��,�Է�����������ôԶ
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
			extern const double KICK_ANGLE; // ���Ի��������������Ƕ�
			extern const double DRIBBLE_SIZE; // ����ʱ����ľ���
			extern const double DRIBBLE_ANGLE; // ���Դ��������������Ƕ�
			extern const double HEAD_ANGLE; // ǰ��Ŀ��ڽǶ�
			extern const double TOUCH_SHIFT_DIST; //��Touchʱ���˵ľ���
		}
	}
	namespace AvoidDist{
		extern const double TEAMMATE_AVOID_DIST;
		extern const double OPP_AVOID_DIST;
		extern const double BALL_AVOID_DIST;
		extern const double DEFENDKICK_MARKING_DIST;
	}
	namespace Vision{
		extern const double FRAME_RATE; // ÿ������
	}
	namespace Latency{ 
        extern float TOTAL_LATED_FRAME; // �ӳٵ�������,��������
	}
	namespace Rule{
		extern const int Version; // ����İ汾
		extern const double MaxDribbleDist; // ���������
	}
}
#endif
