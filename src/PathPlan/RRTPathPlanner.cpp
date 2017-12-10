#include "RRTPathPlanner.h"
#include <cmu/obstacle.h>
#include <cmu/path_planner.h>
#include <cmu/constants.h>
#include <param.h>
#include <utils.h>
#include <TimeCounter.h>
#include <GDebugEngine.h>
#include "cmu/vector.h"
#include <math.h>
#include "PlayInterface.h"
#include <TaskMediator.h>

namespace{
	const double TEAMMATE_AVOID_DIST = Param::Vehicle::V2::PLAYER_SIZE + 4.0f; // 2014/03/13 �޸ģ�Ϊ�˼���stop��ʱ��ס�ĸ��� yys
	const double OPP_AVOID_DIST = Param::Vehicle::V2::PLAYER_SIZE + 2.5f;
	const double BALL_AVOID_DIST = Param::Field::BALL_SIZE + 1.5f;
	//const double VELMIN = 1.0f; //��С�ٶ�
	path_planner pathPlanner[Param::Field::MAX_PLAYER];  //ERRT�㷨ʵ��
}

CRRTPathPlanner::CRRTPathPlanner(const CVisionModule* pVision, const TaskT& task, const double avoidLength) {
	int player = task.executor;
	CGeoPoint target = task.player.pos; // ȫ��Ŀ����λ��
	int flags = task.player.flag;
	int shootCar = task.ball.Sender; // �����ߺ���

	obstacles obs(avoidLength);  // �ϰ���
	vector2f playerPos(pVision->OurPlayer(player).X(), pVision->OurPlayer(player).Y());
	vector2f playerVel(pVision->OurPlayer(player).VelX(), pVision->OurPlayer(player).VelY());
	vector2f target_pos(target.x(), target.y());

	// ���г����ǱܶԷ�����
	obs.add_long_circle(vector2f(Param::Field::PITCH_LENGTH / 2, -Param::Field::PENALTY_AREA_L / 2),
											vector2f(Param::Field::PITCH_LENGTH / 2, Param::Field::PENALTY_AREA_L / 2),
											vector2f(0.0, 0.0),
											Param::Field::PENALTY_AREA_R + Param::Field::MAX_PLAYER_SIZE,
											1);

	// ����Ƕ������
	if (flags & PlayerStatus::AVOID_SHOOTLINE) {
		const PlayerVisionT& shooter = pVision->OurPlayer(shootCar);
		// ��������
		obs.add_long_circle(vector2f(shooter.Pos().x(), shooter.Pos().y()), vector2f(Param::Field::PITCH_LENGTH/2, 0.0f), vector2f(0.0f, 0.0f), 3.0f, 1);
		// ����������
		obs.add_long_circle(vector2f(shooter.Pos().x(), shooter.Pos().y()), vector2f(Param::Field::PITCH_LENGTH/2, Param::Field::GOAL_WIDTH/2.0), vector2f(0.0f, 0.0f), 3.0f, 1);
		// ����������
		obs.add_long_circle(vector2f(shooter.Pos().x(), shooter.Pos().y()), vector2f(Param::Field::PITCH_LENGTH/2, -Param::Field::GOAL_WIDTH/2.0), vector2f(0.0f, 0.0f), 3.0f, 1);
	}

	// set up teammates as obstacles
	if (!(flags & PlayerStatus::NOT_AVOID_OUR_VEHICLE)) {
		for(int i = 1; i <= Param::Field::MAX_PLAYER; ++i) {
			const PlayerVisionT& teammate = pVision->OurPlayer(i);
			if((i != player) && teammate.Valid()) {
				obs.add_circle(vector2f(teammate.Pos().x(), teammate.Pos().y()), vector2f(teammate.Vel().x(), teammate.Vel().y()), TEAMMATE_AVOID_DIST, 1);
				//GDebugEngine::Instance()->gui_debug_arc(teammate.Pos(), TEAMMATE_AVOID_DIST, 0, 360);
			}
		}
	}

	// set up opponents as obstacles
	if (!(flags & PlayerStatus::NOT_AVOID_THEIR_VEHICLE)) {
		for(int i=1; i<=Param::Field::MAX_PLAYER; ++i) {
			const PlayerVisionT& opp = pVision->TheirPlayer(i);
			if(opp.Valid()) {
				if( (false && (flags & PlayerStatus::IGNORE_PLAYER_CLOSE_TO_TARGET) && (target.dist(opp.Pos()) < Param::Field::MAX_PLAYER_SIZE / 2)) /*|| opp.Vel().mod() > VELMIN */) {
					continue;
				}
				else {
					obs.add_circle(vector2f(opp.Pos().x(), opp.Pos().y()), vector2f(opp.Vel().x(), opp.Vel().y()), OPP_AVOID_DIST, 1);
					//GDebugEngine::Instance()->gui_debug_arc(opp.Pos(), OPP_AVOID_DIST, 0, 360);
				}
			}
		}
	}

	// ���� �� ����
	if((true || flags & PlayerStatus::DODGE_OUR_DEFENSE_BOX) && (task.executor != PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie()))) {
		// �ҷ�����
		obs.add_long_circle(vector2f(-Param::Field::PITCH_LENGTH / 2, -Param::Field::PENALTY_AREA_L / 2),
												vector2f(-Param::Field::PITCH_LENGTH / 2, Param::Field::PENALTY_AREA_L / 2),
												vector2f(0.0, 0.0),
												Param::Field::PENALTY_AREA_R + Param::Vehicle::V2::PLAYER_SIZE,
												1);
		// �Է�������
		obs.add_circle(vector2f(Param::Field::PITCH_LENGTH / 2, -Param::Field::GOAL_WIDTH), vector2f(0.0, 0.0), 1.0 * Param::Vehicle::V2::PLAYER_SIZE, 1);
		obs.add_circle(vector2f(Param::Field::PITCH_LENGTH / 2, Param::Field::GOAL_WIDTH), vector2f(0.0, 0.0), 1.0 * Param::Vehicle::V2::PLAYER_SIZE, 1);
	}

	// ball
	if(flags & PlayerStatus::DODGE_BALL) {
		const MobileVisionT& ball = pVision->Ball();
		obs.add_circle(vector2f(ball.Pos().x(), ball.Pos().y()), vector2f(ball.Vel().x(), ball.Vel().y()), BALL_AVOID_DIST, 1);
	}

	if(flags & PlayerStatus::DODGE_REFEREE_AREA) {
		const MobileVisionT& ball = pVision->Ball();
		obs.add_circle(vector2f(ball.Pos().x(), ball.Pos().y()), vector2f(0.0f, 0.0f), 50.0f, 1);
	}

	obs.set_mask(1);

	// set initial state
	state initial;
	state goal;
	state result;
	initial.pos = playerPos;
	goal.pos = target_pos;

//	pathPlanner[player - 1].init(200, 150, 0.05, 0.55, Param::Field::MAX_PLAYER_SIZE, initial);  // �޸��˲����� 2014-03-09�� YYS
	pathPlanner[player - 1].init(400, 100, 0.15, 0.75, Param::Field::MAX_PLAYER_SIZE, initial);  // ���ԣ� 2014-07-07�� zhyaic

	result= pathPlanner[player - 1].plan(&obs, 1, initial, playerVel, goal);
	//GDebugEngine::Instance()->gui_debug_x(CGeoPoint(result.pos.x, result.pos.y), COLOR_GREEN);
	vector2f tempVel(0.0, 0.0);
	tempVel = pathPlanner[player - 1].get_vel();
	_vel = CVector(tempVel.x, tempVel.y);
	_path.clear();
	_path.push_back(CGeoPoint(result.pos.x, result.pos.y)); // ����·����
}