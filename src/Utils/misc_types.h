/************************************************************************/
/*							���ָ��������Ͷ���                          */
/************************************************************************/

#ifndef _MISC_TYPES_H_
#define _MISC_TYPES_H_

#include <geometry.h>
#include <vector>

/// �ײ��ܵ���λ���Ʒ���
enum CTRL_METHOD{	
	CMU_TRAJ	= 1,						// CMU			�����ٵ���
	ZERO_FINAL	= 2,						// Cornell		���ٶȵ���
	ZERO_TRAP	= 3,						// ZJUNlict		���ٶȵ���
	NONE_TRAP	= 4,						// ZJUNLict		�����ٵ���
};

/// ��Ա״̬����ṹ��
struct PlayerStatus{
	/// ���캯�� �� �������Ĭ�ϳ�ʼ��
	PlayerStatus(): flag(0),
					pos(0.0,0.0),angle(0.0),vel(0.0,0.0),rotvel(0.0),
					max_acceleration(0.0),max_deceleration(0.0),rotdir(0),
					is_specify_ctrl_method(false),specified_ctrl_method(ZERO_FINAL),
					needkick(true),needdribble(false),ispass(false),ischipkick(false),kickprecision(0.0),kickpower(0.0),chipkickpower(0.0),
					speed_x(0.0),speed_y(0.0),rotate_speed(0.0) { }

	/// ��ǩ
	int flag;														// �����ǩ

	/// ��λ���� �� ���ڵײ��˶����� , Ĭ��ʹ��
	CGeoPoint pos;													// ȫ��Ŀ����λ��
	double angle;													// ȫ��Ŀ�굽�㳯��
	CVector vel;													// ȫ��Ŀ�굽��ƽ���ٶ�	
	double rotvel;													// ȫ��Ŀ�굽��ת���ٶ�
	int rotdir;														// ��ת�ķ���
	/// �˶����� �� ���ڵײ��˶����� ��ָ����ǩ����
	double max_acceleration;										// �����ٶ�
	double max_deceleration;										// �����ٶ�

	/// ���Ʒ��� �� ���ڵײ��˶����� ��ָ�����Ʒ���
	bool is_specify_ctrl_method;									// �Ƿ�ָ���˶����Ʒ���
	CTRL_METHOD specified_ctrl_method;								// ָ�����˶����Ʒ���
	
	/// ������� �� ����ƽ��������� ��Ĭ��ʹ��
	bool needkick;													// ������ִ�п���
	bool needdribble;												// ������ִ�п���  add by gty 16-6-15
	bool ispass;													// �Ƿ���д���
	bool ischipkick;												// ������ƽ��
	double kickprecision;											// �����򾫶�
	double kickpower;												// ��������
	double chipkickpower;											// ��������	

	/// �ٶȲ��� �� ����ֱ���ٶȿ��� ��ָ����ǩ���� 
	double speed_x;													// ȫ��x����ƽ���ٶ�
	double speed_y;													// ȫ��y����ƽ���ٶ�
	double rotate_speed;											// ת���ٶ�
	
	/// ��ǩ����
	// Ĭ�ϵ�flags
	static const int NOTHING						= 0x00000000; // û��flag
	// ���ߵ�flags
	static const int SLOWLY							= 0x00000001; // ����
	static const int QUICKLY						= 0x00000002; // ���٣��������ײ������Ա���򣬽���
	static const int DO_NOT_STOP					= 0x00000008; // ������ٶ����� --xxx---
	static const int DODGE_BALL						= 0x00000010; // �����
	static const int POS_ONLY						= 0x00000020; // ֻ����λ��,�����ǽǶ�
	static const int AVOID_SHOOTLINE				= 0x00000040; // ���������
	static const int NOT_DRIBBLE					= 0x00000080; // ��Factory��ǿ�Ʋ�������
	static const int DRIBBLING						= 0x00000100; // �ߵĹ����д���
	static const int DODGE_OUR_DEFENSE_BOX			= 0x00000200; // ��Ҫ���뼺������
	static const int NOT_AVOID_THEIR_VEHICLE		= 0x00000400; // ����ܶԷ���
	static const int ACCURATELY						= 0x00000800; // ��ȷ���ߣ��������Ƿ��Ѿ�����
	static const int TURN_AROUND_FRONT				= 0x00001000; // �Գ�����ǰ��Ϊ������ת --xxx---
	static const int DODGE_REFEREE_AREA				= 0x00002000; // ��ܿ���50cm����
	static const int SPECIFY_SPEED					= 0x00004000; // ָ���ٶ�
	static const int SPECIFY_ROTATE_SPEED			= 0x00008000; // ָ����ת�ٶ�
	static const int SPECIFY_ACCELERATION			= 0x00010000; // ָ�����ٶ�
	static const int SPECIFY_ROTATE_ACCELERATION	= 0x00020000; // ָ����ת���ٶ�
	static const int ONLY_IN_FIELD					= 0x00040000; // ֻ�ڳ�������
	static const int NOT_AVOID_OUR_VEHICLE			= 0x00080000; // ������ҷ���

	// �������flags
	static const int SPECIFY_DIRECTION				= 0x01000000; // ָ�����峯��
	static const int IGNORE_PLAYER_CLOSE_TO_TARGET	= 0x02000000; // ���Ե�סĿ���Ķ�Ա
	static const int ALLOW_DSS						= 0x08000000; // DSS�Ŀ�������
	static const int FORCE_KICK						= 0x80000000; // ǿ�ƿ�����
	static const int KICK_WHEN_POSSIABLE			= 0x00200000; // �ܻ����ʱ��ͻ���
	static const int FACE_THAT_POINT				= 0x20000000; // ����ĳ����
};

/// ��״̬�ṹ
struct stBallStatus{
	stBallStatus() : receiver(0){ }
	CGeoPoint pos;				// �����Ŀ���
	int receiver;				// ����Ķ�Ա����
	int Sender;                 // �����ߺ��루added by shizhy)
	double angle;
	bool front;
};

/// ����ṹ
struct TaskT{
	TaskT() : executor(0){ }
	int executor;				// ִ������Ķ�Ա����
	PlayerStatus player;		// ��Ա��״̬
	stBallStatus ball;			// ���״̬
};

/// ָ��ö��
enum CommandTypeT{ CTStop, CTDash, CTTurn, CTArc, CTSpeed, CTKick };

/// ���ȼ�
enum PriorityType{ LowestPriority, LowPriority, NormalPriority, HighPriority, HighestPriority };

#endif	// ��_MISC_TYPES_H_
