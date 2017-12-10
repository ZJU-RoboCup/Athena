#include "DynamicsSafetySearch.h"
#include "time.h"
#include <vector>
#include <utility>
#include <algorithm>

#define min(a,b)    (((a) < (b)) ? (a) : (b))
#define max(a,b)    (((a) > (b)) ? (a) : (b))

#ifndef PI
	#define PI	3.1415926f
#endif

namespace {
#define DIST_IGNORE 90000
#define SAFE 1
#define UNSAFE 0
#define RANDMAX 32767.0f
#define OURPLAYER 0
#define THEIRPLAYER 1
#define THISPLAYER 2
#define BALL 3
#define VELMIN 1 // 1cm/s
#define ACCMIN 1 // 1cm/s^2
#define DEBUGFLAG 0  // 2014/03/12 �رգ�Ϊ�˵���ʱ����ɾ�
#define ACCURATE_AOID 250
	double TEAMMATE_AVOID_DIST = Param::AvoidDist::TEAMMATE_AVOID_DIST / 2; // ���� 13.5
	double OPP_AVOID_DIST = Param::AvoidDist::OPP_AVOID_DIST; // ���� 18
	double BALL_AVOID_DIST = Param::AvoidDist::BALL_AVOID_DIST / 2; // ���� 1.5
	float MAX_TRANSLATION_SPEED; // 350
	float MAX_TRANSLATION_ACC;   // 650
	float MAX_TRANSLATION_DEC;   // 650
}
/************************************************************************/
/* modified 2011-12-26 by HQS                                      
 ��ʼ��																		
/************************************************************************/
CDynamicSafetySearch::CDynamicSafetySearch() {
	{
		DECLARE_PARAM_READER_BEGIN(CGotoPositionV2)
			READ_PARAM(MAX_TRANSLATION_SPEED)
			READ_PARAM(MAX_TRANSLATION_ACC)
			READ_PARAM(MAX_TRANSLATION_DEC)
		DECLARE_PARAM_READER_END
	}
	_e = 0.0f;                    //Ȩֵ
	_gamma = 0.0f;                //С������ʻʱ��
	_pos = vector2f(0.0f,0.0f);   //С����ǰ��λ��
	_vel = vector2f(0.0f,0.0f);   //С����ǰ���ٶ�
	_nvel = vector2f(0.0,0.0f);   //С����һʱ�̵��ٶ�
	_acc = vector2f(0.0f,0.0f);   //С���ļ��ٶ�
	_t_acc = vector2f(0.0f,0.0f); //С�����ż���
	_DECMAX = MAX_TRANSLATION_DEC;  //С�����ļ��ٶ�
	_AECMAX = MAX_TRANSLATION_ACC;  //С�����ļ��ٶ�
	_VAMX = MAX_TRANSLATION_SPEED;//С�������ٶ�
	_stoptime = 0.0f;
	_C = 0.0f;                    //����ִ��һ�����ڵ�ʱ��
	_CNum = 0.0f;
	_flag = 0;
	for (int i = 0; i < 6; i++) {
		last_acc[i] = vector2f(0.0f, 0.0f);
		last_e[i] = 1.0f;         // 1.0��ʾ�����ڵļ��ٶȶ�������Ӱ��
		find_flag[i] = false;
		priority[i] = 0;          // Ĭ��С�������ȼ�Ϊ0
	}
}
/************************************************************************/
/*  modified 2012-2-14 by HQS 
/*  modified 2014-03-07 by YYS
����С�������ȼ�, t_priority Խ�����ȼ�Խ��
/************************************************************************/
void CDynamicSafetySearch::refresh_priority(const int player, const int t_priority, const vector2f target, const vector2f pos) {
	if (t_priority > 0) {
		priority[player - 1] = t_priority;
		return;
	}
	else if (t_priority == 0) {
		vector2f dis = target - pos;
		if (dis.length() <= 3 * Param::Field::MAX_PLAYER_SIZE && dis.length() > Param::Field::MAX_PLAYER_SIZE) { // �����Ŀ���ľ���С��3�������Ҵ���1������
			priority[player - 1] = 1;
		}
		else if (dis.length() <= Param::Field::MAX_PLAYER_SIZE) { // ��Ŀ���ľ���С��һ��������ʾ���յ�ܽ�����ֱ�ӹ�ȥ
			priority[player - 1] = -1;
		}
		else {
			priority[player - 1] = 0;
		}
	}
	if (DEBUGFLAG) {
		cout<<"player:"<<player<<'\t'<<"priority: "<<priority[player-1]<<endl;
	}
}
/************************************************************************/
/* modified 2011-12-26 by HQS
/* modified 2014-03-07 by YYS
 ���ۺ��������ۺ����������ڵ����֣��Լ������ڵļ��ٶȷ���ʹ�С�й�
 Ϊ�˱�֤���ٶ�������
 tempAcc Ϊ����ļ��ٶȣ�player Ϊ����ĳ���
 ����ֵΪ0~1��ֵ
/************************************************************************/
float CDynamicSafetySearch::Evaluation(const vector2f tempAcc, const int player) { // ����Ȩֵ����
	vector2f best_acc = _acc;
	float val1, val2;
	float coef;
	coef = 1.0f - last_e[player - 1];
	val1 = fabs(angle_all(best_acc.x, best_acc.y) - angle_all(tempAcc.x, tempAcc.y));
	if (val1 > PI) {
		val1 = 2 * PI - val1;
	}
	val1 = val1 * val1 + 0.5f * (PI * PI - val1 * val1) / (_AECMAX * _AECMAX) * fabs(best_acc.sqlength() - tempAcc.sqlength());
	val1 = min(val1, PI * PI);
	val2 = fabs(angle_all(last_acc[player - 1].x, last_acc[player - 1].y) - angle_all(tempAcc.x, tempAcc.y));
	if (val2 > PI) {
		val2 = 2 * PI - val2;
	}
	val2 = val2 * val2 + 0.3f * (PI * PI - val2 * val2) / (_AECMAX * _AECMAX) * fabs(_AECMAX * _AECMAX - tempAcc.sqlength());
	val2 = min(val2, PI * PI);
	float temp_e =  ((val1 + coef * val2) / (PI * PI + coef * PI * PI));
	return temp_e; // ��������
}
/************************************************************************/
/* modified 2011-12-22 by HQS 
/* modified 2014-03-07 by YYS
��Ϊ������������С���ĳ��ź������������ٶȣ�����С����һʱ�̵��ٶ�     
��Ҫ����˵����																 
Iter:��ʾ������Ҽ��ٶȵĸ���������Խ��ƽ���ٶ�Խ��������Խ�ٿ����Ҳ����� 
crash_car_dir��crash_car_num��ʾ�����ײС���ĽǶ��복�� 
_C����ʾ����һ�����ڵ�ʱ��
_CNum����ʾ�����Ԥ��С�����е�����
/************************************************************************/
CVector CDynamicSafetySearch::SafetySearch(const int player, CVector Vnext, const CVisionModule* pVision, const int t_priority, const CGeoPoint target, const int flags, const float stoptime, double max_acc) { // �����һ֡���ٶ�
	if (max_acc > 1) {
		_AECMAX = max_acc;
	}
	_stoptime = stoptime;
	_flag = flags; // �˶���ǩ
	int Iter = 15; // ��ʾ������ҵĴ���
	vector2f crashed_car_dir = vector2f(0.0f, 0.0f);
	int crash_car_num = 0;
	vector2f tempAcc = vector2f(0.0, 0.0f);
	_pos = CGeoPoint2vector2f(pVision->OurPlayer(player).Pos());
	refresh_priority(player, t_priority, CGeoPoint2vector2f(target), _pos);
	srand(time(NULL)); // �������ʼ��
	_nvel = CVector2vector2f(Vnext);
	_vel = CVector2vector2f(pVision->OurPlayer(player).Vel());
	_CNum = max(int(_vel.length() / _AECMAX * 60.0f), 15); // ����һ����ֹ��һ���˶����������ǰ���Ƚ��� // ��֤��һ�������ڲ�����ײ
	_C = 1.0f / 60.0f; // ����ִ��һ�����ڵ�ʱ��
	_acc = (_nvel - _vel) / _C;
	_gamma = _C * _CNum;
	_e = 1.0f;		 // Ȩֵ��ʼ��
	
	int type = 1; // type:0��ʾ�ҷ�����1��ʾ�з���
	if (priority[player - 1] == -1 || CheckAccel(player, _acc, pVision, crashed_car_dir, crash_car_num, type, _C) == SAFE) {
		_t_acc = _acc;
		_e = 1.0f;
		find_flag[player - 1] = true;
		//GDebugEngine::Instance()->gui_debug_msg(pVision->OurPlayer(player).Pos(), "Me", COLOR_YELLOW);
	}
	else {
		float angle_start, angle_end, max_radius, t_mid_angle;
		float safe_angle_start, safe_angle_end;
		float avoid_dist, avoid_angle;
		if (type == OURPLAYER) {
			avoid_dist = max(TEAMMATE_AVOID_DIST, (crashed_car_dir.length() / 2));
			avoid_angle = asin((TEAMMATE_AVOID_DIST) / (avoid_dist));
		}
		else if(type == THEIRPLAYER) {
			avoid_dist = max((TEAMMATE_AVOID_DIST + OPP_AVOID_DIST), (crashed_car_dir.length()));
			avoid_angle = asin((TEAMMATE_AVOID_DIST + OPP_AVOID_DIST) / (avoid_dist));
		}
		else if (type == BALL) {
			avoid_dist = max((TEAMMATE_AVOID_DIST + BALL_AVOID_DIST), (crashed_car_dir.length()));
			avoid_angle = asin((TEAMMATE_AVOID_DIST + BALL_AVOID_DIST) / (avoid_dist));
		}
			
		max_radius = _AECMAX;
		t_mid_angle = angle_all(crashed_car_dir.x, crashed_car_dir.y);
		angle_start = t_mid_angle - avoid_angle;
		angle_end = t_mid_angle + avoid_angle;
		safe_angle_start = angle_end;
		safe_angle_end = angle_start + 2 * PI;

		if (DEBUGFLAG) {
			vector2f tl1, tl2;
			tl1 = vector2f(cos(safe_angle_start), sin(safe_angle_start));
			tl2 = vector2f(cos(safe_angle_end), sin(safe_angle_end));
			GDebugEngine::Instance()->gui_debug_line(vector2f2CGeoPoint(_pos), vector2f2CGeoPoint(_pos) + vector2f2CVector(tl1 * avoid_dist), COLOR_RED);
			GDebugEngine::Instance()->gui_debug_line(vector2f2CGeoPoint(_pos), vector2f2CGeoPoint(_pos) + vector2f2CVector(tl2 * avoid_dist), COLOR_BLACK);
		}
		float temp_e = 0.0f; // ��ʱ��Ȩֵ
		int iter_i = 0;

		tempAcc = last_acc[player - 1];
		if (find_flag[player - 1] == true && CheckAccel(player, tempAcc, pVision, crashed_car_dir, crash_car_num, type, _C) == SAFE) { // �ж��ϴεļ��ٶ��Ƿ����
			_t_acc = tempAcc;
			_e = Evaluation(tempAcc, player);
		}
		else {
			find_flag[player - 1] = false;
		}

		for (iter_i = 0; iter_i < Iter; iter_i++) {//��ͨ����
			tempAcc = RandomAcc(safe_angle_start, safe_angle_end, max_radius);
			if (DEBUGFLAG) {
				GDebugEngine::Instance()->gui_debug_line(vector2f2CGeoPoint(_pos), vector2f2CGeoPoint(_pos) + vector2f2CVector(tempAcc), COLOR_YELLOW);
			}
			temp_e = Evaluation(tempAcc, player);
			if( temp_e <= _e && CheckAccel(player, tempAcc, pVision, crashed_car_dir, crash_car_num, type, _C) == SAFE)
			{
				_t_acc = tempAcc;
				_e = temp_e;
				find_flag[player - 1] = true;
			}
		}
		CGeoPoint temp;
		vector2f tp;

		if(type == OURPLAYER) {
			temp = pVision->OurPlayer(crash_car_num).Pos();
			tp = _pos - CGeoPoint2vector2f(pVision->OurPlayer(crash_car_num).Pos());
		}
		else {
			temp = pVision->TheirPlayer(crash_car_num).Pos();
			tp = _pos - CGeoPoint2vector2f(pVision->TheirPlayer(crash_car_num).Pos());
		}
		if (DEBUGFLAG) {
			GDebugEngine::Instance()->gui_debug_arc(vector2f2CGeoPoint(_pos), TEAMMATE_AVOID_DIST * 5, 0.0f, 360.0f, COLOR_YELLOW);
		}
		if (find_flag[player - 1] == false) {
			string str;
			char appdex[10];
			itoa(player, appdex, 10);
			str = str.append(appdex).append(" NOT FIND");
			GDebugEngine::Instance()->gui_debug_msg(target, str.c_str(), 1);
		}
		//if (t_find_flag == false && tp.length() < (TEAMMATE_AVOID_DIST*2.0f))
		//{//���⴦��������С���ľ���ܽ�ʱ������û�л�����Ž�ʱ����Ҫ��һ��������Ѱ�Ұ�ȫ����
		//	_t_acc = OutOfTrouble(player,_pos,_acc,(TEAMMATE_AVOID_DIST*5.0f ),pVision) * _AECMAX;
		//	GDebugEngine::Instance()->gui_debug_line(vector2f2CGeoPoint(_pos),vector2f2CGeoPoint(_pos)+vector2f2CVector(_t_acc),COLOR_YELLOW);
		//	_e = Evaluation(_t_acc,player);
		//	GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(10.0,10.0),"Near",1);
		//}
		//else
		//{
		//	GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0.0,0.0),"CRASH2!",1);
		//}
	}

	if (find_flag[player - 1] == false) {
		if (_vel.length() / _DECMAX > _C) {
			_t_acc = _vel.norm() * (-1.0f) * _DECMAX;
			_nvel = _t_acc * _C + _vel;
		}
		else {
			_nvel = vector2f(0.0f, 0.0f);
		}
		last_acc[player - 1] = (_nvel - _vel) / _C;
		last_e[player - 1] = 1.0f;
	}
	else {
		last_e[player - 1] = _e;
		last_acc[player - 1] = _t_acc;
		_nvel = _t_acc * _C + _vel;
	}
	if (DEBUGFLAG) {
		GDebugEngine::Instance()->gui_debug_line(vector2f2CGeoPoint(_pos), vector2f2CGeoPoint(_pos) + vector2f2CVector(_t_acc), COLOR_BLUE);
		if (CheckAccel(player, _t_acc, pVision, crashed_car_dir, crash_car_num, type, EPSILON) == UNSAFE) {
			GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-20.0, -20.0), "EROOR", 1);
			CheckAccel(player, _t_acc, pVision, crashed_car_dir, crash_car_num, type, EPSILON);
		}
	}
	return vector2f2CVector(_nvel);
}
/************************************************************************/
/* modified 2011-12-26 by HQS                                           */
/* modified 2014-03-09 by YYS                                           */
/* limitTime ����0~limitTime�ڵ���ײ��Ч								    */
/* type ��ǲ�ͬ���͵ĳ��������ҷ����Է����͵�ǰС��                         */
/* �����Ƿ�������С����ײ�Լ����������ײ�ĳ��źͳ��ķ���                     */
/* limitTime��ʱ�䷶Χ�ڲ�������ײ                                        */
/************************************************************************/
bool compare_order(const std::pair<int, double>& a, const std::pair<int, double>& b) {
	return a.second > b.second;
}

bool CDynamicSafetySearch::CheckAccel (const int player, vector2f acc, const CVisionModule* pVision, vector2f& crash_car_dir, int& crash_car_num, int& type, const float limitTime) {
	// type: 0��ʾ�ҷ���, 1��ʾ�з���, 2��ʾ���ڹ滮��С��, 3��ʾ��
	std::vector<pair<int, double>> order;
	std::pair<int, double> temp;
	// ������
	if (!(_flag & PlayerStatus::NOT_AVOID_OUR_VEHICLE)) { // ����ҷ���
		for(int i = 1; i <= Param::Field::MAX_PLAYER; i++) { // �����Լ�������Ч�ĳ��洢��order��
			if(i != player && pVision->OurPlayer(i).Valid()) {
				temp.first = i;
				temp.second = pVision->OurPlayer(i).Pos().dist2(pVision->OurPlayer(player).Pos());
				if (temp.second < DIST_IGNORE) {
					order.push_back(temp);
				}	
			}
		}
	}
	sort(order.begin(), order.end(), compare_order); // ������С����Զ��������
	while(!order.empty()) {
		const PlayerVisionT& teamate = pVision->OurPlayer(order.back().first);
		const PlayerVisionT& lastCycle_teamate = pVision->OurPlayer(pVision->LastCycle(), order.back().first);
		vector2f pj = vector2f(teamate.Pos().x(), teamate.Pos().y()); // λ������
		vector2f vj = vector2f(teamate.VelX(), teamate.VelY()); // �ٶ�����
		vector2f Aj; // ���ٶ�����
		if (order.back().first > player) {
			Aj = (vj - vector2f(lastCycle_teamate.VelX(), lastCycle_teamate.VelY())) / _C; // ���Ĳ���ʱע�⣬���Ҳ������Ҫ�ı䣿
		}
		else { // ����ǰ���С��������Ҫ���ȿ������ǵļ��ٶ�
			Aj = last_acc[order.back().first - 1];
		}
		type = OURPLAYER;
		if(CheckRobot(player, _pos, _vel, acc, order.back().first, pj, vj, Aj, type, limitTime) == UNSAFE) { // �ж��Ƿ���ײ
			crash_car_num = order.back().first;
			float tc;
			if (Aj.sqlength() <= ACCMIN) {
				tc = _C * _CNum;
			}
			else {
				tc = (Aj.norm() * _VAMX - vj).length() / Aj.length();
				tc = min(_C * _CNum, tc);
			}
			crash_car_dir = pj + vector2f(tc * vj.x, tc * vj.y) + vector2f(0.5f * tc * tc * Aj.x, 0.5f * tc * tc * Aj.y) - _pos;
			return UNSAFE; //�����ײֱ�ӷ���
		}
		order.pop_back();
	}
	//�з���
	if (!(_flag & PlayerStatus::NOT_AVOID_THEIR_VEHICLE)) {
		for(int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
			if(pVision->TheirPlayer(i).Valid()) {
				temp.first = i;
				temp.second = pVision->TheirPlayer(i).Pos().dist2(pVision->OurPlayer(player).Pos());
				if (temp.second < DIST_IGNORE) {
					order.push_back(temp);
				}
			}
		}
	}
	sort(order.begin(), order.end(), compare_order);//���з�С����Զ��������
	type = THEIRPLAYER;
	while(!order.empty()) {
		const PlayerVisionT& opp = pVision->TheirPlayer(order.back().first);
		const PlayerVisionT& last_opp = pVision->TheirPlayer(pVision->LastCycle(), order.back().first);
		vector2f pj = vector2f(opp.Pos().x(), opp.Pos().y());
		vector2f vj = vector2f(opp.VelX(), opp.VelY());
		vector2f Aj;
		Aj = (vj - vector2f(last_opp.VelX(), last_opp.VelY())) /_C; 
		if(CheckRobot(player, _pos, _vel, acc, order.back().first, pj, vj, Aj, type, limitTime) == UNSAFE) {
			crash_car_num = order.back().first;
			float tc;
			if (Aj.sqlength() <= ACCMIN) {
				tc = _C * _CNum;
			}
			else {
				tc = (Aj.norm() * _VAMX - vj).length() / Aj.length();
				tc = min(_C * _CNum,tc);
			}
			crash_car_dir = pj + vector2f(tc * vj.x, tc * vj.y) + vector2f(0.5f * tc * tc * Aj.x, 0.5f * tc * tc * Aj.y) - _pos;
			return UNSAFE; // �����ײֱ�ӷ���
		}
		order.pop_back();
	}
	// ��
	type = BALL;
	if(pVision->Ball().Valid() && (_flag & PlayerStatus::DODGE_BALL)) {
		vector2f pj = vector2f(pVision->Ball().Pos().x(), pVision->Ball().Pos().y());
		vector2f vj = vector2f(pVision->Ball().VelX(), pVision->Ball().VelY());
		const MobileVisionT& last_ball = pVision->Ball(pVision->LastCycle());
		vector2f Aj;
		Aj = (vj - vector2f(last_ball.VelX(), last_ball.VelY())) /_C; 
		if(CheckRobot(player, _pos, _vel, acc, -1, pj, vj, Aj, type, limitTime) == UNSAFE) {
			crash_car_num = -1;
			float tc;
			if (Aj.sqlength() <= ACCMIN) {
				tc = _C * _CNum;
			}
			else {
				tc = (Aj.norm() * _VAMX - vj).length() / Aj.length();
				tc = min(_C * _CNum, tc);
			}
			crash_car_dir = pj + vector2f(tc * vj.x, tc * vj.y) + vector2f(0.5f * tc * tc * Aj.x, 0.5f * tc * tc * Aj.y) - _pos;
			return UNSAFE; // �����ײֱ�ӷ���
		}
	}
	return SAFE;
}
/************************************************************************/
/* modified 2011-12-22 by HQS
   ����С�����ٶȺͼ��ٹ���С����·��
   ��������С���Ƿ���ײ
/************************************************************************/
bool CDynamicSafetySearch::CheckRobot(const int player, vector2f pi, vector2f vi, vector2f Ai, const int obstacle, vector2f pj, vector2f vj, vector2f Aj, int type, const float limitTime) {
	// �ж�����С��֮���Ƿ��໥��ײ
	Trajectory Pi, Pj;
	float tradius = TEAMMATE_AVOID_DIST * sqrt(1 + 2 * vi.length() / ACCURATE_AOID); // ��ֹ������ʼλ���ѿ�ʼ����
	float tend = 0.0f; // С����ֹʱ��
	Pi._limitTime = limitTime;
	Pj._limitTime = limitTime;
	Pi.MakeTrajectory(player, pi, vi, Ai, 0.0f, _gamma, _stoptime, tradius ,_DECMAX, THISPLAYER); // �����Լ���·��
	float tc = min(vj.length() / _DECMAX, _C * _CNum);
	if (type == THEIRPLAYER) {
		tradius = OPP_AVOID_DIST * sqrt(1 + 2 * vj.length() / ACCURATE_AOID);
	}
	else if (type == OURPLAYER) {
		tradius = TEAMMATE_AVOID_DIST * sqrt(1 + 2 * vj.length() / ACCURATE_AOID) ;
	}
	else if (type == BALL) {
		tradius = BALL_AVOID_DIST * sqrt(1 + vj.length() / 800.0f) ;
	}
	Pj.MakeTrajectory(obstacle, pj, vj, Aj, 0.0f, tc, Pi._tend, tradius, _DECMAX, type); // �����ϰ����·��
	return Pi.CheckTrajectory(Pj);
}
/************************************************************************/
/* modified 2011-12-22 by HQS                                           */
/* �ڼ��ٶȿռ���������ɼ��ٶ�
   start_angle��end_angle��ʾ�ڹ涨��Χ�ڲ���������ٶ�
   max_radius��ʾ���ٿռ������ļ��ٶ�
/************************************************************************/
vector2f CDynamicSafetySearch::RandomAcc(float start_angle,float end_angle,float max_radius) {
	// ����ٶ�����Բ�İ뾶Զ���ڼ��ٶȿռ�Բ�İ뾶
	float theta,coef;
	float ax;
	float ay;
	float tc = _gamma; //Ԥ��ʱ�䳤��
	theta = (float(rand()) / RANDMAX) * (end_angle - start_angle) + start_angle;//����0~1�������
	coef = (float(rand()) / RANDMAX);
	ax = max_radius * cos(theta) * coef;
	ay = max_radius * sin(theta) * coef;

	if ((((ax + _vel.x / tc) * (ax + _vel.x / tc) + (ay + _vel.y / tc) * (ay + _vel.y / tc)) - _VAMX * _VAMX / (tc * tc) <= 0)) {
		return vector2f(ax, ay);
	}
	return vector2f(0.0f, 0.0f);
}

int compare (const void * a, const void * b) {
	if(*(const float*)a < *(const float*)b) {
		return -1;
	}
	return *(const float*)a > *(const float*)b;
}

inline vector2f CDynamicSafetySearch::CVector2vector2f(const CVector vec1) {
	return vector2f(vec1.x(), vec1.y());
}
inline vector2f CDynamicSafetySearch::CGeoPoint2vector2f(const CGeoPoint pos) {
	return vector2f(pos.x(), pos.y());
}
inline CVector CDynamicSafetySearch::vector2f2CVector(const vector2f vec1) {
	return CVector(vec1.x, vec1.y);
}
inline CGeoPoint CDynamicSafetySearch::vector2f2CGeoPoint(const vector2f pos) {
	return CGeoPoint(pos.x, pos.y);
}
/************************************************************************/
/* modified 2011-12-22 by HQS                                           */
/* ���ݲ�ͬ���͵�С�����ɲ�ͬ����Ĺ켣
   qlength��ʾ�켣�ĸ���,���3��
/************************************************************************/
void Trajectory::MakeTrajectory(const int player, const vector2f pos, const vector2f vel, const vector2f acc, const float t0, const float tc, const float tend, const float Radius, const float DMAX, const int type) {
	// ����켣
	_qlength = 0;
	if (type == THISPLAYER) {
		_type = THISPLAYER;
		_carnum = player;
		double x0 = pos.x;
		double y0 = pos.y;
		double vx0 = vel.x;
		double vy0 = vel.y;
		double t1 = t0 + tc;
		if (t1 > tend) {
			// ��С���Ѿ�����Ŀ�ĵ�ʱ,ֱ�Ӽ���
			double v0 = sqrt(vx0 * vx0 + vy0 * vy0);
			double ax0 =  - vx0 / v0 * DMAX;
			double ay0 =  - vy0 / v0 * DMAX;
			t1 = tend;
			q[_qlength].init(CCar(CPoint(x0, y0), vx0, vy0, ax0, ay0, Radius), t0, t1);
			_qlength++;
			_tend = t1;
		}
		else {
			double ax0 = acc.x;
			double ay0 = acc.y;
			q[_qlength].init(CCar(CPoint(x0, y0), vx0, vy0, ax0, ay0, Radius), t0, t1);
			_qlength++;
			double x1 = x0 + vx0 * tc + 0.5f * ax0 * tc * tc;
			double y1 = y0 + vy0 * tc + 0.5f * ay0 * tc * tc;
			double vx1 = vx0 + ax0 * tc;
			double vy1 = vy0 + ay0 * tc;
			double v1 = sqrt(vx1 * vx1 + vy1 * vy1);
			if (v1 <= VELMIN) { // ����ͣ
				_tend = t1;
				_qlength = 1;
				return;
			}
			double ax1 =  - vx1/v1 * DMAX;
			double ay1 =  - vy1/v1 * DMAX;
			double t2 = t1 + v1/DMAX;
			if (t2 > tend) {
				t2 = tend;
			}
			q[_qlength].init(CCar(CPoint(x1, y1), vx1, vy1, ax1, ay1, Radius), t1, t2);
			_qlength++;
			_tend = t2; //ȷ��������ʱ��
		}
	}
	else if (type == OURPLAYER) {
		_type = OURPLAYER;
		_carnum = player;
		if (vel.x < VELMIN && vel.y < VELMIN && acc.x < ACCMIN && acc.y < ACCMIN) { // С����ֹ
			double t1 = tend;
			double x0 = pos.x;
			double y0 = pos.y;
			double vx0 = 0.0f;
			double vy0 = 0.0f;
			double ax0 = 0.0f;
			double ay0 = 0.0f;
			q[_qlength].init(CCar(CPoint(x0, y0), vx0, vy0, ax0, ay0, Radius), t0, t1);
			_qlength++;
		}
		else {
			double t1 = t0 + tc;
			double x0 = pos.x;
			double y0 = pos.y;
			double vx0 = vel.x;
			double vy0 = vel.y;
			double ax0 = acc.x;
			double ay0 = acc.y;
			if (t1 >= tend) {
				t1 = tend;
				q[_qlength].init(CCar(CPoint(x0, y0), vx0, vy0, ax0, ay0, Radius), t0, t1);
				_qlength++;
				return;
			}
			q[_qlength].init(CCar(CPoint(x0, y0), vx0, vy0, ax0, ay0, Radius), t0, t1);
			_qlength++;
			double x1 = x0 + vx0 * tc + 0.5f * ax0 * tc * tc;
			double y1 = y0 + vy0 * tc + 0.5f * ay0 * tc * tc;
			double vx1 = vx0 + ax0 * tc;
			double vy1 = vy0 + ay0 * tc;
			double v1 = sqrt(vx1 * vx1 + vy1 * vy1);
			if (v1 <= EPSILON) { // С����ֹ
				double t2 = tend;
				q[_qlength].init(CCar(CPoint(x1, y1), 0.0f, 0.0f, 0.0f, 0.0f, Radius), t1, t2);
				_qlength++;
				return;
			}
			double ax1 =  - vx1 / v1 * DMAX;
			double ay1 =  - vy1 / v1 * DMAX;
			double t2 = t1 + v1 / DMAX;
			if (t2 >= tend) {
				t2 = tend;
				q[_qlength].init(CCar(CPoint(x1, y1), vx1, vy1, ax1, ay1, Radius), t1, t2);
				_qlength = 2;
			}
			else {
				q[_qlength].init(CCar(CPoint(x1, y1), vx1, vy1, ax1, ay1, Radius), t1, t2);
				_qlength++;
				double x2 = x1 + vx1 * (t2 - t1) + 0.5f * ax1 * (t2 - t1) * (t2 - t1);
				double y2 = y1 + vy1 * (t2 - t1) + 0.5f * ay1 * (t2 - t1) * (t2 - t1);
				double t3 = tend; // ����1sС���϶��ܾ�ֹ
				q[_qlength].init(CCar(CPoint(x2, y2), 0.0f, 0.0f, 0.0f, 0.0f, Radius), t2, t3);
				_qlength++;
			}
		}
	}
	else { // ���ڵз�С����ֻ�����������ٱ仯
		_type = type;
		_carnum = player;
		double t1 = tend;
		double x0 = pos.x;
		double y0 = pos.y;
		double vx0 = vel.x;
		double vy0 = vel.y;
		double ax0 = 0.0f;
		double ay0 = 0.0f;
		q[_qlength].init(CCar(CPoint(x0, y0), vx0, vy0, ax0, ay0, Radius), t0, t1);
		_qlength++;
	}
}
/************************************************************************/
/* modified 2011-12-22 by HQS                                           */
/* �ж������켣�Ƿ���ײ
/************************************************************************/
bool Trajectory::CheckTrajectory(const Trajectory& T1) { // ��������켣֮���Ƿ��໥��ײ
	if (T1._type == OURPLAYER) {
		if (((T1.q[0].pX - q[0].pX) * q[0].pVelX + (T1.q[0].pY - q[0].pY) * q[0].pVelY) <= 0 ) { // ��ʾ����С�����ٶȷ���ĺ��棬���ʾ��ȫ
			return SAFE;
		}
		else { // ����Ͱ��������ж������ж�
			for (int i = 0; i < _qlength; i++) {
				for(int j = 0; j < T1._qlength; j++) {
					CQuadraticEquation q1 = q[i];  
					CQuadraticEquation q2 = T1.q[j];
					CQuarticEquation q3 = q1 - q2;
					if (q3.getIsCrash()) {
						vector2f tdir = vector2f((q2.pX - q1.pX), (q2.pY - q1.pY));
						vector2f tdirgo = vector2f(q1.ax, q1.ay);
						float dir_angle = 1.0f;
						if (tdir.length() < EPSILON || tdirgo.length() < EPSILON) {
							dir_angle = 0.0f;
						}
						else {
							dir_angle = cosine(tdir, tdirgo);
						}
						/*cout<<"time:"<<q3.getRoot()<<endl;*/
						//�ж�С���Ƿ���ײ����Ҫ������ײ��ʱ�䣬�Լ���ײС�����ٶ����
						if ((q3.getRoot() >= _limitTime) || (q3.getRoot() <= _limitTime && i == 0 && dir_angle > 0.3f)) {
							return UNSAFE;
						}
					}
				}
			}
		}
	}
	else {
		for(int i = 0; i < _qlength; i++) {
			for( int j = 0; j < T1._qlength; j++) {
				CQuadraticEquation q1 = q[i];  
				CQuadraticEquation q2 = T1.q[j];
				CQuarticEquation q3 = q1 - q2;
				if (q3.getIsCrash()) {
					vector2f tdir = vector2f((q2.pX - q1.pX), (q2.pY - q1.pY));
					vector2f tdirgo = vector2f(q1.ax, q1.ay);
					float dir_angle = 1.0f;
					if (tdir.length() < EPSILON || tdirgo.length() < EPSILON) {
						dir_angle = 0.0f;
					}
					else {
						dir_angle = cosine(tdir, tdirgo);
					}
					//�ж�С���Ƿ���ײ����Ҫ������ײ��ʱ�䣬�Լ���ײС�����ٶ����
					if ((q3.getRoot() >= _limitTime) || (q3.getRoot() <= _limitTime && i == 0 && dir_angle > 0.3f)) {
						return UNSAFE;
					}
				}
			}
		}
	}
	return SAFE;
}