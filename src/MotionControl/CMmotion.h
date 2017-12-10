#ifndef CM_MOTION_H
#define CM_MOTION_H
#include <WorldDefine.h>
inline bool finite(double num)
{
	return fabs(num)<9999;
}
float motion_time_1d(float dx,float vel0,float vel1,
					 float max_vel,float max_accel,
					 float &t_accel,float &t_cruise,float &t_decel);
void compute_motion_1d(double x0, double v0, double v1,
					   double a_max, double d_max, double v_max, double a_factor,
					   double &traj_accel, double &traj_time, double &traj_time_acc, double &traj_time_dec, double &traj_time_flat);
void compute_motion_2d(CVector x0, CVector v0, CVector v1,
					   double a_max, double d_max, double v_max, double a_factor,
					   CVector &traj_accel, double &time);
double compute_stop(double v, double max_a);
void goto_point_omni( const PlayerPoseT& start,
					 const PlayerPoseT& final,
					 const PlayerCapabilityT& capability,
					 const double& accel_factor,
					 const double& angle_accel_factor,
					 PlayerPoseT& nextStep);
double expectedCMPathTime(const PlayerPoseT& start, const CGeoPoint& final, double maxAccel, double maxVelocity, double accel_factor);
double predictedTime(const PlayerVisionT& start, const CGeoPoint& Target);
#endif