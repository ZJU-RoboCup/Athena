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

#ifndef __PATH_PLANNER_H__
#define __PATH_PLANNER_H__

#include "obstacle.h"
#include "kdtree.h"

const int MAX_NODES = 400;      // ���ڵ�
const int MAX_WAYPTS = 100;     // ���waypoint����
const float NEAR_DIST = 20.0;   // ������С��20cmʱ�Ϳ���ֱ�ӹ�ȥ.

class path_planner {
	state node[MAX_NODES];
	state waypoint[MAX_WAYPTS];
	state last_pathpoint[MAX_WAYPTS];  // ��¼·����
	state pathpoint[MAX_WAYPTS];
	state goal;       // Ŀ���
	vector2f out_vel; // ����������ٶ�(��С������)

	KDTree<state> tree;
	int num_nodes;
	int max_nodes;
	int num_waypoints;
	int last_num_pathpoint;
	int num_pathpoint;
	static const int max_extend = 1;

	float goal_target_prob;
	float waypoint_target_prob;
	float waypoint_target_prob_old;
	float step_size;

	float last_pathlength;    //��¼·������
	float pathlength;
	float path_target_length;       //��¼nearestpoint���ϰ���֮��ľ���
	float last_path_target_length;
	bool path_target_collision;     //��¼nearestpoint�Ƿ����յ����ϰ���
	bool last_path_target_collision;
	int still_count;          //·��û�б仯�ļ�����

	bool init_;

	obstacles *obs;
public:
	path_planner();
	void init(int _max_nodes, int _num_waypoints, float _goal_target_prob, float _waypoint_target_prob, float _step_size, state _goal);
	float distance(state &state0, state &state1);
	state random_state();
	state *add_node(state newExtend, state *parent);
	state choose_target(int &targ);
	state *find_nearest(state target);
	int extend(state *nearest, state target);
	state errt(state initial, state _goal, bool drawtreeflag);
	state plan(obstacles *_obs, int obs_mask, state initial, vector2f _vel, state _goal);
	void save_newpath();
	void cal_vel();
	vector2f get_vel();
};

#endif /*__PATH_PLANNER_H__*/