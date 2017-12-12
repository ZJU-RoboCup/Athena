#include <param.h>

namespace Param{
	namespace Field{
		double PITCH_LENGTH = 900;                 // ���س�
		double PITCH_WIDTH = 600;                  // ���ؿ�
		double PITCH_MARGIN = 1;                   // �߽���
		double CENTER_CIRCLE_R = 50;               // ��Ȧ�뾶
		double PENALTY_AREA_WIDTH = 250;           // �������
		double PENALTY_AREA_DEPTH = 100;           // �������
		double PENALTY_AREA_R = 100;               // ����Բ��
		double PENALTY_AREA_L = 50;                // ��������Բ�����߶�
		double PENALTY_MARK_X = 350;               // ������X����
		double OUTER_PENALTY_AREA_WIDTH = 175;     // ��Χ�������(���⿪��ʱ����վ�ڸ�����)
		double FREE_KICK_AVOID_BALL_DIST = 50;     // ���������ʱ��,�Է�����������ôԶ
		double FIELD_WALL_DIST = 20;               // ���ػ������߽�ľ���
		double GOAL_WIDTH = 100;
		double GOAL_DEPTH = 20;
		double RATIO = 1.5;
	}
	namespace Rule{
		const int Version = 2008; // ����İ汾/���
		const double MaxDribbleDist = 50; // ���������, 0����û������
	}
	
	namespace Vision{
		const double FRAME_RATE = 61; 
	}
	namespace Latency{ 
		// ����ֿ��Ĳ����Ǹ�server�������õ�
		float TOTAL_LATED_FRAME = 4.7f;
	}
	namespace AvoidDist{
		//�������ò���
		//const double TEAMMATE_AVOID_DIST =  Param::Field::MAX_PLAYER_SIZE/2+15.0f; // ���� 12
		const double TEAMMATE_AVOID_DIST = Param::Vehicle::V2::PLAYER_SIZE*3;
		const double OPP_AVOID_DIST = Param::Field::MAX_PLAYER_SIZE; // ���� 18
		const double BALL_AVOID_DIST = Param::Field::BALL_SIZE/2+2.0f; // ����3
		const double DEFENDKICK_MARKING_DIST = 65;
	}
	namespace Vehicle{
		namespace V2{
			const double PLAYER_SIZE = 9;
			const double PLAYER_FRONT_TO_CENTER = 8.0;
			const double PLAYER_CENTER_TO_BALL_CENTER = 9.93;
			const double KICK_ANGLE = ::Param::Math::PI*30/180; // ���Ի��������������Ƕ�
			const double DRIBBLE_SIZE = PLAYER_FRONT_TO_CENTER + ::Param::Field::BALL_SIZE; // ����ʱ����ľ���
			const double DRIBBLE_ANGLE = ::Param::Math::PI*17/180; // ���Դ��������������Ƕ�
			const double HEAD_ANGLE = 57*Param::Math::PI/180; // ǰ��Ŀ��ڽǶ�
			//const double TOUCH_SHIFT_DIST = 10.06;		// Touchʱ���˵ľ���
			const double TOUCH_SHIFT_DIST = 9.96;
		}
	}
}