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

#ifndef __OBSTACLE_H__
#define __OBSTACLE_H__

#include "vector.h"
#include "geometry.h"
#include "utils.h"

#define EPSILON (1.0E-10)
enum ObstacleType{ OBS_CAR, OBS_DEFENCE }; //车和防守区域对速度的变化规则不同

struct state{
  vector2f pos;
  state *parent;
  state *next;
};

class obstacle{
	float robot_radius; // robot的半径
public:
	obstacle(){}
	int type; // type of obstacle
	int mask; // enable mask
	vector2f seg_start; // 长条圆的第一个中心点
	vector2f seg_end;   // 长条圆的第二个中心点
	vector2f vel;  // object velocity
	float radius;  // radius for circle
public:
	void set_robot_radius(float r){ robot_radius = r; }
	float margin(state s);
	float closest_point(state s, vector2f &p);
	bool check(state s);
	bool check(state s0, state s1);
	vector2f repulse(state s);
	float get_robot_radius(){return robot_radius;}
};

const int MAX_OBSTACLES = 24;
// 2*MAX_TEAM_ROBOTS+1+2+4
// robots, ball, defense areas, walls

class obstacles {
	float robot_radius;
	int num; //障碍物数量
	int current_mask;
public:
	obstacle obs[MAX_OBSTACLES];
public:
	obstacles(float robotRadius) : robot_radius(robotRadius), num(0), current_mask(0) {
		for( int i = 0; i < MAX_OBSTACLES; ++i ) {
			obs[i].set_robot_radius(robot_radius);
		}
	}
	void clear() { num = 0;}
	void add_long_circle(vector2f x0, vector2f x1, vector2f v, float r, int mask);
	void add_circle(vector2f x0, vector2f v, float r, int mask);

	void change_world(vector2f s, vector2f p, vector2f v, float maxD); //考虑速度因素影响，修改障碍物的体积

	void set_mask(int mask) { current_mask = mask; }
	bool check(vector2f p);
	bool check(vector2f p, int &id);
	bool check(state s);
	bool check(state s, int &id);
	bool check(state s0, state s1);
	bool check(state s0, state s1, int &id);
	vector2f repulse(state s);\
	vector2f repulse(state s, const int &id);
	int get_num() { return num; }
	state dynamic_obstacle(vector2f vel, state initial, state result, vector2f& output_vel);
	void draw_long_circle(const vector2f& seg_start, const vector2f& seg_end, const float& radius);
};
#endif /*__OBSTACLE_H__*/