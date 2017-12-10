/// FileName : 		CommandSender.h
/// 				declaration file
/// Description :	It supports command send interface for ZJUNlict,
///	Keywords :		fitting, send, interface
/// Organization : 	ZJUNlict@Small Size League
/// Author : 		cliffyin
/// E-mail : 		cliffyin@zju.edu.cn
///					cliffyin007@gmail.com
/// Create Date : 	2011-07-25
/// Modified Date :	2011-07-25 
/// History :	

#ifndef __COMMAND_SENDER_H__
#define __COMMAND_SENDER_H__

#include <singleton.h>
#include <WirelessModule.h>
#include "RobotCommand.h"
#include <param.h>
#include <vector>
#include <map>

/// <summary>	Enumeration that represent Robot Command Send Mode.  </summary>
///
/// <remarks>	cliffyin, 2011/7/25. </remarks>

enum RobotMode
{
	MODE_NONE = -1,
	MODE_NORMAL_ROBOT = 0,
	MODE_DEBUG_PID_WRITE,
	MODE_DEBUG_PID_READ,
	MODE_DEBUG_OneCar,
	MODE_DEBUG_Check,
	MODE_DEBUG_KICK_PARAM_SET,
	MODE_DEBUG_KICK_PARAM_GET
};

/// <summary>	Command sender, for command sending.  </summary>
///
/// <remarks>	cliffyin, 2011/7/25. </remarks>

class CCommandSender
{
public:

	/// <summary>	Action command.  </summary>
	///
	/// <remarks>	cliffyin, 2011/7/25. </remarks>

	struct ActionCommand {
		double			_xSpeed;	//x方向速度
		double			_ySpeed;	//y方向速度
		double			_rSpeed;	//旋转速度
		unsigned char	_dribble;	//是否带球
		unsigned int	_kick;		//射门力度
		double			_angle;		//陀螺仪转过角度
		unsigned int	_radius;	//陀螺仪旋转半径
		bool			_gyro;	//陀螺仪的旋转车号
		bool        _stop;
		ActionCommand():_xSpeed(0), _ySpeed(0), _rSpeed(0), _dribble(0), _kick(0),
						_angle(0), _radius(0), _gyro(false),_stop(false)
		{}
	};

	/// <summary>	Pid write command.  </summary>
	///
	/// <remarks>	cliffyin, 2011/7/25. </remarks>

	struct PidWriteCommand{
		unsigned int _p[4];
		unsigned int _i[4];
		unsigned int _d[4];
	};

	/// <summary>	Default constructor. </summary>
	///
	/// <remarks>	cliffyin, 2011/7/25. </remarks>

	CCommandSender(void);

	/// <summary>	Finaliser. </summary>
	///
	/// <remarks>	cliffyin, 2011/7/25. </remarks>

	~CCommandSender(void);

	/// <summary>	load compensation values. </summary>
	///
	/// <remarks>	bykang, 2015/7/11. </remarks>

	void loadLevels();

	/// <summary>	Sets a mode.// </summary>
	/// 机器人模式设定,车号默认-1表示全体设定
	/// <remarks>	cliffyin, 2011/7/25. </remarks>
	///
	/// <param name="robot_mode">	(optional) the robot mode. </param>
	/// <param name="robot_num"> 	(optional) the robot number. </param>

	void setMode(int robot_mode = MODE_NORMAL_ROBOT, int robot_num = -1);  

	/// <summary>	Sends the command. </summary>
	///
	/// <remarks>	cliffyin, 2011/7/25. </remarks>

	void sendCommand();

	/// <summary>	Sends the command. </summary>
	/// send (PC->Robot)
	/// <remarks>	cliffyin, 2011/7/25. </remarks>
	///
	/// <param name="robotIndex">	Real index of the vehicle list. </param>

	void sendCommand(unsigned char robotIndex[]);

	/// <summary>	Sets motion speed interface. </summary>
	/// send (PC->Robot)
	/// <remarks>	cliffyin, 2011/7/25. </remarks>
	///
	/// <param name="num">   	Vehicle num. </param>
	/// <param name="xSpeed">	The x coordinate speed. </param>
	/// <param name="ySpeed">	The y coordinate speed. </param>
	/// <param name="rSpeed">	The rotation speed. </param>

	void setSpeed(int num, double xSpeed, double ySpeed, double rSpeed);

	/// <summary>	Sets motion speed interface(with compensation). </summary>
	/// send (PC->Robot)
	/// <remarks>	bykang, 2015/7/11. </remarks>
	///
	/// <param name="num">   	Vehicle num. </param>
	/// <param name="realNum">  Real vehicle num. </param>
	/// <param name="xSpeed">	The x coordinate speed. </param>
	/// <param name="ySpeed">	The y coordinate speed. </param>
	/// <param name="rSpeed">	The rotation speed. </param>

	void setSpeed(int num, int realNum, double xSpeed, double ySpeed, double rSpeed);

	/// <summary>	Sets dribble. </summary>
	///
	/// <remarks>	cliffyin, 2011/7/25. </remarks>
	///
	/// <param name="num">	  	Vehicle num. </param>
	/// <param name="dribble">	The dribble power. </param>

	void setDribble(int num, unsigned char dribble);

	/// <summary>	Sets Gyro. </summary>
	/// num陀螺仪车号 angle旋转角度 radius传球距离 speed旋转速度
	/// <remarks>	zhyaic, 2012/4/14. </remarks>
	///
	/// <param name="num"> 	Vehicle num. </param>
	/// <param name="angle"> The rot angle. </param>
	/// <param name="radius"> The rot radius. </param>
	/// <param name="rspeed"> The rot speed. </param>

	void setGyro(int num, unsigned char dribble, double angle, unsigned int radius, double rspeed);

	/// <summary>	Sets kick. </summary>
	/// kick击球速度 chip挑球距离，pass传球距离
	/// <remarks>	cliffyin, 2011/7/25. </remarks>
	///
	/// <param name="num"> 	Vehicle num. </param>
	/// <param name="kick">	The kick power. </param>
	/// <param name="chip">	The chip distance. </param>
	/// <param name="pass">	The pass distance. </param>

	void setKick(int num, double kick, double chip, double pass);

	/// <summary>	Sets a pid parameter. </summary>
	/// PID参数设定
	/// <remarks>	cliffyin, 2011/7/25. </remarks>
	///
	/// <param name="num">		   	Vehicle num. </param>
	/// <param name="proportion">  	[in,out] If non-null, the proportion. </param>
	/// <param name="intergrate">  	[in,out] If non-null, the intergrate. </param>
	/// <param name="differential">	[in,out] If non-null, the differential. </param>

	void setPidParam(int num, unsigned int* proportion, unsigned int* intergrate, unsigned int* differential);

	/// <summary>	Gets a robot information.// </summary>
	/// receive (Robot->PC) , 返回值表示数据的时间序号
	/// <remarks>	cliffyin, 2011/7/25. </remarks>
	///
	/// <param name="num"> 	Vehicle num. </param>
	/// <param name="info">	[in,out] If non-null, the information. </param>
	///
	/// <returns>	The robot information. </returns>

	int getRobotInfo(int num, ROBOTINFO* info);

	void setstop(int num,bool torf);

private:

	/// <summary>	Action command to robot command. </summary>
	///
	/// <remarks>	cliffyin, 2011/7/25. </remarks>
	///
	/// <param name="cmd">	   	The action command. </param>
	/// <param name="robotCmd">	[in,out] The robot command. </param>

	void command2RobotCmd(const ActionCommand& cmd, ROBOTCOMMAND& robotCmd);

	/// <summary>	Speed tran. </summary>
	/// 速度分解换算
	/// <remarks>	cliffyin, 2011/7/25. </remarks>
	///
	/// <param name="m_fX">		 	The f x coordinate. </param>
	/// <param name="m_fY">		 	The f y coordinate. </param>
	/// <param name="m_fZ">		 	The f z coordinate. </param>
	/// <param name="m_fOrg">	 	The f org. </param>
	/// <param name="isNewRobot">	(optional) the is new robot. </param>

	void SpeedTran(double m_fX,double m_fY,double m_fZ,double m_fOrg,bool isNewRobot=false);

	/// <summary>	Flat kick power fitting. </summary>
	///
	/// <remarks>	cliffyin, 2011/7/25. </remarks>
	///
	/// <param name="num">	   	Vehicle num. </param>
	/// <param name="kick">	   	The kick power. </param>
	/// <param name="chipkick">	(optional) the chipkick. </param>
	///
	/// <returns>	. </returns>

	unsigned int kickPower2Mode(int num, double kick, bool chipkick = false );

	/// <summary>	speedY compensation </summary>
	///
	/// <remarks>	bykang, 2015/7/2. </remarks>
	///
	/// <param name="carNum">	car number		</param>
	/// <param name="speedY">	y speed.		</param>
	/// <param name="speedW">	rotate speed>	</param>
	///
	/// <returns>  rotate speed after compensation. </returns>
	double docompensate(const int carNum, const double speedY, const double speedW);

private:

	/// <summary> The current command </summary>
	ActionCommand _currentCommand[Param::Field::MAX_PLAYER+1];

	/// <summary> The current parameter </summary>
	PidWriteCommand _currentParam[Param::Field::MAX_PLAYER];

	/// <summary> The wireless </summary>
	CWirelessModule *_wireless;

	/// <summary> Zero-based index of the real robot </summary>
	unsigned char _realRobotIndex[Param::Field::MAX_PLAYER];

	// 轮子分布参数
	/// <summary> The 1,t 2,t 3,t 4,l radius </summary>
	double t1,t2,t3,t4,l_radius;

	/// <summary> The wheel speed </summary>
	double wheel_speed[4];

	// 机器人模式
	/// <summary> The robot mode </summary>
	int _robotMode[Param::Field::MAX_PLAYER];

	/// <summary> true to mode setting </summary>
	bool _modeSetting[Param::Field::MAX_PLAYER]; // 被设置为非默认模式的周期

	/// <summary> 车号到底盘号 </ summary >
	std::map<int, int> realToChassis;

	/// <summary> 底盘号-分档 </ summary >
	std::map<int, std::vector<double> > getLevels;

	/// <summary> 底盘号-分档补偿值 </ summary >
	std::map<int, std::vector<double> > getValues;
};

/// <summary>	Defines an alias representing the command sender. . </summary>
///
/// <remarks>	cliffyin, 2011/7/25. </remarks>

typedef NormalSingleton<CCommandSender> CommandSender;

#endif