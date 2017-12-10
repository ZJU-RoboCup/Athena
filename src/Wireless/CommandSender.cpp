/// FileName : 		CommandSender.cpp
/// 				implementation file
/// Description :	It supports command send interface for ZJUNlict,
///	Keywords :		fitting, send, interface
/// Organization : 	ZJUNlict@Small Size League
/// Author : 		cliffyin
/// E-mail : 		cliffyin@zju.edu.cn
///					cliffyin007@gmail.com
/// Create Date : 	2011-07-25
/// Modified Date :	2011-07-25 
/// History :

#include "commandsender.h"
#include <utils.h>
#include <PlayInterface.h>
#include <ConfigReader.h>
#include <fstream>
#include "VisionModule.h"
//#include "param_reader_macros.h"

namespace {
	const double pi = Param::Math::PI;

	/// 平射分档参数
	float A_KICK[12];
	float B_KICK[12];   
	float C_KICK[12];
	int MIN_FLAT_KICK_POWER[12] = {20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20};
	int MAX_FLAT_KICK_POWER[12] = {127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127};

	/// 挑射分档参数
	float A_CHIP_KICK[12];
	float B_CHIP_KICK[12];
	float C_CHIP_KICK[12];
	const int MIN_CHIP_POWER = 25;
	int MIN_CHIP_KICK_POWER[12] = {20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20};
	int MAX_CHIP_KICK_POWER[12] = {127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127};

	/// 平/挑射参数外部文件读入
	CR_DECLARE(ROBOT_ID)
	CR_DECLARE(APARAM_FLAT_KICK)
	CR_DECLARE(BPARAM_FLAT_KICK)
	CR_DECLARE(CPARAM_FLAT_KICK)
	CR_DECLARE(MINPARAM_FLAT_KICK_POWER)
	CR_DECLARE(MAXPARAM_FLAT_KICK_POWER)
	CR_DECLARE(APARAM_CHIP_KICK)
	CR_DECLARE(BPARAM_CHIP_KICK)
	CR_DECLARE(CPARAM_CHIP_KICK)
	CR_DECLARE(MINPARAM_CHIP_KICK_POWER)
	CR_DECLARE(MAXPARAM_CHIP_KICK_POWER)

	bool USE_COMPENSATION_FOR_Y = true;
	CR_DECLARE(Y_COMPENSATION_GEARS);
	CR_DECLARE(W_COMPENSATION_VALUE);
}

CCommandSender::CCommandSender(void)
{
	_wireless = new CWirelessModule;
	CR_SETUP(KickParam, ROBOT_ID, CR_INT)
	CR_SETUP(KickParam, APARAM_FLAT_KICK, CR_DOUBLE)
	CR_SETUP(KickParam, BPARAM_FLAT_KICK, CR_DOUBLE)
	CR_SETUP(KickParam, CPARAM_FLAT_KICK, CR_DOUBLE)
	CR_SETUP(KickParam, MINPARAM_FLAT_KICK_POWER, CR_INT)
	CR_SETUP(KickParam, MAXPARAM_FLAT_KICK_POWER, CR_INT)
	CR_SETUP(KickParam, APARAM_CHIP_KICK, CR_DOUBLE)
	CR_SETUP(KickParam, BPARAM_CHIP_KICK, CR_DOUBLE)
	CR_SETUP(KickParam, CPARAM_CHIP_KICK, CR_DOUBLE)
	CR_SETUP(KickParam, MINPARAM_CHIP_KICK_POWER, CR_INT)
	CR_SETUP(KickParam, MAXPARAM_CHIP_KICK_POWER, CR_INT)
	
	// ROBOT_ID 中保存了当前场上存在的车的车号
	int robots = VARSIZE(ROBOT_ID);
	int param_pairs = VARSIZE(BPARAM_FLAT_KICK);
	for (int i = 0 ; i < robots; ++ i) {
		int robot_num = VIVAR(ROBOT_ID)[i];
		A_KICK[i] = VDVAR(APARAM_FLAT_KICK)[robot_num-1];
		B_KICK[i] = VDVAR(BPARAM_FLAT_KICK)[robot_num-1];
		C_KICK[i] = VDVAR(CPARAM_FLAT_KICK)[robot_num-1];
		MIN_FLAT_KICK_POWER[i] = VIVAR(MINPARAM_FLAT_KICK_POWER)[robot_num-1];
		MAX_FLAT_KICK_POWER[i] = VIVAR(MAXPARAM_FLAT_KICK_POWER)[robot_num-1];
		A_CHIP_KICK[i] = VDVAR(APARAM_CHIP_KICK)[robot_num-1];
		B_CHIP_KICK[i] = VDVAR(BPARAM_CHIP_KICK)[robot_num-1];
		C_CHIP_KICK[i] = VDVAR(CPARAM_CHIP_KICK)[robot_num-1];
		MIN_CHIP_KICK_POWER[i] = VIVAR(MINPARAM_CHIP_KICK_POWER)[robot_num-1];
		MAX_CHIP_KICK_POWER[i] = VIVAR(MAXPARAM_CHIP_KICK_POWER)[robot_num-1];
	}
	
	// 默认机器人模式
	memset(_robotMode, MODE_NORMAL_ROBOT, sizeof(_robotMode));
	memset(_modeSetting, false, sizeof(_modeSetting));

	// 是否启用速度补偿
	DECLARE_PARAM_READER_BEGIN(CGotoPositionV2)
	READ_PARAM(USE_COMPENSATION_FOR_Y)
		DECLARE_PARAM_READER_END


	// 读取y方向补偿分档参数
	loadLevels();
	
}

CCommandSender::~CCommandSender(void)
{
	delete _wireless;
}

void CCommandSender::loadLevels()
{
	// 车号对应的底盘号
	CR_DECLARE(ROBOT_NUM);
	CR_DECLARE(CHASSIS_NUM);
	CR_SETUP(KickParam, ROBOT_NUM, CR_DOUBLE);
	CR_SETUP(KickParam, CHASSIS_NUM, CR_DOUBLE);
	if (VARSIZE(ROBOT_NUM) != VARSIZE(CHASSIS_NUM)) {
		USE_COMPENSATION_FOR_Y = false;
	} else {
		if (VARSIZE(ROBOT_NUM) >= 1) {
			for (int i = 0; i < VARSIZE(ROBOT_NUM); ++i) {
				if (VDVAR(ROBOT_NUM)[i] <= 12 && VDVAR(ROBOT_NUM)[i] > 0) {
					realToChassis[(int)VDVAR(ROBOT_NUM)[i]] = (int)VDVAR(CHASSIS_NUM)[i];
				}
			}
		}
	}
	// 读15-1补偿数据
	CR_DECLARE(LEVEL1501);
	CR_DECLARE(VALUE1501);
	CR_SETUP(KickParam, LEVEL1501, CR_DOUBLE);
	CR_SETUP(KickParam, VALUE1501, CR_DOUBLE);
	if (VARSIZE(LEVEL1501) == VARSIZE(VALUE1501)) {
		std::vector<double> tmpLevel, tmpValue;
		for (int i = 0; i < VARSIZE(LEVEL1501); ++i) {
			tmpLevel.push_back(VDVAR(LEVEL1501)[i]);
			tmpValue.push_back(VDVAR(VALUE1501)[i]);
		}
		if (tmpLevel.size() > 1) {
			getLevels[1501] = tmpLevel;
			getValues[1501] = tmpValue;
		}
	}
	//读15-2补偿数据
	CR_DECLARE(LEVEL1502);
	CR_DECLARE(VALUE1502);
	CR_SETUP(KickParam, LEVEL1502, CR_DOUBLE);
	CR_SETUP(KickParam, VALUE1502, CR_DOUBLE);
	if (VARSIZE(LEVEL1502) == VARSIZE(VALUE1502)) {
		std::vector<double> tmpLevel, tmpValue;
		for (int i = 0; i < VARSIZE(LEVEL1502); ++i) {
			tmpLevel.push_back(VDVAR(LEVEL1502)[i]);
			tmpValue.push_back(VDVAR(VALUE1502)[i]);
		}
		if (tmpLevel.size() > 1) {
			getLevels[1502] = tmpLevel;
			getValues[1502] = tmpValue;
		}
	}
	//读15-3补偿数据
	CR_DECLARE(LEVEL1503);
	CR_DECLARE(VALUE1503);
	CR_SETUP(KickParam, LEVEL1503, CR_DOUBLE);
	CR_SETUP(KickParam, VALUE1503, CR_DOUBLE);
	if (VARSIZE(LEVEL1503) == VARSIZE(VALUE1503)) {
		std::vector<double> tmpLevel, tmpValue;
		for (int i = 0; i < VARSIZE(LEVEL1503); ++i) {
			tmpLevel.push_back(VDVAR(LEVEL1503)[i]);
			tmpValue.push_back(VDVAR(VALUE1503)[i]);
		}
		if (tmpLevel.size() > 1) {
			getLevels[1503] = tmpLevel;
			getValues[1503] = tmpValue;
		}
	}
	//读15-4补偿数据
	CR_DECLARE(LEVEL1504);
	CR_DECLARE(VALUE1504);
	CR_SETUP(KickParam, LEVEL1504, CR_DOUBLE);
	CR_SETUP(KickParam, VALUE1504, CR_DOUBLE);
	if (VARSIZE(LEVEL1504) == VARSIZE(VALUE1504)) {
		std::vector<double> tmpLevel, tmpValue;
		for (int i = 0; i < VARSIZE(LEVEL1504); ++i) {
			tmpLevel.push_back(VDVAR(LEVEL1504)[i]);
			tmpValue.push_back(VDVAR(VALUE1504)[i]);
		}
		if (tmpLevel.size() > 1) {
			getLevels[1504] = tmpLevel;
			getValues[1504] = tmpValue;
		}
	}
	//读15-5补偿数据
	CR_DECLARE(LEVEL1505);
	CR_DECLARE(VALUE1505);
	CR_SETUP(KickParam, LEVEL1505, CR_DOUBLE);
	CR_SETUP(KickParam, VALUE1505, CR_DOUBLE);
	if (VARSIZE(LEVEL1505) == VARSIZE(VALUE1505)) {
		std::vector<double> tmpLevel, tmpValue;
		for (int i = 0; i < VARSIZE(LEVEL1505); ++i) {
			tmpLevel.push_back(VDVAR(LEVEL1505)[i]);
			tmpValue.push_back(VDVAR(VALUE1505)[i]);
		}
		if (tmpLevel.size() > 1) {
			getLevels[1505] = tmpLevel;
			getValues[1505] = tmpValue;
		}
	}
	//读15-6补偿数据
	CR_DECLARE(LEVEL1506);
	CR_DECLARE(VALUE1506);
	CR_SETUP(KickParam, LEVEL1506, CR_DOUBLE);
	CR_SETUP(KickParam, VALUE1506, CR_DOUBLE);
	if (VARSIZE(LEVEL1506) == VARSIZE(VALUE1506)) {
		std::vector<double> tmpLevel, tmpValue;
		for (int i = 0; i < VARSIZE(LEVEL1506); ++i) {
			tmpLevel.push_back(VDVAR(LEVEL1506)[i]);
			tmpValue.push_back(VDVAR(VALUE1506)[i]);
		}
		if (tmpLevel.size() > 1) {
			getLevels[1506] = tmpLevel;
			getValues[1506] = tmpValue;
		}
	}
	//读15-7补偿数据
	CR_DECLARE(LEVEL1507);
	CR_DECLARE(VALUE1507);
	CR_SETUP(KickParam, LEVEL1507, CR_DOUBLE);
	CR_SETUP(KickParam, VALUE1507, CR_DOUBLE);
	if (VARSIZE(LEVEL1507) == VARSIZE(VALUE1507)) {
		std::vector<double> tmpLevel, tmpValue;
		for (int i = 0; i < VARSIZE(LEVEL1507); ++i) {
			tmpLevel.push_back(VDVAR(LEVEL1507)[i]);
			tmpValue.push_back(VDVAR(VALUE1507)[i]);
		}
		if (tmpLevel.size() > 1) {
			getLevels[1507] = tmpLevel;
			getValues[1507] = tmpValue;
		}
	}
	//读15-8补偿数据
	CR_DECLARE(LEVEL1508);
	CR_DECLARE(VALUE1508);
	CR_SETUP(KickParam, LEVEL1508, CR_DOUBLE);
	CR_SETUP(KickParam, VALUE1508, CR_DOUBLE);
	if (VARSIZE(LEVEL1508) == VARSIZE(VALUE1508)) {
		std::vector<double> tmpLevel, tmpValue;
		for (int i = 0; i < VARSIZE(LEVEL1508); ++i) {
			tmpLevel.push_back(VDVAR(LEVEL1508)[i]);
			tmpValue.push_back(VDVAR(VALUE1508)[i]);
		}
		if (tmpLevel.size() > 1) {
			getLevels[1508] = tmpLevel;
			getValues[1508] = tmpValue;
		}
	}
	//读15-9补偿数据
	CR_DECLARE(LEVEL1509);
	CR_DECLARE(VALUE1509);
	CR_SETUP(KickParam, LEVEL1509, CR_DOUBLE);
	CR_SETUP(KickParam, VALUE1509, CR_DOUBLE);
	if (VARSIZE(LEVEL1509) == VARSIZE(VALUE1509)) {
		std::vector<double> tmpLevel, tmpValue;
		for (int i = 0; i < VARSIZE(LEVEL1509); ++i) {
			tmpLevel.push_back(VDVAR(LEVEL1509)[i]);
			tmpValue.push_back(VDVAR(VALUE1509)[i]);
		}
		if (tmpLevel.size() > 1) {
			getLevels[1509] = tmpLevel;
			getValues[1509] = tmpValue;
		}
	}
	// 读15-10补偿数据
	CR_DECLARE(LEVEL1510);
	CR_DECLARE(VALUE1510);
	CR_SETUP(KickParam, LEVEL1510, CR_DOUBLE);
	CR_SETUP(KickParam, VALUE1510, CR_DOUBLE);
	if (VARSIZE(LEVEL1510) == VARSIZE(VALUE1510)) {
		std::vector<double> tmpLevel, tmpValue;
		for (int i = 0; i < VARSIZE(LEVEL1510); ++i) {
			tmpLevel.push_back(VDVAR(LEVEL1510)[i]);
			tmpValue.push_back(VDVAR(VALUE1510)[i]);
		}
		if (tmpLevel.size() > 1) {
			getLevels[1510] = tmpLevel;
			getValues[1510] = tmpValue;
		}
	}

}

void CCommandSender::setMode(int robot_mode, int robot_num) 
{
	if (robot_num > 0) {
		// 前面已经设置过了，就不能再设置
		if (! _modeSetting[robot_num-1]) {
			_robotMode[robot_num-1] = robot_mode;
			_modeSetting[robot_num-1] = true;
		}
	} else {
		for (int i = 0; i < Param::Field::MAX_PLAYER; ++i) {
			if (! _modeSetting[i]) {
				_robotMode[i] = robot_mode;
				_modeSetting[i] = true;
			}
		}
	}

	return ;
}

void CCommandSender::sendCommand()
{
	ROBOTCOMMAND robotCmd;
	for (int i = 1; i <= Param::Field::MAX_PLAYER; ++ i) {
		robotCmd.mode = _robotMode[i-1];
		SpeedTran(_currentCommand[i]._xSpeed, _currentCommand[i]._ySpeed, _currentCommand[i]._rSpeed, 0, false);
		command2RobotCmd(_currentCommand[i], robotCmd);

		// TODO 为何这里没有设置为真实的车号
		_wireless->doWirelessModule(i-1, robotCmd);
		_modeSetting[i-1] = false; // 设置复位
	}

	return ;
}

void CCommandSender::sendCommand(unsigned char robotIndex[])
{
	ROBOTCOMMAND robotCmd;
	for (int i = 1; i <= Param::Field::MAX_PLAYER; ++ i) {
		// 进行速度分解
		SpeedTran(_currentCommand[i]._xSpeed, _currentCommand[i]._ySpeed, _currentCommand[i]._rSpeed, 0, false);
		/*if (i==4){
			cout<<"**************************************************************************"<<endl;
			cout<<"Com Vx : "<<_currentCommand[i]._xSpeed<<endl;
			cout<<"Com Vy : "<<_currentCommand[i]._ySpeed<<endl;
			cout<<"Com Rx : "<<_currentCommand[i]._rSpeed<<endl;
		}*/
		// 上层ActionModule模块里设置的_currentCommand值在这里被转换成无线模块能接受的robotCmd形式
		// 将实际世界中的变量值意义按与底层机器人硬件指定协议转换到指定范围内,发送出去
		command2RobotCmd(_currentCommand[i], robotCmd);
		/*if (i==5){
			cout<<"**************************************************************************"<<endl;
			cout<<"RC Vx : "<<robotCmd.x<<endl;
			cout<<"RC Vy : "<<robotCmd.y<<endl;
			cout<<"RC Rx : "<<robotCmd.rotate<<endl;
		}*/
		// 进行模式设定
		robotCmd.mode = _robotMode[i-1];
		if (robotCmd.mode == MODE_DEBUG_PID_WRITE) {
			for (int j = 0; j < 4; j ++) {
				robotCmd.driver_p[j] = _currentParam[i-1]._p[j];
				robotCmd.driver_i[j] = _currentParam[i-1]._i[j];
				robotCmd.driver_d[j] = _currentParam[i-1]._d[j];
			}
		}

		// 对应的真实车号
		_realRobotIndex[i-1] = robotIndex[i-1]-1;
		//cout<<"num:"<<(int)robotIndex[i-1]<<endl;
		//cout<<"realnum:"<<(int)_realRobotIndex[i-1]<<endl;
		// 串口发送
		_wireless->doWirelessModule(_realRobotIndex[i-1], robotCmd);

		// 设置复位
		_modeSetting[i-1] = false; 
	}

	return ;
}

void CCommandSender::command2RobotCmd(const CCommandSender::ActionCommand& cmd, ROBOTCOMMAND& robotCmd)
{
	double xtemp = cmd._xSpeed;
	double ytemp = cmd._ySpeed;

	bool Using2011proto = true;
	if (Using2011proto) {
		// 以下部分用于新协议[6/11/2011 cliffyin]
		// vx : 1cm/s		限速 511cm/s
		// vy : 1cm/s		限速 511cm/s
		// w  : 40rad/s		限速 12.75rad/s
		robotCmd.x = (abs(xtemp)>511) ? Utils::Sign(xtemp)*511 : xtemp;
		robotCmd.y = (abs(ytemp)>511) ? Utils::Sign(ytemp)*511 : ytemp;
		robotCmd.rotate = (abs(cmd._rSpeed*40) > 500 ) ? Utils::Sign(cmd._rSpeed)*500 : cmd._rSpeed*40;
		/*robotCmd.x = robotCmd.x/0.94;
		robotCmd.y = robotCmd.y/0.85;*/
	} else {
		// 以下部分用于老协议 [6/11/2011 cliffyin]
		// vx : cm / s	限速 4m/s
		// vy : cm / s  限速 4m/s
		// w  : 40 * rad / s 
		robotCmd.x = (abs(xtemp)/2/2 > 126) ? Utils::Sign(xtemp)*126 : xtemp/2/2;
		robotCmd.y = (abs(ytemp)/2/2 > 126) ? Utils::Sign(ytemp)*126 : ytemp/2/2;
		robotCmd.rotate = (abs(cmd._rSpeed)*10 > 126) ? Utils::Sign(cmd._rSpeed)*126 : cmd._rSpeed*10;
	}

	// ofstream out("speedData.out", ios::out|ios::app);
	//if ( !(robotCmd.x == 0 && robotCmd.y == 0 && robotCmd.rotate == 0)) {
		//out << robotCmd.x << '\t' << robotCmd.y << '\t' << robotCmd.rotate << '\n';
	//}
	//out.close();
	robotCmd.cb = cmd._dribble;
	//cout<<"Dribble:  "<<int(cmd._dribble)<<endl;
	for (int i = 0; i < 4; ++ i) {
		robotCmd.speed[i] = wheel_speed[i];
	}
	
	robotCmd.shoot = cmd._kick;

	bool isTestInfrared = false;
	if (isTestInfrared) {
		robotCmd.x = 0;
		robotCmd.y = 0;
		robotCmd.rotate = 0;
		robotCmd.shoot = 20;
		robotCmd.cb = 0;
	}

	robotCmd.radius = cmd._radius;
	robotCmd.angle = cmd._angle;
	robotCmd.gyro = cmd._gyro;
    robotCmd.stop=cmd._stop;
	return ;
}

void CCommandSender::setSpeed(int num, double xSpeed, double ySpeed, double rSpeed)
{
	if (num < 1 || num > Param::Field::MAX_PLAYER) {
		cout<<"CommandSender : setSpeed invalid vehicle!"<<endl;
		return;
	}
	double changeRSpeed = rSpeed;
	double changeYspeed = ySpeed;
	//if (abs(ySpeed) > 240) {
	//	if (ySpeed > 0) {
	//		changeYspeed = 240;
	//	} else {
	//		changeYspeed =  -240;
	//	}
	//}
	//ofstream outFile("package.dw", ios::out|ios::app);
	//outFile << num << '\t' << changeYspeed << '\t';
	//outFile.close();

	//if (USE_COMPENSATION_FOR_Y) {
	//	changeRSpeed = docompensate(changeYspeed, rSpeed);
	//}

	_currentCommand[num]._xSpeed = xSpeed;
	_currentCommand[num]._ySpeed = changeYspeed;
	_currentCommand[num]._rSpeed = changeRSpeed;


	return ;
}

void CCommandSender::setSpeed(int num, int realNum, double xSpeed, double ySpeed, double rSpeed)
{
	if (num < 1 || num > Param::Field::MAX_PLAYER) {
		cout<<"CommandSender : setSpeed invalid vehicle!"<<endl;
		return;
	}
	double changeRSpeed = rSpeed;
	double changeYspeed = ySpeed;

	//ofstream outFile("setSpeed.txt", ios::out|ios::app);
	//outFile << realNum << '\t' << realToChassis[realNum] << endl;
	//outFile.close();
	
	if (USE_COMPENSATION_FOR_Y) {
		changeRSpeed = docompensate(realNum, changeYspeed, rSpeed);
	}

	_currentCommand[num]._xSpeed = xSpeed;
	_currentCommand[num]._ySpeed = changeYspeed;
	_currentCommand[num]._rSpeed = changeRSpeed;


	return ;
}

void CCommandSender::setGyro(int num, unsigned char dribble, double angle, unsigned int radius, double rspeed)
{
	if (num < 1 || num > Param::Field::MAX_PLAYER) {
		cout<<"CommandSender : setDribble invalid vehicle!"<<endl;
		return;
	}
	
	if(angle != 0){
		_currentCommand[num]._gyro = true;
	} else{
		_currentCommand[num]._gyro = false;
	}
	_currentCommand[num]._dribble = dribble;
	_currentCommand[num]._angle = angle;
	_currentCommand[num]._radius = radius;
	_currentCommand[num]._rSpeed = rspeed;
}
void CCommandSender::setDribble(int num, unsigned char dribble)
{
	if (num < 1 || num > Param::Field::MAX_PLAYER) {
		cout<<"CommandSender : setDribble invalid vehicle!"<<endl;
		return;
	}
	//cout<<"Num:  "<<num<<"  dribble:  "<<int(dribble)<<endl;
	_currentCommand[num]._dribble = dribble;

	return ;
}

void CCommandSender::setKick(int num, double kick, double chip, double pass)
{
	if (num < 1 || num > Param::Field::MAX_PLAYER) {
		cout<<"CommandSender : setKick invalid vehicle!"<<endl;
		return;
	}

	// 平/挑射分档
	if (fabs(kick) < 0.01) {
		if(fabs(chip)>0.01) {
			unsigned int ckickpower = kickPower2Mode(num,chip,true);
			_currentCommand[num]._kick =  ckickpower| SHOOT_CHIP;
		} else {
			_currentCommand[num]._kick = 0;
		}
	} else {
		if(fabs(chip)>0.01) {
			cout << "Both flatkick and chip occured" << endl;
		}
		_currentCommand[num]._kick = kickPower2Mode(num, kick, false);
	}


	return ;
}

void CCommandSender::setPidParam(int num, unsigned int* proportion, unsigned int* intergrate, unsigned int* differential)
{
	for (int i = 0; i < 4; ++ i) {
		_currentParam[num-1]._p[i] = proportion[i];
		_currentParam[num-1]._i[i] = intergrate[i];
		_currentParam[num-1]._d[i] = differential[i];
	}

	return ;
}

unsigned int CCommandSender::kickPower2Mode(int num, double kick, bool chipkick){
	int shootpower = 0;
	int realNum = _realRobotIndex[num-1]; // 数组返回值0-5
	//cout << "kick " << kick << endl;
	if (chipkick) {
		shootpower = (int)(A_CHIP_KICK[realNum]*kick*kick + B_CHIP_KICK[realNum]*kick + C_CHIP_KICK[realNum]);
		if (shootpower > MAX_CHIP_KICK_POWER[realNum]) {
			shootpower = MAX_CHIP_KICK_POWER[realNum];
		} else if (shootpower < MIN_CHIP_KICK_POWER[realNum]) {
			shootpower = MIN_CHIP_KICK_POWER[realNum];
		}
		if (kick >= 999000 && kick <= 999127) { // test mode
			shootpower = kick - 999000; // the real shoot power
		}
		//cout<<"chipShootPower:  "<<shootpower<<endl;
	} else {
		// TODO 二次函数拟合，A是二次项系数，B是一次项系数，C是常数项
		// 二次项系数乘了0.00001，这里要特别注意，yys 2015-05-02
		
		shootpower = (int)(0.00001*A_KICK[realNum]*kick*kick + B_KICK[realNum]*kick + C_KICK[realNum]);
		if (kick >= 9999) {
			shootpower = 127;
		}

		if (fabs(kick - 8888) < 0.5) {
			shootpower = 70;
			return shootpower;
		}
		if (shootpower > MAX_FLAT_KICK_POWER[realNum]) {
			shootpower = MAX_FLAT_KICK_POWER[realNum];
		} else if (shootpower < MIN_FLAT_KICK_POWER[realNum]) {
			shootpower = MIN_FLAT_KICK_POWER[realNum];
		}
		/* Added by FTQ, automatically measure the kick parameter 2017年11月24日*/
		if (kick >= 999000 && kick <= 999127) { // test mode
			shootpower = kick - 999000; // the real shoot power
		}
	}
	//cout << "CommandSender::" << shootpower << endl;
	return shootpower;
}

void CCommandSender::SpeedTran(double m_fX,double m_fY,double m_fZ,double m_fOrg,bool isNewRobot)
{
	//新车参数
	if (true == isNewRobot) {
		// 08年国内赛作出的新车
		t1 = 60;
		t2 = 300;
		t3 = 229;
		t4 = 131;
		l_radius=75.55/10;
	} else {
		// 07年国际赛做出的新车
		t1=57;
		t2=303;
		t3=225;
		t4=135;
		l_radius=75.55/10;
	}

	double g1,g2,g3,g4;

	m_fY=m_fY;
	m_fZ=m_fZ;
	m_fX=-m_fX;
	g1=pi*(90.0 + m_fOrg + t1)/180.0;
	g2=pi*(90.0 + m_fOrg + t2)/180.0;
	g3=pi*(90.0 + m_fOrg + t3)/180.0;
	g4=pi*(90.0 + m_fOrg + t4)/180.0;

	double a00 = cos(g1);	double a01 = sin(g1);  double a02 = l_radius;
	double a10 = cos(g2);	double a11 = sin(g2);  double a12 = l_radius;
	double a20 = cos(g3);	double a21 = sin(g3);  double a22 = l_radius;
	double a30 = cos(g4);	double a31 = sin(g4);  double a32 = l_radius;  
	
	wheel_speed[0] = (a00*m_fX + a01*m_fY + a02*m_fZ);
	wheel_speed[1] = (a10*m_fX + a11*m_fY + a12*m_fZ);
	wheel_speed[2] = (a20*m_fX + a21*m_fY + a22*m_fZ);
	wheel_speed[3] = (a30*m_fX + a31*m_fY + a32*m_fZ);

	return ;
}

//////////////////////////////////////////////////////////////////////////
// receive function
//////////////////////////////////////////////////////////////////////////
int CCommandSender::getRobotInfo(int num, ROBOTINFO* info)
{
	int realnum = PlayInterface::Instance()->getRealIndexByNum(num);
	CCommControl *pCommControl = _wireless->getControlComm();
	if (NULL != pCommControl) {
		return pCommControl->RequestSpeedInfo(realnum, info);
	} else {
		return -1;
	}

	return -1;	
}

void CCommandSender::setstop(int num, bool torf){
	if (num < 1 || num > Param::Field::MAX_PLAYER) {
		cout<<"CommandSender : setDribble invalid vehicle!"<<endl;
		return;
	}
	_currentCommand[num]._stop=torf;
}

double CCommandSender::docompensate(int carNum, double const speedY, double const speedW) 
{
	std::vector<double> curLevels = getLevels[realToChassis[carNum]];
	std::vector<double> curValues = getValues[realToChassis[carNum]];
	double addSpeedW = 0;
	if (curValues.size() == curLevels.size() && curLevels.size() > 1) {
		int i = 0;
		for (i = 0; i < curLevels.size(); ++i) {
			if (abs(speedY) < curLevels[i]) {
				break;
			}
		}
		if (i < curLevels.size()) {
			addSpeedW = curValues[i];
		} else {
			addSpeedW = curValues[curLevels.size() - 1];
		}
	}

	if (speedY < 0) {
		addSpeedW *= -1;
	}
	//ofstream outFile("compensation.txt", ios::out|ios::app);
	//outFile << carNum << '\t' << realToChassis[carNum] << '\t' << addSpeedW << endl;
	//outFile.close();
	return speedW + addSpeedW;
}
