#include <iostream>
#include <math.h>
#include <utils.h>
#include <param.h>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include "TrapezoidalVelTrajectory.h"
#include <TimeCounter.h>

//#define _PRINT_DEBUG_MSG
#ifdef _PRINT_DEBUG_MSG
#define PRINT_DEBUG(x) std::cout << #x << " : " << (x) << std::endl;
#define PRINT_MSG(x) std::cout << x << std::endl;
#else 
#define PRINT_DEBUG(x)
#define PRINT_MSG(x)
#endif

//static CVector VEL_LOG[2] = {CVector(-9999,-9999), CVector(-9999, -9999)};
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	vector<vector<double>> tg_recurs(double t0, double z0, double zf, double vz0, int rev,
/// 	int iter, double uza, double uzd, double vzMax)
///
/// @brief ���ٵ���滮�������㷨
///       
///        ���� t0 ʱ�̵Ļ�����״̬������õ�����㵽�յ�֮��������滮���̡�
///     �÷�������������ڼ��ٺͼ��ٹ����У������˵ļ��ٶ��Ǻ㶨����ġ�
///     ���������Ļ����ϣ����ݻ����˵ĳ�ʼ״̬, �������˵��˶���Ϊ��
///     ���ࣺ
///        1: ���ٶ���λ�Ʒ���һ�£��Ƚ����ٵ� 0
///        2: ���ٶ���λ�Ʒ���һ��
///           2.1: ���ٶ�С������ٶ� Vmax, �Ҽ���������ٶ�ʱλ�Ʋ���
///         ����Ŀ��㣬���ȼ���������ٶȡ�
///           2.2: ��������ٶ��˶�
///           2.3: �������ٶȼ��٣�Ҳ�ᳬ��Ŀ��㣬���������ټ���
///        3: ���ٴ�������ٶȣ��ȼ��ٵ� Vmax
/// 
///     
/// 
/// @author	Yonghai Wu
/// @date	2009-12-9
///
/// @param	t0		initial time = absolute time when this stage begins
/// @param	z0		initial position 
/// @param	zf		final position 
/// @param	vz0		initial velocity. 
/// @param	rev		The reverse. 
/// @param	iter	The iterator. 
/// @param	uza		the acceleration. 
/// @param	uzd		The decelerate. 
/// @param	vzMax	The vz maximum. 
///
/// @return	���ع滮�õ���·����·���ɶ����ɣ�ÿ�ΰ������²�����
/// 
///         z0U: ��ʼλ��
///        vz0U: ��һ�׶εĳ��ٶ�
///          az: ��һ�׶εļ��ٶ�
///         rev: �Ƿ�ı�ο�����
///           t: ��һ�׶�����ʱ��
///         t0U: ����ʱ�䣨��������һ�׶Σ�
///
////////////////////////////////////////////////////////////////////////////////////////////////////

vector<vector<double>> tg_recurs(double t0, 
								 double z0, 
								 double zf, 
								 double vz0, 
								 int rev, 
								 int iter, 
								 double uza,
								 double uzd,
								 double vzMax)
{
	//std::cout<<"TrapezoidalVelTrajectory"<<std::endl;
	double tol = 0.01;   // tolerance for machine precision
	double zfU = zf;
	double z0U = z0;
	double vz0U = vz0;
	double az;
	double t=0;
	double t0U = 0;
	vector<vector<double> > DataVector;
	iter = iter + 1;
	if (abs(z0)!=0)
	{
		// Adjust offset for z0
		zfU = zf-z0;
		z0U = 0;
		vz0U = vz0;
		az = 0;
		t = 0;
	} 
	while ( (abs(zfU)>=tol) || (abs(vz0U)>=tol) )
	{
		//std::cout<<"zfU:"<<zfU<<"vz0U:"<<vz0U<<std::endl;
		vz0 = vz0U;
		if (zfU<0){
			//Destination in wrong direction, need to flip the sign
			PRINT_MSG("Flip direction");
			z0U = 0;
			zfU = -zfU;
			vz0U = -vz0U;
			az = 0;
			t = 0;
			if (rev==1)
				rev = -1;
			else
				rev = 1;
		}
		else if ( vz0U > vzMax ){
			//Case 3: Need to decelerate until vz = vzMax
			PRINT_MSG("Case 3");
			t = (vz0U-vzMax)/uzd;
			z0U = 0;
			zfU = zfU - (vz0U*vz0U-vzMax*vzMax)/2.0/uzd;
			vz0U = vzMax;
			az = -uzd;
		}
		else if (vz0U<0){
			// ��Ҫ�����Ȱ�ԭ�����ٶȼ�����
			//Case 1: Moving in wrong direction, need to decelerate first
			PRINT_MSG("Case 1");
			t = abs(vz0U/uzd);
			double zDelta = vz0U*t + uzd*t*t/2.0;
			z0U = 0;
			zfU = zfU-zDelta;
			vz0U = 0; // vz0 + uzd*t; // "correct" solution isn't great, rounding errors
			az = uzd;
		}    
		else if (zfU<=(vz0U*vz0U/2.0/uzd + tol)){
			// ���������������ٶȼ��٣�Ҳ�ᳬ��Ŀ���
			// Case 2.3: Need to decelerate
			PRINT_MSG("Case 2.3");
			t = vz0U/uzd;
			double zDelta = vz0U*vz0U/2.0/uzd;
			z0U = 0;
			zfU = zfU-zDelta;
			vz0U = 0;
			az = -uzd;
		}
		else if ( fabs(vz0U - vzMax) < tol ){
			// ���㱣������ٶȵ�ʱ�䣬���ʱ���˶��ľ��룬��������ʱʣ�µ�·��
			//Case 2.2: Cruising at vzMax until you have to decelerate
			PRINT_MSG("Case 2.2");
			z0U = 0;
			double zDelta = vzMax*vzMax/2.0/uzd;		
			vz0U = vzMax;
			az = 0;
			double zCruise = zfU - zDelta;
			t = zCruise / vzMax;
			zfU = vzMax*vzMax/2.0/uzd;
			PRINT_DEBUG(t);
		}    
		else{
			// Case 2.1: Accelerate
			PRINT_MSG("Case 2.1");
			// Accelerate to vzMax
			double t1 = (vzMax - vz0U)/uza;

			// Accelerate without reaching vzMax
			double zDelta = vz0U*vz0U/2.0/uza;
			float temp = (float)( (zfU + zDelta)/(1.0/2/uza + 1.0/2/uzd) );
			double vz2 = Utils::SquareRootFloat( temp );
			double t2 = (vz2-vz0U)/uza;
			if (t1<t2){
				t = t1;
				z0U = 0;
				zfU = zfU - vz0U*t1 - uza/2.0*t1*t1;
				vz0U = vzMax;
				az = uza;
			}
			else{
				t = t2;
				z0U = 0;
				zfU = zfU- vz0U*t2 - uza/2.0*t2*t2;
				vz0U = vz2;
				az = uza;
			}
		}
		// zfU: ʣ��·��
		// vz0: ���ٶ�
		// vz0U: ��һ�׶εĳ��ٶ�
		// az: ��һ�׶εļ��ٶ�
		// rev: �Ƿ�ı�ο�����
		// t: ��һ�׶�����ʱ��
		// t0U: ����ʱ�䣨��������һ�׶Σ�
		double data[6] = {z0U, vz0, az, rev, t, t0U};
		vector<double> storeData(data,data+6);
		DataVector.push_back(storeData);
		t0U = t + t0U;   //Initial time for the next stage

		if ( vz0U == vzMax && vzMax < 0 )
			break;
	}
	double data[6] = {0, 0, 0, rev, 0, t0U};
	vector<double> storeData(data,data+6);
	DataVector.push_back(storeData);
	return DataVector;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	vector< vector<double> > tg_recurs(double t0, double z0, double zf, double vz0,
/// 	double vzf, int rev, int iter, double uza, double uzd, double vzMax)
///
/// @brief	�����ٵ���滮��
///  
///         ???? ��Ŀ����趨���������˵��˶�ѧԼ������������ǻ����Ǻ�ϸ�£��п��ܻ�����⡣
///        ���� t0 ʱ�̵Ļ�����״̬������õ�����㵽�յ�֮��������滮���̡�
///     �÷�������������ڼ��ٺͼ��ٹ����У������˵ļ��ٶ��Ǻ㶨����ġ�
///     ���������Ļ����ϣ����ݻ����˵ĳ�ʼ״̬, �������˵��˶���Ϊ��
///     ���ࣺ
///        1: ���ٶ���λ�Ʒ���һ�£��Ƚ����ٵ� 0
///        2: ���ٶ���λ�Ʒ���һ��
///           2.1: ���ٶ�С������ٶ� Vmax, �Ҽ���������ٶ�ʱλ�Ʋ���
///         ����Ŀ��㣬���ȼ���������ٶȡ�
///           2.2: ��������ٶ��˶�
///           2.3: �������ٶȼ��٣�Ҳ�ᳬ��Ŀ��㣬���������ټ���
///        3: ���ٴ�������ٶȣ��ȼ��ٵ� Vmax
///
/// @author	Yonghai Wu
/// @date	2009-12-9
///
/// @param	t0		initial time = absolute time when this stage begins
/// @param	z0		initial position 
/// @param	zf		final position 
/// @param	vz0		initial velocity.
/// @param  vzf     final velocity. 
/// @param	rev		The reverse. 
/// @param	iter	The iterator. 
/// @param	uza		the acceleration. 
/// @param	uzd		The decelerate. 
/// @param	vzMax	The vz maximum. 
///
/// @return	���ع滮�õ���·����·���ɶ����ɣ�ÿ�ΰ������²�����
/// 
///         z0U: ��ʼλ��
///        vz0U: ��һ�׶εĳ��ٶ�
///          az: ��һ�׶εļ��ٶ�
///         rev: �Ƿ�ı�ο�����
///           t: ��һ�׶�����ʱ��
///         t0U: ����ʱ�䣨��������һ�׶Σ�. 
////////////////////////////////////////////////////////////////////////////////////////////////////

vector< vector<double> > tg_recurs(double t0,
								   double z0,
								   double zf,
								   double vz0,
								   double vzf,
								   int rev,
								   int iter,
								   double uza,
								   double uzd,
								   double vzMax)
{
	// zfU: ʣ��·��
	// vz0: ���ٶ�
	// vz0U: ��һ�׶εĳ��ٶ�
	// az: ��һ�׶εļ��ٶ�
	// rev: �Ƿ�ı�ο�����
	// t: ��һ�׶�����ʱ��
	// t0U: ����ʱ�䣨��������һ�׶Σ�
	const double tol = 0.01;   // tolerance for machine precision
	const double pos_tol = vzMax/100;
	const double vel_tol = vzMax/100;
	double zfU = zf;
	double z0U = z0;
	double vz0U = vz0;
	double az;
	double t=0;
	double t0U = 0;
	vector<vector<double> > DataVector;
	PRINT_DEBUG(z0);
	PRINT_DEBUG(zf);
	PRINT_DEBUG(vz0);
	PRINT_DEBUG(vzf);
	iter = iter + 1;
	int numOfStep = 0;  // 
	bool canReach = true;

	if (fabs(z0)!=0)
	{
		// Adjust offset for z0
		zfU = zf-z0;
		z0U = 0;
		vz0U = vz0;
		az = 0;
		t = 0;
	}
	/// check if the robot can reach the target

	while ( ((fabs(zfU)>= pos_tol) || (fabs(vz0U-vzf)>= vel_tol)) && (numOfStep < 10) )
	{
		//std::cout<<"zfU:"<<zfU<<"vz0U:"<<vz0U<<std::endl;
		vz0 = vz0U;
		if (zfU<0 && 0 == numOfStep){
			// Destination in wrong direction, need to flip the sign
			PRINT_MSG("Flip direction");
			z0U = 0;
			zfU = -zfU;
			vz0U = -vz0U; 
			vzf = -vzf;
			az = 0;
			t = 0;
			if (rev==1)
				rev = -1;
			else
				rev = 1;
		}

		else if (vz0U<0){
			// ��Ҫ�����Ȱ�ԭ�����ٶȼ�����
			//Case 1: Moving in wrong direction, need to decelerate first
			PRINT_MSG("Case 1");
			t = fabs(vz0U/uzd);
			double zDelta = vz0U*t + uzd*t*t/2.0;
			z0U = 0;
			zfU = zfU-zDelta;
			vz0U = 0; // vz0 + uzd*t; // "correct" solution isn't great, rounding errors
			az = uzd;
			PRINT_DEBUG(t);
			PRINT_DEBUG(zfU);
			PRINT_DEBUG(vz0U);
		}    
		else if (zfU<=((vz0U*vz0U - vzf*vzf)/2.0/uzd + pos_tol)){
			// ���������������ٶȼ��٣�Ҳ�ᳬ��Ŀ���
			// Case 2.3: Need to decelerate
			PRINT_MSG("Case 2.3");

			double zDelta = (vz0U*vz0U - vzf*vzf)/2.0/uzd;

			t = (vz0U-vzf)/uzd;
			zfU = zfU-zDelta;
			vz0U = vzf;
			//	az = -uzd;
			PRINT_DEBUG(t);
			PRINT_DEBUG(zfU);
			PRINT_DEBUG(vz0U);

			z0U = 0;
			az = -uzd;
		}
		else if ( fabs(vz0U - vzMax) < vel_tol ){
			// ���㱣������ٶȵ�ʱ�䣬���ʱ���˶��ľ��룬��������ʱʣ�µ�·��
			//Case 2.2: Cruising at vzMax until you have to decelerate
			PRINT_MSG("Case 2.2");
			z0U = 0;
			double zDelta = (vzMax*vzMax - vzf*vzf)/2.0/uzd;		
			vz0U = vzMax;
			az = 0;
			double zCruise = zfU - zDelta;
			t = zCruise / vzMax;
			zfU = (vzMax*vzMax - vzf*vzf)/2.0/uzd;
			PRINT_DEBUG(t);
			PRINT_DEBUG(zfU);
			PRINT_DEBUG(vz0U);
		}    
		else{
			// Case 2.1: Accelerate
			PRINT_MSG("Case 2.1");
			// Accelerate to vzMax
			double zDelta = ( vzf*vzf-vz0U*vz0U)/2/uza;

			if( vz0U < vzf && zfU < zDelta)
			{
				z0U = 0;
				double vz2 = sqrt( vz0U*vz0U+2*uza*zfU);
				t = (vz2-vz0U)/uza;
				vz0U = vz2;
				zfU = 0;
				vzf = vz0U;
				az = uza;
			}
			else{
				double t1 = (vzMax - vz0U)/uza;

				// Accelerate without reaching vzMax
				double zDelta = vz0U*vz0U/2.0/uza + vzf*vzf/2.0/uzd;
				float temp = (float)( (zfU + zDelta)/(1.0/2/uza + 1.0/2/uzd) );
				double vz2 = Utils::SquareRootFloat( temp );
				double t2 = (vz2 - vz0U)/uza;
				if (t1<t2){
					t = t1;
					z0U = 0;
					zfU = zfU - vz0U*t1 - uza/2.0*t1*t1;
					vz0U = vzMax;
					az = uza;
					PRINT_DEBUG(t1);
					PRINT_DEBUG(zfU);
					PRINT_DEBUG(vz0U);
				}
				else{
					t = t2;
					z0U = 0;
					zfU = zfU- vz0U*t2 - uza/2.0*t2*t2;
					vz0U = vz2;
					az = uza;
					PRINT_DEBUG(t2);
					PRINT_DEBUG(zfU);
					PRINT_DEBUG(vz0U);
				}
			}
		}
		numOfStep++;
		PRINT_DEBUG( numOfStep );
		double data[6] = {z0U, vz0, az, rev, t, t0U};
		vector<double> storeData(data,data+6);
		DataVector.push_back(storeData);
		t0U = t + t0U;   //Initial time for the next stage

		if ( vz0U == vzMax && vzMax < 0 )
			break;
	}
	double data[6] = {0, 0, 0, rev, 0, t0U};
	vector<double> storeData(data,data+6);
	DataVector.push_back(storeData);
	return DataVector;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void buildPath(const vector<vector<double>>& storeData,double& z, double& vz,
/// 	double& azList)
///
/// @brief	���ݺ��� <tg_recurs> �Ľ��������һ���ڵ�λ�� z���ٶ� vz �ͼ��ٶ� azList��
///           
///         1. �����㷨�Ļ������裬���ٶȺ㶨����˼��ٶ� a = storeData[stage-1][2];
///         2. �ٶ� v = v0 + a*dt
///         3. λ�� z= z0 + v*dt
/// 
/// @author	Yonghai Wu
/// @date	2009-12-9
///
/// @param	storeData		Information describing the pathe. 
/// @param [in,out]	z		the next postion. 
/// @param [in,out]	vz		the next velocity. 
/// @param [in,out]	azList	the next acceration. 
////////////////////////////////////////////////////////////////////////////////////////////////////

void buildPath(const vector<vector<double>>& storeData,double& z, double& vz, double& azList)
{
	double num_stages = storeData.size();
	vector<double> data = storeData[num_stages-1];
	double Duration = data[5]; // �˶�������ʱ��
	int stage = 0;
	z = storeData[0][0];
	int tfint = Duration*Param::Vision::FRAME_RATE;
	int	maxStepNumber=(std::min)(tfint, 1);
	for (int index = 0;index<=maxStepNumber;index++){
		if (index>=Duration*Param::Vision::FRAME_RATE)
			continue;
		while (index>=storeData[stage][5]*Param::Vision::FRAME_RATE)
			stage = stage + 1;		
		double t0 = storeData[stage-1][5] ;
		double vz0 = storeData[stage-1][1];
		double az  = storeData[stage-1][2];
		double rev = storeData[stage-1][3];
		azList = rev*az;	
		vz = rev*vz0 + rev*az * (index*1.0/Param::Vision::FRAME_RATE-t0);
		if (index!=0)
			z+=vz0/Param::Vision::FRAME_RATE;	
	}
	// This is to prevent errors when duration = 0
	if (tfint == 0){
		//std::cout<<"duration = 0!!!!!"<<endl;
		z      = storeData[0][0];
		vz     = storeData[0][1];
		azList = storeData[0][2];
		//std::cout<<"tfint = 0 "<<z<<" "<<vz<<" "<<azList<<endl;
	}	
	//std::cout<<"stage:"<<stage<<" "<<z<<" "<<vz<<" "<<vz/Param::Vision::FRAME_RATE<<endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void buildPath(const vector< vector<double> >& storeData, double& z, double& vz,
/// 	double& azList, vector< vector<double> >& pathList)
///
///    ���ݺ��� <tg_recurs> �Ľ��������һ���ڵ�λ�� z���ٶ� vz �ͼ��ٶ� azList ������·�� pathList��
///           
///         1. �����㷨�Ļ������裬���ٶȺ㶨����˼��ٶ� a = storeData[stage-1][2];
///         2. �ٶ� v(t) = v0 + a*dt
///         3. λ�� z(t) = z0 + v(t)*dt
///
/// @author	Yonghai Wu
/// @date	2009-12-9
///
/// @param	storeData			Information describing the store. 
/// @param [in,out]	z			the z coordinate. 
/// @param [in,out]	vz			the vz. 
/// @param [in,out]	azList		list of azs. 
/// @param [in,out]	pathList	list of pathes. 
////////////////////////////////////////////////////////////////////////////////////////////////////

void buildPath(const vector< vector<double> >& storeData,
			   double& z,
			   double& vz,
			   double& azList,
			   vector< vector<double> >& pathList)
{
	double num_stages = storeData.size();
	vector<double> data = storeData[num_stages-1];
	double Duration = data[5]; // �˶�������ʱ��
	int stage = 0;
	double z_t, vz_t, azList_t;
	z_t = storeData[0][0];
	int tfint = Duration*Param::Vision::FRAME_RATE;
	int	maxStepNumber = (std::min)( 60, (std::max)(tfint, 1));
	int indexStep = 6;
	for (int index = -1;index<=maxStepNumber;){
		if( index < 1 )
			index ++;
		else
			index += indexStep;

		if (index>=Duration*Param::Vision::FRAME_RATE)
			continue;
		while (index>=storeData[stage][5]*Param::Vision::FRAME_RATE)
			stage = stage + 1;		
		double t0  = storeData[stage-1][5];
		double vz0 = storeData[stage-1][1];
		double az  = storeData[stage-1][2];
		double rev = storeData[stage-1][3];
		azList_t = rev*az;	
		vz_t = rev*vz0 + rev*az * (index*1.0/Param::Vision::FRAME_RATE-t0);
		if (index!=0 ){
			if( index == 1 )
				z_t+=vz_t/Param::Vision::FRAME_RATE;
			else 
				z_t+=indexStep*vz_t/Param::Vision::FRAME_RATE;
		}

        if ( 1 == index)
        {
             z = z_t;
			 vz = vz_t;
			 azList = azList_t;
			 //std::cout<<"stage:"<<stage<<" index: "<<index<<" "<<z<<" "<<vz<<" "<<azList<<endl;
        }
        double data[3] = { z_t, vz_t, azList_t };
		vector<double> storedata(data, data+3);
		pathList.push_back(storedata);
	//	std::cout<<"stage:"<<stage<<" index: "<<index<<" "<<z<<" "<<vz<<" "<<azList<<endl;

		
	// This is to prevent errors when duration = 0
	if (tfint <= 1){
		//std::cout<<"duration = 0!!!!!"<<endl;
		z      = storeData[0][0];
		vz     = storeData[0][1];
		azList = storeData[0][2];
		//std::cout<<"tfint = 0"<<z<<" "<<vz<<" "<<azList<<endl;
		//std::cout<<z<<" "<<vz<<" "<<azList<<endl;
		}

	}
	}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	double syncTG(const PlayerPoseT& start, CGeoPoint finalPos,
/// 	const PlayerCapabilityT& capability)
///
/// @brief	���ö��ַ������� x �� y  ���������ٶȺ������ٶȣ��Դﵽ x, y �������õ�ʱ��
///         ���. ���ַ������ı���Ϊ alpha, ��仯��Χ�� [0, pi/2] 
///
/// @author	Yonghai Wu
/// @date	2009-12-9
///
/// @param	start		The start. 
/// @param	finalPos	The final position. 
/// @param	capability	The capability. 
///
/// @return	����������·�����õ�ʱ��. 
////////////////////////////////////////////////////////////////////////////////////////////////////

double syncTG(const PlayerPoseT& start, CGeoPoint finalPos,const PlayerCapabilityT& capability)
{
	// Do a binary search to find the proper control effort, such that the times for the
	// trajectories in x and y direction are the same.
	// The variable to vary is "alpha", which runs from 0 to pi/2
	//The return value is the expected path time.
	const double factor = 1.2;

	double x0      = start.X();
	double y0      = start.Y();
	double xf      = finalPos.x();
	double yf      = finalPos.y();
	double vx0     = start.VelX();
	double vy0     = start.VelY();
    
	double maxSpeed = capability.maxSpeed;
	double maxAccel = capability.maxAccel;
	double maxAngleSpeed = capability.maxAngularSpeed;
	double maxAngleAccel = capability.maxAngularAccel;

	int numIter = 10;       //number of iterations performed. error in u is 2^(-numIter)
	double u   =  1.568;    // approx. PI/2
	double du  = -1.568;
    double alpha,durX,durY;
	for (int j=1;j<=numIter;j++)
	{
		du = du*0.5;
		alpha = u+du;
		double axMax = sin(alpha)*maxAccel;
		double ayMax = cos(alpha)*maxAccel;
		double vxMax = sin(alpha)*maxSpeed;
		double vyMax = cos(alpha)*maxSpeed;

		//Find duration for x-direction    
		int iter = 0;
		double t0 = 0;
		double rev = 1;
		vector<vector<double> > storeDataX = tg_recurs(t0, x0, xf, vx0, rev, iter, axMax, factor*axMax, vxMax);
		double num_stagesX = storeDataX.size();
		durX = storeDataX[num_stagesX-1][5];
		//Find duration for y-direction    
		iter = 0;
		t0 = 0;
		rev = 1;
		vector<vector<double> > storeDataY = tg_recurs(t0, y0, yf, vy0, rev, iter, ayMax, factor*ayMax, vyMax);
		double num_stagesY = storeDataY.size();
		durY =  storeDataY[num_stagesY-1][5];
		double tfDelta = durX-durY;
		//std::cout<<"alpha:"<<alpha<<"durx:"<<durX<<"dury:"<<durY<<endl;
		if (tfDelta <= 0.0)
			u=alpha;
	}
	double duration = (durX+durY)/2;
	return duration;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void trapezoidalVelocityPath( const PlayerPoseT& start, const PlayerPoseT& final,
/// 	double frameRate, const PlayerCapabilityT& capability, PlayerPoseT& nextStep )
///
/// @brief	���ٵ���·���滮�������㷨�������£�
///           1. ���ö��ַ���ͬ�� x �� y �����λ��
///           2. ��������ת�����������ʱ�� t_theta, ȡ t_x, t_y, t_theta �е����ֵ
///              Ϊ���������ʱ��
///         
///           
/// @author	Yonghai Wu
/// @date	2010-3-24
///
/// @param	start				The start. 
/// @param	final				The final. 
/// @param	frameRate			The frame rate. 
/// @param	capability			The capability. 
/// @param [in,out]	nextStep	the next step. ������һ�ܵ�λ�ƺ��ٶ�
////////////////////////////////////////////////////////////////////////////////////////////////////

void trapezoidalVelocityPath( const PlayerPoseT& start, const PlayerPoseT& final, double frameRate, const PlayerCapabilityT& capability, PlayerPoseT& nextStep )
{
	double x0      = start.X();
	double y0      = start.Y();
	double xf      = final.X();
	double yf      = final.Y();
	double vx0     = start.VelX();
	double vy0     = start.VelY();
	double vtheta0 = start.RotVel();
	double theta0  = start.Dir();
	double thetaf  = final.Dir();
	double x,vx,ax,y,vy,ay,theta,vtheta,atheta;
	if (capability.maxAccel==0 || capability.maxAngularAccel==0 || capability.maxSpeed==0 || capability.maxAngularSpeed==0){
		cout << "trapezoidalVelocityPath code error: invalid scale(s)" << endl;
	}

	double maxSpeed = capability.maxSpeed;
	double maxAccel = capability.maxAccel;
    double maxAngleSpeed = capability.maxAngularSpeed;
	double maxAngleAccel = capability.maxAngularAccel;
	//finds the shortest "rotational path" between theta0 and thetaf
	double dtheta=thetaf-theta0;
	if (dtheta < -Param::Math::PI)
		dtheta = dtheta + 2*Param::Math::PI;
	if (dtheta > Param::Math::PI)
		dtheta = dtheta - 2*Param::Math::PI;
	thetaf=theta0 + dtheta;

    //double controlAlpha = syncTG(x0,y0,vx0,vy0,maxAccel,maxAccel,maxSpeed);
	// Do a binary search to find the proper control effort, such that the times for the
	// trajectories in x and y direction are the same.
	// The variable to vary is "alpha", which runs from 0 to pi/2

	const double factor = 1.5;

	int numIter = 5;       //number of iterations performed. error in u is 2^(-numIter)
	double u   =  1.568;    // approx. PI/2
	double du  = -1.568;
	double alpha,durX,durY;
	vector<vector<double> > storeDataX,storeDataY;
	int iter = 0;
	double t0 = 0;
	double rev = 1;
	for (int j=1;j<=numIter;j++)
	{
		du = du*0.5;
		alpha = u+du;
		double axMax = sin(alpha)*maxAccel;
		double ayMax = cos(alpha)*maxAccel;
		double vxMax = sin(alpha)*maxSpeed;
		double vyMax = cos(alpha)*maxSpeed;
		//std::cout<<axMax<<" "<<ayMax<<endl;
		//Find duration for x-direction    
		int iter = 0;
		double t0 = 0;
		double rev = 1;
		storeDataX = tg_recurs(t0, x0, xf, vx0, rev, iter, axMax, factor*axMax, vxMax);
		double num_stagesX = storeDataX.size();
		durX = storeDataX[num_stagesX-1][5];
		//Find duration for y-direction    
		iter = 0;
		t0 = 0;
		rev = 1;
		storeDataY = tg_recurs(t0, y0, yf, vy0, rev, iter, ayMax, factor*ayMax, vyMax);
		double num_stagesY = storeDataY.size();
		durY =  storeDataY[num_stagesY-1][5];
		double tfDelta = durX-durY;
		//std::cout<<"alpha:"<<alpha<<"durx:"<<durX<<"dury:"<<durY<<endl;
		if (tfDelta <= 0.0)
			u=alpha;
	}
	vector<vector<double> >  storeDataTheta = tg_recurs(t0,theta0,thetaf,vtheta0,rev,iter,maxAngleAccel,factor*maxAngleAccel,maxAngleSpeed);
	buildPath(storeDataTheta,theta,vtheta,atheta);
	buildPath(storeDataX,x,vx,ax);
	buildPath(storeDataY,y,vy,ay);
	double duration = durX > durY ? durX : durY; // �˶�������ʱ��
	nextStep.SetValid(true);
	nextStep.SetPos(x,y);
	nextStep.SetVel(vx,vy);
	nextStep.SetDir(theta);
	nextStep.SetRotVel(vtheta);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void trapezoidalVelocityPath( const PlayerPoseT& start, const PlayerPoseT& final,
/// 	double frameRate, const PlayerCapabilityT& capability, PlayerPoseT& nextStep,
/// 	vector< vector<double> >& pathList)
///
/// @brief	�����ٵ���·���滮�������㷨�������£�
///           1. ���ö��ַ���ͬ�� x �� y �����λ��
///           2. ��������ת�����������ʱ�� t_theta, ȡ t_x, t_y, t_theta �е����ֵ
///              Ϊ���������ʱ��
///
/// @author	Yonghai Wu
/// @date	2010-3-24
///
/// @param	start				The start. 
/// @param	final				The final. 
/// @param	frameRate			The frame rate. 
/// @param	capability			The capability. 
/// @param [in,out]	nextStep	the next step.  ��һ���ڵ�λ�ƺ��ٶ�
/// @param [in,out]	pathList	list of pathes. ����·��
////////////////////////////////////////////////////////////////////////////////////////////////////

void trapezoidalVelocityPath( const PlayerPoseT& start,
							 const PlayerPoseT& final,
							 double frameRate,
							 const PlayerCapabilityT& capability,
							 PlayerPoseT& nextStep,
							 vector< vector<double> >& pathList)
	{
	

	double x0      = start.X();
	double y0      = start.Y();
	double xf      = final.X();
	double yf      = final.Y();
	double vx0     = start.VelX();
	double vy0     = start.VelY();
	double vxf     = final.VelX();
	double vyf     = final.VelY();
	double vtheta0 = start.RotVel();
	double vthetaf = final.RotVel();
	double theta0  = start.Dir();
	double thetaf  = final.Dir();

	double x,vx,ax,y,vy,ay,theta,vtheta,atheta;
	if (capability.maxAccel==0
		|| capability.maxAngularAccel==0
		|| capability.maxSpeed==0
		|| capability.maxAngularSpeed==0){
			cout << "trapezoidalVelocityPath code error: invalid scale(s)" << endl;
		}
	/*std::cout<<" maxacc: "<<capability.maxAccel<<" maxdec: "<<capability.maxDec<<" maxangacc: "<<capability.maxAngularAccel
		<<" maxangdec: "<<capability.maxAngularDec<<" maxspeed: "<<capability.maxSpeed<<endl;
	std::cout<<" x0: "<<x0<<" xf: "<<xf<<" vx0: "<<vx0<<" vxf: "<<vxf<<endl;*/

	double maxSpeed = capability.maxSpeed;
	double maxAccel = capability.maxAccel;
	double maxAngleSpeed = capability.maxAngularSpeed;
	double maxAngleAccel = capability.maxAngularAccel;
	double maxDec = capability.maxDec;
	double maxAngleDec = capability.maxAngularDec;

	//finds the shortest "rotational path" between theta0 and thetaf
	double dtheta=thetaf-theta0;
	if (dtheta < -Param::Math::PI)
		dtheta = dtheta + 2*Param::Math::PI;
	if (dtheta > Param::Math::PI)
		dtheta = dtheta - 2*Param::Math::PI;
	thetaf=theta0 + dtheta;

	//double controlAlpha =
	// syncTG(x0,y0,vx0,vy0,maxAccel,maxAccel,maxSpeed); Do a binary
	// search to find the proper control effort, such that the times
	// for the trajectories in x and y direction are the same. The
	// variable to vary is "alpha", which runs from 0 to pi/2

	//  syncTG(start, final, capability);


	int numIter = 5;       //number of iterations performed. error in u is 2^(-numIter)
	double u   =  1.568;    // approx. PI/2
	double du  = -1.568;
	double alpha,durX,durY;
	vector<vector<double> > storeDataX,storeDataY;
	int iter = 0;
	double t0 = 0;
	double rev = 1;

	 double tol_pos = 1;
	 double tol_vel = 2;
	 double tol_theta = 0.01;
	 double tol_vtheta = 0.02;
	//////////////////////////////////////////////////////////////////////////
	CUsecTimer _usecTimer;
	_usecTimer.start();
	//////////////////////////////////////////////////////////////////////////
	for (int j=1;j<=numIter;j++)
		{
			
			
		du = du*0.5;
		alpha = u+du;
		//alpha = PI/2;
		double axMax = sin(alpha)*maxAccel;
		double ayMax = cos(alpha)*maxAccel;
		double adxMax = sin(alpha)*maxDec;
		double adyMax = cos(alpha)*maxDec;
		double vxMax = sin(alpha)*maxSpeed;
		double vyMax = cos(alpha)*maxSpeed;
		//std::cout<<axMax<<" "<<ayMax<<endl;

		//Find duration for x-direction    
		int iter = 0;
		double t0 = 0;
		double rev = 1;
		storeDataX = tg_recurs(t0,
			x0,
			xf,
			vx0,
			vxf,
			rev,
			iter,
			axMax,
			adxMax,
			vxMax,
			tol_pos,
			tol_vel);
		double num_stagesX = storeDataX.size();
		durX = storeDataX[num_stagesX-1][5];
		//Find duration for y-direction    
		iter = 0;
		t0 = 0;
		rev = 1;
		storeDataY = tg_recurs(t0,
			y0,
			yf,
			vy0,
			vyf,
			rev,
			iter,
			ayMax,
			adyMax,
			vyMax,
			tol_pos,
			tol_vel);
		double num_stagesY = storeDataY.size();
		durY =  storeDataY[num_stagesY-1][5];
		double tfDelta = durX-durY;
		//std::cout<<"alpha:"<<alpha<<"durx:"<<durX<<"dury:"<<durY<<endl;
		if (tfDelta <= 0.0001 )
			{
			//std::cout<<j<<" alpha: "<<alpha<<" durx: "<<durX<<" dury: "<<durY<<endl;
			u=alpha;
			//     break;
			}
		if( fabs(tfDelta) < 0.0001 )
			break;

		
		}
	////////////////////////////////////////////////////////////////////////////
	//_usecTimer.stop();
	//cout<<"Time: "<<_usecTimer.time()*1.0/1000<<endl;
	////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	_usecTimer.start();
	//////////////////////////////////////////////////////////////////////////

	PRINT_DEBUG(alpha);
	PRINT_DEBUG(durX);
	PRINT_DEBUG(durY);

	vector<vector<double> >  storeDataTheta = tg_recurs(t0,
		theta0,
		thetaf,
		vtheta0,
		vthetaf,
		rev,
		iter,
		maxAngleAccel,
		maxAngleDec,
		maxAngleSpeed,
		tol_theta,
		tol_vtheta);

	

	//cout<<"theta: "<<alpha<<endl;
	//cout<<maxAngleAccel<<"::::"<<maxAngleDec<<"::::"<<maxAngleSpeed<<endl;
	//cout<<theta0<<"::::"<<thetaf<<"::::"<<vtheta0<<"..."<<vthetaf<<endl;
	vector< vector<double> > pathListR, pathListX, pathListY;
	buildPath(storeDataTheta,theta,vtheta,atheta,pathListR);
	buildPath(storeDataX,x,vx,ax, pathListX);
	buildPath(storeDataY,y,vy,ay, pathListY);

	////////////////////////////////////////////////////////////////////////////
	//_usecTimer.stop();
	//cout<<"Time2: "<<_usecTimer.time()*1.0/1000<<endl;
	////////////////////////////////////////////////////////////////////////////

	

	double duration = durX > durY ? durX : durY; // �˶�������ʱ��

	int pathStepX = pathListX.size();
	int pathStepY = pathListY.size();
	int pathStepR = pathListR.size();
	//std::cout<<" path len: r "<<pathStepR<<std::endl;
	PRINT_DEBUG(pathStepX);
	PRINT_DEBUG(pathStepY);
	PRINT_DEBUG(pathStepR);
   
	nextStep.SetValid(true);
	nextStep.SetPos(x,y);
	nextStep.SetVel(vx,vy);
	nextStep.SetDir(theta);
	nextStep.SetRotVel(vtheta);
	//std::cout<<"~~ vx0: "<<vx0<<" vxf: "<<vxf<<" x0: "<<x0<<" xf: "<<xf<<" ax: "<<ax<<" t= "<<pathStepX<<std::endl;
	//std::cout<<"~~ vY0: "<<vy0<<" vyf: "<<vy0<<" y0: "<<y0<<" yf: "<<yf<<" yx: "<<ay<<" t= "<<pathStepY<<std::endl;
//	std::cout<<"~~ vx: "<<vx<<" v: "<<vy<<" x: "<<x0<<" y: "<<y0<<" vx0: "<<vx0<<" vy0: "<<vy0<<" xf: "<<xf<<" yf: "<<yf<<std::endl;
	//std::cout<<" num of X: "<<pathStepX<<" num of Y: "<<pathStepY<<" num of R: "<<pathStepR<<endl;
	int maxPathStep = max( pathStepX, max(pathStepY, pathStepR) );
	PRINT_DEBUG(maxPathStep);

	
	
	/// save every step of the path
	for (int i=0; i < maxPathStep; i++)
		{
		double data[7] = {0};
		data[0] = i;

		if ( i >= pathStepX)
			{
			data[1] = 0;
			data[4] = 0;
			}
		else{
			data[1] = pathListX[i][0];
			data[4] = pathListX[i][1];
			}
		if ( i >= pathStepY)
			{
			data[2] = 0;
			data[5] = 0;
			}
		else{
			data[2] = pathListY[i][0];
			data[5] = pathListY[i][1];
			}

		if ( i >= pathStepR)
			{
			data[3] = 0;
			data[6] = 0;
			}
		else{
			data[3] = pathListR[i][0];
			data[6] = pathListR[i][1];
			}
		vector<double>  storeData(data, data+7);
		pathList.push_back(storeData);
		//  PRINT_DEBUG(pathList.size());
		}

	
	
	}
