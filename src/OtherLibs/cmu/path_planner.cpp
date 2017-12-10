/* LICENSE:
  =========================================================================
    CMDragons'02 RoboCup F180 Source Code Release
  -------------------------------------------------------------------------
    Copyright (C) 2002 Manuela Veloso, Brett Browning, Mike Bowling,
                       James Bruce; {mmv, brettb, mhb, jbruce}@cs.cmu.edu
    School of Computer Science, Carnegie Mellon University
  -------------------------------------------------------------------------
    This software is distributed under the GNU General Public License,
    version 2.  If you do not have a copy of this licence, visit
    www.gnu.org, or write: Free Software Foundation, 59 Temple Place,
    Suite 330 Boston, MA 02111-1307 USA.  This program is distributed
    in the hope that it will be useful, but WITHOUT ANY WARRANTY,
    including MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  ------------------------------------------------------------------------- */

#include <stdio.h>
#include "vector.h"
#include "constants.h"
#include "obstacle.h"
#include "path_planner.h"
#include <GDebugEngine.h>
#include <exception>
#include <time.h>
#include <float.h>

namespace{
	#define SAMEPOINT 18  //�����ж������Ƿ�ͬһ��
	#define STILLCOUNT 3  //�����ж�·���Ƿ��Ѿ��ﵽ������

	inline int boost_lrand48() {
		return rand();
	}

	inline float boost_drand48() { // ���� [0, 1]
		return float(boost_lrand48() % 10000) / 10000;
	}

    const bool plan_print = false;
    const float out_of_obs_dot = 0.2;

    inline float sdrand48() { return(2 * boost_drand48() - 1); }  //���� [-1, 1]
	
    void drawPath(state* p, const state& t) {
		if (p) {
			GDebugEngine::Instance()->gui_debug_line(CGeoPoint(t.pos.x, t.pos.y), CGeoPoint(p->pos.x, p->pos.y), COLOR_WHITE);
		}
		while (p && p->parent) {
			GDebugEngine::Instance()->gui_debug_line(CGeoPoint(p->parent->pos.x, p->parent->pos.y), CGeoPoint(p->pos.x, p->pos.y), COLOR_WHITE);
			p = p->parent;
		}
	}
}

path_planner::path_planner() {
	srand(time(0));
	init_ = false;
}

void path_planner::save_newpath() {
	for (int i = 0; i < num_pathpoint; i++) {
		last_pathpoint[i] = pathpoint[i];
	}
	last_pathlength = pathlength;
	last_num_pathpoint = num_pathpoint;
	last_path_target_collision = path_target_collision;
	last_path_target_length = path_target_length;
}

void path_planner::cal_vel() { //�˺�����Ҫʹ����save_newpth����, �����ٶ�
	vector2f lastPathPoint0to1;     // ������·����0 -> 1
	vector2f lastPathPoint1to2;     // ������·����1 -> 2
	float pdot;                     // lastPathPoint0to1 ��� lastPathPoint1to2
	float lastPathPoint0to1_length; // ������·����0 -> 1 �ĳ���
	float lastPathPoint1to2_length; // ������·����1 -> 2 �ĳ���
	float cos_lastp01_p12_angle;    // ·����0 -> 1 �� ·����1 -> 2 �нǵ�����
	float minVel;                   // ��С�ٶ�
	float tempVel;                  // ��ʱ�ٶȱ���
	float tempDist;                 // ��ʱ�������
	vector2f nearestPoint;          // �ϰ����ϵ������
	vector2f tempVertor;            // ��ʱ��������
	float tempDot;                  // ��ʱ��˱���
	float tempLength;               // ��ʱ���ȱ���
	float tempAngle_cos;            // ��ʱ�Ƕ�����ֵ
	int obs_num = obs->get_num();   // �ϰ�������
	float maxDist = 100.0;          // ֻ���Ǿ�����1m(100cm)�ڵ��ϰ���
	int maxDeceleration = 400;      // �����ٶ� cm/s^2
	float accelerationFactor = 0.2; // ���ٶ�����

	if (last_num_pathpoint >= 3 ) {
		lastPathPoint0to1 = last_pathpoint[1].pos-last_pathpoint[0].pos;
		lastPathPoint1to2 = last_pathpoint[2].pos-last_pathpoint[1].pos;
		pdot = lastPathPoint0to1.dot(lastPathPoint1to2);
		lastPathPoint0to1_length = lastPathPoint0to1.length();
		lastPathPoint1to2_length = lastPathPoint1to2.length();
		if ( pdot <= 0 || lastPathPoint0to1_length < 0.0001 || lastPathPoint1to2_length < 1) {  // �������򳤶�̫С���ٶ�Ϊ0
			out_vel = vector2f(0.0, 0.0);
			return;
		}
		else {
			//�������ϰ�����ٶ�
			cos_lastp01_p12_angle = pdot / (lastPathPoint0to1_length * lastPathPoint1to2_length);
			minVel = sqrt(lastPathPoint1to2_length * maxDeceleration); // ����������ٶȣ������ţ�������С�ٶ�
			if (minVel > 300) {  // ��С�ٶȲ��ܴ���300cm/s, С�����������
				minVel = 300;
			}
			minVel = minVel * cos_lastp01_p12_angle;
			tempVel = minVel;
			
			//�����ϰ�����ٶ�
			for(int i = 0; i < obs_num; i++) {
				//������ϰ���ľ����Լ��ϰ����ϵ������(�ϰ����Ե�ϵĵ�)
				tempDist = obs->obs[i].closest_point(last_pathpoint[1], nearestPoint) - obs->obs[i].get_robot_radius() - obs->obs[i].radius;
				if (tempDist < maxDist) { // ֻ���Ǿ�����1m�ڵ��ϰ���
					tempVertor = nearestPoint - last_pathpoint[1].pos;

					// ����pathpoint[2]���ٶȵ�����
					tempDot = lastPathPoint1to2.dot(tempVertor); // ����
					tempLength = tempVertor.length();
					if (lastPathPoint1to2_length < SAMEPOINT || tempLength < SAMEPOINT) { // �����һ���ܽ��������ϰ���ܽ����ٶ���Ҫ��С
						out_vel = vector2f(0.0, 0.0); // ֱ���˳�
						return;
					} 
					else {
						tempAngle_cos = tempDot / (lastPathPoint1to2_length * tempLength); // ��ȡ�Ƕ�����ֵ
						if (tempDot > 0.001) { // ����ϰ����λ�ú����崹ֱ(tempDot����0ʱ��Ϊ��ֱ)���򲻿�����������Ӱ�졣
							tempVel = sqrt(tempDist * 2 * maxDeceleration) / tempAngle_cos * accelerationFactor; // ����ٶ�
						}
					}

					// ����pathpoint[1]���ٶȵ�����,ͬ��
					tempDot = lastPathPoint0to1.dot(tempVertor);   // ����
					tempLength = tempVertor.length();
					if (lastPathPoint0to1_length < SAMEPOINT || tempLength < SAMEPOINT) { // �����һ���ܽ�,�����ϰ���ܽ����ٶ���Ҫ��С
						out_vel = vector2f(0.0, 0.0); // ֱ���˳�
						return;
					} 
					else {
						tempAngle_cos = tempDot / (lastPathPoint0to1_length * tempLength); // ��ȡ�Ƕ�ֵ
						if (tempDot > 0.001) { // ����ϰ����λ�ú����崹ֱ���򲻿�����������Ӱ�졣
							tempVel = sqrt(tempDist * 2 * maxDeceleration) / tempAngle_cos * accelerationFactor; // ����ٶ�
						}
					}

					if (tempVel < minVel) {
						minVel = tempVel;
					}			
				}
			}
			out_vel = lastPathPoint1to2.norm(minVel); //�ٶȳ���Ϊ��һ��·���ķ���
			return;
		}
	}
	out_vel = vector2f(0.0, 0.0);
	return;
}

vector2f path_planner::get_vel() {
	cal_vel();
	return out_vel;
}

state path_planner::random_state() {  // �������״̬
    state s;
    s.pos = vector2f((Param::Field::PITCH_LENGTH / 2 + Param::Field::GOAL_DEPTH) * sdrand48(), Param::Field::PITCH_LENGTH / 2 * sdrand48()); // ȫ��
    s.parent = NULL;
    return(s);
}

// ��ʼ��rrt��Ϣ���������ڵ�����waypoints�д���Ľڵ�������goalΪĿ��ĸ��ʣ���waypoint�еĵ�ΪĿ��ĸ��ʣ�����.
// �ڵ���ʱ�������²���path_planner[i].init(150, 80, 0.15, 0.65, Param::Field::MAX_PLAYER_SIZE, initial);
void path_planner::init(int _max_nodes, int _num_waypoints, float _goal_target_prob, float _waypoint_target_prob, float _step_size, state _goal) {
	if ( init_ ) {
		return;
	}
    vector2f minv;
	vector2f maxv;

    max_nodes = _max_nodes;
    num_waypoints = _num_waypoints;
	goal_target_prob = _goal_target_prob;
	waypoint_target_prob = _waypoint_target_prob;
	waypoint_target_prob_old = _waypoint_target_prob;
	step_size = _step_size;
	num_pathpoint = 0;
	last_num_pathpoint = 0;
	pathlength = 0;
	last_pathlength = 99999999;
	path_target_length = 0;
	last_path_target_length = 999999999;
	last_path_target_collision = false;
	path_target_collision = false;
	still_count = 0;
	out_vel = vector2f(0.0, 0.0);

	// ��waypoint�����ʼ��Ϊ�����
	for(int i = 0; i < num_waypoints; i++){
		waypoint[i] = random_state();
	}

	//������(minv, maxv), �������½Ǻ����Ͻ�
	minv = vector2f(-Param::Field::PITCH_LENGTH / 2 - Param::Field::GOAL_DEPTH, -Param::Field::PITCH_WIDTH / 2);
	maxv = vector2f(Param::Field::PITCH_LENGTH / 2 + Param::Field::GOAL_DEPTH / 2, Param::Field::PITCH_WIDTH / 2);
    tree.setdim(minv, maxv, 16, 8); // ����һ����
	init_ = true;
}

//���������ľ���
inline float path_planner::distance(state &state0, state &state1) {
	return(Vector::distance(state0.pos, state1.pos));
}

// ���ڵ�n����kdtree,�������¼���Ľڵ�
state *path_planner::add_node(state newExtend, state *parent) {
	// �����n����չ��Ľڵ�Ҳ����nearest+step֮��Ľڵ�
	// parent��nearest�ڵ�
    if(num_nodes >= max_nodes) {
		return(NULL);
	}
    newExtend.parent = parent;
    node[num_nodes] = newExtend;
    tree.add(&node[num_nodes]); //state��cmu��node��ת��
    num_nodes++;
    return(&node[num_nodes - 1]);
}

/*  ѡ������Ŀ���
    ѡ�����ŵĳ���㣬����0��1�������p
	targetType = 0����goal_target_prob��0.15���ĸ��ʰ�goal��Ϊtarget
				 1����goal_waypoint_prob��0.65���ĸ��ʰ�waypoints�еĵ���Ϊtarget
				 2����1-0.65-0.15=0.20�ĸ��ʣ��������Ϊtarget */
state path_planner::choose_target(int &targetType) {
    double random = boost_drand48();
    int i;
    if (random < goal_target_prob) { // 0.15
		targetType = 0;
		return(goal);
	}
	else if (random < goal_target_prob + waypoint_target_prob) { // 0.65
		targetType = 1;
		i = boost_lrand48() % num_waypoints;
		return(waypoint[i]); // ���ȡwaypoint�еĵ�
	}
	else { // 0.20
		targetType = 2;
		return(random_state());
	}
}

state *path_planner::find_nearest(state target) { // �ҵ�����target��������нڵ�
    state *nearest;
    float nearestDist = 0;
	// ����kdtree.cpp�е�neareast,�õ�λ�õ�������target����ĵ�
    nearest = tree.nearest(nearestDist, target.pos);

    if (!nearest) {  // nearest == NULL
		// NOTE: something bad must have happened if we get here.
		// find closest current state
		nearest = &node[0];
		float d, nd;
		nd = distance(*nearest, target);
		for (int i = 0; i < num_nodes; i++) {
			d = distance(node[i], target);
			if (d < nd) {
				nearest = &node[i];
				nd = d;
			}
		}
	}
    return(nearest);
}

int path_planner::extend(state *nearest, state target) {
	// Extend�����ŵĵ���nearest��target������,����nearest����Ϊstep_size�ĵ�,��Ҫ��֤����㲻��ײ���ϰ���.
	// rrt������,�������½ڵ�
	state newExtend;
    vector2f step;
	float stepLength;
	// ȷ������step�ķ���Ϊtarget��nearest���߷���
    step = target.pos - nearest->pos;
    stepLength = step.length();
    if(stepLength < step_size) {
		return(0);
	}
    step *= step_size / stepLength;	// ������λ�����ٸ���������step_size

    newExtend.pos = nearest->pos + step;	// newExtend�����½ڵ�
	
	// �����������ʼ��s�Ƿ��Ӵ��ϰ���
	// obs->check()������obstacle.cpp ��,�����ϰ�������0�����ϰ�����1��
	if(obs->check(*nearest, newExtend)) {
		nearest = add_node(newExtend, nearest);
	}
    // add_node()��ɺ󣬽�newExtend���뵽rrt��,�洢��node[num_nodes-1]��
    return(num_nodes); // num_nodes�Ѿ���add_node()������
}

// ERRT�㷨*******************************************************************************************************
state path_planner::errt(state initial, state _goal, bool drawtreeflag) {
/*	0�������ʼ���Ѿ�����goal�ȽϽ������ü򻯷���������ִ��rrt��plan
	1��ѡ��target
	2��Ѱ��nearest
	3����nearest�㴦��target����(extend)��ȷ����һ���㣬���㹻�ӽ�goal,ת��4�������ظ�1��
	4�����򿼲������Ƿ���Լ򻯣������򿼲�ÿһ���ڵ㣬�����ܷ��initialֱ����ͨ��û���ϰ��赲��
	5�����ɹ��滮����·���ϵĵ㣬����ģ����ּ���waypoints�С�
	6�����ص�ǰ����Ҫ����һ��λ�� */
	state target, *nearest, *nearest_goal;
	float tempDist;
	float newDist;
	goal = _goal;
	// ����0: ���initial��goal�㹻��ʱ�ļ�㷽��: ֱ�ӿ���target��initial��goal������,��������goal,�����ϰ�
	tempDist = Vector::distance(initial.pos, goal.pos);
	if (false && tempDist < 20) { // ����
		pathlength =  0;
		path_target_length = 0;
		path_target_collision = 1;
		num_pathpoint = 2;
		pathpoint[0] = initial;
		pathpoint[1] = goal;
		return(goal);
	}
	if (obs->check(initial, goal)) {
		// ������յ�����֮��û����ײ
		pathlength =  0;
		path_target_length = 0;
		path_target_collision = 1;
		num_pathpoint = 2;
		pathpoint[0] = initial;
		pathpoint[1] = goal;
		if (_isnan(goal.pos.x) || _isnan(goal.pos.y)) {
			printf("ERROR INIT3\n");
		}
		return(goal);
	}
	else {
		tree.clear();
		int i, iter_limit;
		int targ_type;
		i = num_nodes = 0;
		nearest = nearest_goal = add_node(initial, NULL);
		tempDist = distance(*nearest, goal);
		// plan rrt�滮,ֱ������Ŀ����㹻��,����ѭ�������ﵽ����,�㹻��ʱ�Ϳ��������ϰ���
		iter_limit = max_nodes; // ���ԵĴ������ڵĽ����
		// i������Ŀ����num_nodes������Ŀ��nearest_goal��Ŀ�������趨ֵʱ������while
		// i����,num_nodes����,�����ҵ���Ŀ���ܽ��ĵ���,�ͻ�����while
		while (i < iter_limit && num_nodes < max_nodes /* && d>NEAR_DIST*/ ) {
			// ����1: ѡ��target
			// targ_type �Ǹ����ã���������������ĸ��ʣ�����goal��waypoint, ����randomstate
			// init�У���waypoint�Ѿ�ȫ������Ϊ����㣬randomState
			target = choose_target(targ_type);
			// ����2: Ѱ��nearest(���target����goal,��ֱ��ʹ��nearest_goal,�伴Ϊ�����; �������target�ٵ���find_nearest()�����ҵ������)
			// find_nearest��Ѱ�����ϣ���x�����state��һ��ʼ������һ�ſ���
			nearest = (targ_type == 0) ? nearest_goal : find_nearest(target);
			// ����3: ��nearest�㴦��target����(extend)��ȷ����һ����
			// target���ǲ���1��ѡ������֣�nearest���ǲ���2��ѡ�������..�������ҵ�,���߳�ʼ��
			// extend��nearest��target������,��stepѰ��һ�������
			// extend�л��� ��չ ��� ���...�����check��add_node
			// extend�в������Ż�add_node()
			extend(nearest, target); // ��exrend�в�������nearest�����Ϊ����չ�ĵ�
			// �õ��¼���ĵ�
			// ���extend��check�ǰ�ȫ��,��add_node, node��num_nodes���ᱻ����
			newDist = distance(node[num_nodes - 1], goal);
			if (newDist < tempDist) { // tempDist��ʼ����nearest��goal֮��ľ��룬nearestһ��ʼ����ʼ��Ϊinitial
				// �¼���ĵ���Ŀ����㹻��,����������nearest_goal,���������Ŀ������tempDist
				nearest_goal = &node[num_nodes - 1];
				tempDist = newDist;
			}
			i++;
		}
		// ����4:����·��,Ѱ���Ż�,���򿼲������Ƿ���Լ�,�����򿼲�ÿһ���ڵ�,�����ܷ��initialֱ����ͨ,û���ϰ��赲
		state *p;
		p = nearest_goal; // �ո���while�б�����,����������Ŀ���ܽ��ĵ�
		path_target_length = newDist; // ·�����յ�ĳ���...��ʵ�����һ����ӵĵ㵽Ŀ���ľ���
		path_target_collision = obs->check(goal, *p);
		num_pathpoint = 0;
		pathlength = 0;
		pathpoint[num_pathpoint++] = initial; // ��·���㱣��
		// ÿ�ν�ѭ��pathpoint[]�ĵ�0��Ԫ�ض��ᱻ���ó�initial
		// p����while���ҵ���nearest_goal
		if (Vector::distance(p->pos, initial.pos) > 1.0) { // �������㲻�����ʱ
			p = NULL;
			while (p != nearest_goal) {
				// ��ͷ�ң��ҵ���initial���ĸ�state��������һ��ѭ��������state���ĸ�state������ֱ��ֱ���ƶ���nearest_goal
				// ������֤��pathpoint�еĵ㶼��û������ģ���û����ײ��
				p = nearest_goal;
				// ����ʼ�㲻����,���nearest_goal����������һ����: ����ʼ��initial���õ��������ϰ��ﲻ����,������ֱ������;
				while (p != NULL && !obs->check(pathpoint[num_pathpoint - 1], *p)) {
					p = p->parent; // ���initial��p�������ϰ�����������p����Ϊp��parent���ټ�飬ֱ���鵽һ����initial���������ϰ���ĵ�
				}
				pathlength += Vector::distance(pathpoint[num_pathpoint-1].pos, p->pos); // ��ʼ�㵽����p��ľ���
				pathpoint[num_pathpoint++] = *p;
			}
			pathpoint[num_pathpoint++] = *p; // ��ʱ *p == nearest_goal
			pathlength += path_target_length; // ���յ�pathlength����Ҫ����nearest��goal֮��ľ���
		}
		pathpoint[num_pathpoint++] = goal;
		if (num_pathpoint > 2) {
			// �������յ���Ϊ��ֱ��Ŀ���,����һ֡��Ŀ���
			target = pathpoint[1]; //initial֮���һ���㣬��һ����nearest
		}
		else {
			// ���ݵ������ҵ�����;
			pathlength = 9999;
			path_target_length = 9999;
			path_target_collision = 1;
			num_pathpoint = 2;
			pathpoint[0] = initial;
			pathpoint[1] = goal;
			vector2f tempv = obs->repulse(initial);
			target.pos = (initial.pos + tempv * step_size);
			pathpoint[num_pathpoint] = pathpoint[num_pathpoint - 1];
			pathpoint[num_pathpoint - 1] = target;
			num_pathpoint ++;
			pathlength = Vector::distance(pathpoint[0].pos, pathpoint[1].pos);
			path_target_length = Vector::distance(pathpoint[1].pos, pathpoint[2].pos);
			path_target_collision = obs->check(pathpoint[1], pathpoint[2]);
			return(goal);
		}
		// ����5:���ɹ��滮����·���ϵĵ㣬����ģ����ּ���waypoints�С�
		if (drawtreeflag) {
		    //waypoint����!!!���ó�nearest֮ǰ�ĵ㣬���������һЩ�㡣
			int j;
			if((num_pathpoint > 2)/* && (d < NEAR_DIST)*/ || boost_drand48() < 0.1) {
				// ���ܵ���Ŀ���,����һ��С����; �򽫵�ǰ����Ŀ�������nearest_goal����������·���ϵĵ����waypoints
				j = 1;
				while ((num_pathpoint - 1) != j) {
					i = boost_lrand48() % num_waypoints;
					waypoint[i] = pathpoint[j];
					waypoint[i].parent = NULL;
					// �ӵ���ֱ��Ŀ��㵽nearest_goal��μӵ�waypoint��ȥ,֮ǰ���Թ�
					j++;
				}
				if (0 /*d < NEAR_DIST*/) {
					waypoint_target_prob = 1.0 - goal_target_prob - 0.01;
				}
			}
			else {
				// ����,������ɵ�,�����ȥ
				for (j = 0; j < num_waypoints; j++) {
					i = boost_lrand48() % num_waypoints;
					waypoint[i] = random_state();
					waypoint_target_prob = waypoint_target_prob_old;
				}
			}
		}
		// ����6:���ص�ǰ����Ҫ����һ��λ��
		return(target);
	}			
}

state path_planner::plan(obstacles *_obs, int obs_mask, state initial, vector2f vel, state _goal) {
	state target_now; // Ҫ���ص�Ŀ��㣬ÿ��ֻ����һ���㣬��һ���ٽ�����������һ���㣬planner��ȫ�ֵ�
	vector2f lastMotionVector;
	float lastMotionVectorLen;
	int i;
	lastMotionVector = vector2f(0.0, 0.0);
	obs = _obs;
	obs->set_mask(obs_mask);

	if ( last_num_pathpoint > 1 ) { // һ��ʼ��������ѡ�񣬵ڶ��ν���һ���ѡ���(��������Ҳû����)
		if (Vector::distance(last_pathpoint[last_num_pathpoint - 1].pos, _goal.pos) > SAMEPOINT) { // �����յ�,�����¿�ʼ�Ż�
			still_count = 0;
			last_path_target_collision = false;
			last_num_pathpoint = 0;  
		}
		else {
			// �ж�ԭʼ·���Ƿ������е��ϰ�����ײ
			for (i = 0; i < last_num_pathpoint - 2; i++) {
				if ( !obs->check(last_pathpoint[i], last_pathpoint[i + 1])) {
					break; // ������������ײ
				}
			}
			if ( i == (last_num_pathpoint - 2) && ((obs->check(last_pathpoint[i], last_pathpoint[i + 1])) /* || (Vector::distance(last_pathpoint[i].pos,last_pathpoint[i+1].pos) < 20) */)) { // ȫ��û����ײ��������������ܽ�
				last_path_target_collision = true;  // ��ʾû��ײ
			}
			else {
				last_path_target_collision = false; // ��ʾ��ײ
				still_count = 0;                    // ����״̬����
			}

			// ��ԭʼ·�����ݸ��º��˶���������
			last_pathpoint[0].pos = initial.pos;    // ���³�ʼλ��
			lastMotionVector = last_pathpoint[1].pos - last_pathpoint[0].pos; // �˶��ķ�������
			lastMotionVectorLen = lastMotionVector.length();
			if (lastMotionVectorLen > SAMEPOINT) {
				lastMotionVector = lastMotionVector / lastMotionVectorLen; // ��һ��
			}
			else {
				lastMotionVector = vel.norm(300.0); // ���С��0������ԭ��·��
			}

			if ( Vector::distance(last_pathpoint[1].pos, initial.pos) < 25 ) { // ������λ���Ƿ񵽴��һ��,�������,��ı�·��(25cm֮�ڱ�ʾ����)
				still_count = 0; // ����Ѱ�����ŵ�
				if ( last_num_pathpoint > 2) { // ����ж����ʱ
					for (i = 1; i < last_num_pathpoint - 1; i++) {
						last_pathpoint[i] = last_pathpoint[i + 1];
					}
					last_num_pathpoint -= 1;
				}
			}
			if (last_num_pathpoint == 1) {
				printf("DEADLY ERROR************************\n");
			}
			last_pathpoint[last_num_pathpoint - 1].pos = _goal.pos;
		}
	}
	target_now = errt(initial, _goal, 1); // ERRT
	return target_now; // ERRT�����γ��˵�goal����,���ҷ�����һ��target,Ϊ����,initial֮����Ǹ�state
}