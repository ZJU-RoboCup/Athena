#include <iostream>
#include <math.h>
#include <stdio.h>
#include <CMmotion.h>
#include <utils.h>
#include <fstream>
using namespace std;

namespace {
	const double FRAME_PERIOD = 1.0 / Param::Vision::FRAME_RATE;
	bool CARCONTROL = false;
	bool hasSetted = 0;
	ofstream carControlCommand("D://zjunlict//zeus2017ap//bin//play_books//carControlCommand.txt");
}

 ////////////////////////////////////////////////////////////////////////////////////////////////////
 /// @fn	void compute_motion_1d(double x0, double v0, double v1, double a_max, double v_max,
 /// 		double a_factor, double &traj_accel, double &traj_time)
 ///
 /// @brief	一维状态下计算运动过程，给定初始速度和末速度，最大加速度和最大速度，返回加速时间和过程时间
 ///
 /// @author	Song Zhibai( Song Zhibai Zju.edu.cn)
 /// @date		2010-4-2
 ///
 /// @param	x0				初始位置
 /// @param	v0				初速度
 /// @param	v1				末速度
 /// @param	a_max			最大加速度
 /// @param	v_max			最大速度
 /// @param	a_factor		常数，影响加速度的因子
 /// @param	traj_accel		计算得加速度
 /// @param	traj_time		计算得加速时间
 ////////////////////////////////////////////////////////////////////////////////////////////////////

void compute_motion_1d(double x0, double v0, double v1,
    double a_max, double d_max, double v_max, double a_factor,
    double &traj_accel, double &traj_time, double &traj_time_acc, double &traj_time_dec, double &traj_time_flat)
{
    if (x0 == 0 && v0 == v1)
    { 
        traj_accel = 0;
        traj_time_acc = traj_time_dec = 0;
        return; 
    }

    if(!finite(x0) || !finite(v0) || !finite(v1))
    {
        traj_accel = 0; 
		traj_time_acc = traj_time_dec = 0;
        return;
    }

    a_max /= a_factor;
	d_max /= a_factor;

    double accel_time_to_v1 = fabs(v1 - v0) / a_max;                                                  // 最大加速度加减速到末速度的时间
    double accel_dist_to_v1 = fabs((v1 + v0) / 2.0) * accel_time_to_v1;                               // 单一加速到末速度时的位移
    double decel_time_to_v1 = fabs(v0 - v1) / d_max;                                                  // 最大减速度减速到末速度的时间
    double decel_dist_to_v1 = fabs((v0 + v1) / 2.0) * decel_time_to_v1;                               // 单一减速到末速度时的位移

    // 这个时间很关键，设得较大则定位精度将大大降低 by qxz
    double period = 1 / 40.0; // 一段很小的时间，处理运动到目标点附近时加速度，稳定到点，防止超调


	// 计算时间部分

    // 从x0运动到零点
    // 初速和目标点反向 或 初速大于目标速来不及减速到目标速
    // 全程减速
    if (v0 * x0 > 0 || (fabs(v0) > fabs(v1) && decel_dist_to_v1 > fabs(x0))) 
    {
        // 停下后到达的时间 + 停下所用时间
        double time_to_stop = fabs(v0) / d_max;                                                       // 停下时间
        double x_to_stop = v0 * v0 / (2.0 * d_max);                                                   // 停止时运动距离

        compute_motion_1d(x0 + _copysign(x_to_stop, v0), 0, v1, a_max * a_factor, d_max * a_factor,
            v_max, a_factor, traj_accel, traj_time, traj_time_acc, traj_time_dec, traj_time_flat);    // 递归运算直到跳出这一条件
        traj_time += time_to_stop;                                                                    // 加上路径规划时间

        // 减速 
		if (traj_time < period)
			traj_accel = compute_stop(v0, d_max); // 很快就要到了，计算一个合适的加速度
		if (time_to_stop < period) {
			traj_accel = time_to_stop / period * -_copysign(d_max, v0) + (1.0 - time_to_stop / period) * traj_accel;
		}
		else {
			traj_accel = -_copysign(d_max, v0);
		}

        return;
    }

    // 初速和目标点同向

    if (fabs(v0) > fabs(v1))                                                                          // 初速度大于目标速，但可以减速到目标速 先加速再减速
    {
		traj_time_acc = (sqrt((d_max * v0 * v0 + a_max * (v1 * v1 + 2 * d_max * fabs(x0))) / (a_max + d_max)) - fabs(v0)) / a_max; 

        if (traj_time_acc < 0.0)
			traj_time_acc = 0;
		traj_time_dec = ((fabs(v0) - fabs(v1)) + a_max * traj_time_acc) / d_max;
    } 

    else if (accel_dist_to_v1 > fabs(x0))                                                             // 初速度小于目标速，且不可加速到目标速 全程加速
    {
		traj_time_acc = (sqrt(v0 * v0 + 2 * a_max * fabs(x0)) - fabs(v0)) / a_max;
		traj_time_dec = 0.0;
    }

    else                                                                                              // 初速度小于目标速，且可以加速到目标速 先加速再减速
    {
		traj_time_acc = (sqrt((d_max * v0 * v0 + a_max * (v1 * v1 + 2 * d_max * fabs(x0))) / (a_max + d_max)) - fabs(v0)) / a_max;
        if (traj_time_acc < 0.0)
			traj_time_acc = 0;
		traj_time_dec = ((fabs(v0) - fabs(v1)) + a_max * traj_time_acc) / d_max;
    }

    // 计算所得车速可能超过车最大速度，会有一段匀速运动

    if (traj_time_acc * a_max + fabs(v0) > v_max)                                                     // 匀速运动的时间
    {
		double v_max1 = v_max - a_max * FRAME_PERIOD;
		double dist_without_flat = (v_max1 * v_max1 - v0 * v0) / (2 * a_max) + (v_max1 * v_max1 - v1 * v1) / (2 * d_max);
		traj_time_flat = (fabs(x0) - dist_without_flat) / v_max;
    }
	else traj_time_flat = 0;

	// 分配加速度部分

	double a_to_v1_at_x0 = fabs(v0 * v0 - v1 * v1) / (2 * fabs(x0));
	double t_to_v1_at_x0 = (-fabs(v0) + sqrt(v0 * v0 + 2 * fabs(a_to_v1_at_x0) * fabs(x0))) / fabs(a_to_v1_at_x0);
	if (t_to_v1_at_x0 < period) {
		traj_accel = -_copysign(a_to_v1_at_x0, v0);
		return;
	}

	if (FRAME_PERIOD * a_max + fabs(v0) > v_max && traj_time_flat > period) {                         // 匀速运动阶段
		traj_time = traj_time_flat / 1.1 + traj_time_dec / 1.1;
		traj_accel = 0;
	}
    else if (traj_time_acc < period && traj_time_dec == 0.0)                                          // 加速到点
    {
		traj_time = traj_time_acc;
        traj_accel = _copysign(a_max * a_factor, -x0);
    }
    else if (traj_time_acc < period && traj_time_dec > 0.0)                                           // 加速接近结束且需减速
    {
		traj_time = traj_time_dec;
		traj_accel = _copysign(d_max, -v0);
    }
    else
	{
		traj_time = traj_time_acc + traj_time_flat / 1.4 + traj_time_dec / 1.4;
        traj_accel = _copysign(a_max * a_factor, -x0);
    }
}

 ////////////////////////////////////////////////////////////////////////////////////////////////////
 /// @fn	void compute_motion_2d(CVector x0, CVector v0, CVector v1, double a_max, double v_max,
 /// 		double a_factor, CVector &traj_accel, double &time)
 ///
 /// @brief 在二维下计算运动过程，给定初始速度向量和最终速度向量，以及最大加速度，最大速度
 ///
 /// @author	Song Zhibai( Song Zhibai Zju.edu.cn)
 /// @date		2010-4-2
 ///
 /// @param	x0						initial coordinate
 /// @param	v0						initial velocities
 /// @param	v1						final velocities 
 /// @param	a_max					maxium of accelerate
 /// @param	v_max					maxium of velocities
 /// @param	a_factor				factor of accelerate
 /// @param [in,out]	traj_accel	the traj accel. 
 /// @param [in,out]	time		the time. 
 ////////////////////////////////////////////////////////////////////////////////////////////////////

void compute_motion_2d(CVector x0, CVector v0, CVector v1,
                    double a_max, double d_max, double v_max, 
                    double a_factor, CVector &traj_accel, double &time)
{
  double time_x, time_x_acc, time_x_dec, time_x_flat;
  double time_y, time_y_acc, time_y_dec, time_y_flat;
  double rotangle;
  double traj_accel_x;
  double traj_accel_y;
  if (v1.mod() == 0)
	rotangle = x0.dir();
  else rotangle = v1.dir();

  x0 = x0.rotate(-rotangle);
  v0 = v0.rotate(-rotangle);
  v1 = v1.rotate(-rotangle); //坐标系转换，转换到末速度方向为x轴的坐标系中

  compute_motion_1d(x0.x(), v0.x(), v1.x(), a_max, d_max, v_max, a_factor,
		    traj_accel_x, time_x, time_x_acc, time_x_dec, time_x_flat);
  compute_motion_1d(x0.y(), v0.y(), v1.y(), a_max, d_max, v_max, a_factor,
		    traj_accel_y, time_y, time_y_acc, time_y_dec, time_y_flat);//两轴同样的最大速度、加速度独立考虑求两轴运动时间


  traj_accel = CVector(traj_accel_x,traj_accel_y);
  if (traj_accel.mod())
	traj_accel = traj_accel.rotate(rotangle);
  time = time_x;
}

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  /// @fn	double compute_stop(double v, double max_a)
  ///
  /// @brief	Calculates the stop. 
  ///
  /// @author	Song Zhibai( Song Zhibai Zju.edu.cn)
  /// @date		2010-4-2
  ///
  /// @param	v		The v. 
  /// @param	max_a	The maximum a. 
  ///
  /// @return	The calculated stop. 
  ////////////////////////////////////////////////////////////////////////////////////////////////////

double compute_stop(double v, double max_a)
{
	if (fabs(v) > max_a * FRAME_PERIOD) return _copysign(max_a, -v); // 如果一帧内不能减速到0，就用最大加速度
	else return -v / FRAME_PERIOD; // 如果能减到0，加速度就用速度除以一帧的时间即可
}

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  /// @fn	void goto_point_omni( const PlayerPoseT& start, const PlayerPoseT& final,
  /// 		const PlayerCapabilityT& capability, const double& accel_factor,
  /// 		const double& angle_accel_factor, PlayerPoseT& nextStep)
  ///
  /// @brief	给定初始坐标，结束坐标及小车性能，计算小车可能的轨迹，更新下一个位置的机器人状态
  ///
  /// @author	Song Zhibai( Song Zhibai Zju.edu.cn)
  /// @date		2010-4-2
  ///
  /// @param	start				The start. 
  /// @param	final				The final. 
  /// @param	capability			The capability. 
  /// @param	accel_factor		The accel factor. 
  /// @param	angle_accel_factor	The angle accel factor. 
  /// @param [in,out]	nextStep	the next step. 
  ////////////////////////////////////////////////////////////////////////////////////////////////////

  void goto_point_omni( const PlayerPoseT& start,
	  const PlayerPoseT& final,
	  const PlayerCapabilityT& capability,
	  const double& accel_factor,
	  const double& angle_accel_factor,
	  PlayerPoseT& nextStep)
  {
	  CGeoPoint target_pos = final.Pos();
	  CVector x = start.Pos() - target_pos;
	  CVector v = start.Vel();
	  double ang = Utils::Normalize(start.Dir() - final.Dir());
	  double ang_v = start.RotVel();
      CVector target_vel = final.Vel();
	  double max_accel = capability.maxAccel;
      double max_decel = capability.maxDec;
	  double max_speed = capability.maxSpeed;
	  double max_angle_speed = capability.maxAngularSpeed;
	  double max_angle_accel = capability.maxAngularAccel;
      double max_angle_decel = capability.maxAngularDec;
	  CVector a;
	  double ang_a, factor_a;
	  double time_a, time_a_acc, time_a_dec, time_a_flat, time;
	  if (x.mod() < 5) {
		  max_accel /= 2;
		  max_decel /= 2;
	  }
	  compute_motion_2d(x, v, target_vel, max_accel, max_decel, max_speed, accel_factor, a, time);
	  factor_a = 1;
	  compute_motion_1d(ang, ang_v, 0.0, max_angle_accel, max_angle_decel, max_angle_speed, angle_accel_factor, ang_a, time_a, time_a_acc, time_a_dec, time_a_flat);
	  //cout << start.Vel() << " " << final.Vel() << endl;

	  v = v + a * FRAME_PERIOD;
	  ang_v += ang_a * FRAME_PERIOD;
	  double delta_ang = ang_v * FRAME_PERIOD * 4;
	  v.rotate(delta_ang);
	  if (v.mod() > max_speed)
		  v = v* max_speed/v.mod();
	  if (ang_v>max_angle_speed)
	  {
		  ang_v = max_angle_speed;
	  }
      else if (ang_v<-max_angle_speed)
	  {
		  ang_v = -max_angle_speed;
	  }
	  CGeoPoint next_pos = start.Pos()+Utils::Polar2Vector(v.mod()*FRAME_PERIOD,v.dir());
	  double next_angle = start.Dir()+ang_v*FRAME_PERIOD;
      double t =  std::max(time,time_a);
	  //static double t_last;
	  //static int tmpCnt;
	  //cout << ++tmpCnt << " " << t << " " << t_last - t << " " << x << endl;
	  //t_last = t;
	  nextStep.SetValid(true);
	  nextStep.SetPos(next_pos);
	  nextStep.SetDir(next_angle);
	  nextStep.SetVel(v);
	  carControlCommand << v.x() << " " << v.y() << endl;
	  nextStep.SetRotVel(ang_v);
  }

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	double expectedCMPathTime(const PlayerPoseT& start, const CGeoPoint& final,
/// 	double maxAccel, double maxVelocity, double accel_factor)
///
/// @brief	调用 compute_motion_2d
///
/// @author	Song Zhibai( Song Zhibai Zju.edu.cn)
/// @date	2010-4-2
///
/// @param	start			The start. 
/// @param	final			The final. 
/// @param	maxAccel		The maximum accel. 
/// @param	maxVelocity		The maximum velocity. 
/// @param	accel_factor	The accel factor. 
///
/// @return	. 
////////////////////////////////////////////////////////////////////////////////////////////////////

double expectedCMPathTime(const PlayerPoseT& start, const CGeoPoint& final, double maxAccel, double maxVelocity, double accel_factor)
{
	CGeoPoint target_pos = final;
	CVector x = start.Pos() - target_pos;
	CVector v = start.Vel();
	CVector target_vel = CVector(0,0);
	CVector a;  
	double time;
	compute_motion_2d(x, v, target_vel, 
		maxAccel, 
        maxAccel,
		maxVelocity,
		accel_factor,
		a, time);
    return time;
}

double predictedTime(const PlayerVisionT& start, const CGeoPoint & Target)
{
	CVector x = start.Pos() - Target;
	CVector v = start.Vel();
	CVector target_vel = CVector(0, 0);
	double time;
	CVector a;
	compute_motion_2d(x, v, target_vel, 500, 500, 300, 1, a, time);
	return time;
}

