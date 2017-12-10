/************************************************************************/
/* file created by shengyu, 2005.06.11                                  */
/************************************************************************/
#include <robot_power.h>

#ifndef _ROBOT_COMMAND_H_
#define _ROBOT_COMMAND_H_

//#define SHOOT_ENABLE				0x40
//#define SHOOT_DONTDETECTBALL		0x20
//#define SHOOT_CHIP					0x10
//#define SHOOT_SHUTDRIBBLER			0x08

#define SHOOT_CHIP					0x100

struct ROBOTCOMMAND
{
	int mode; // ����ģʽ
	double speed[4];	//�ĸ������ٶ�
	unsigned int driver_p[4];
	unsigned int driver_i[4];
	unsigned int driver_d[4];
	unsigned int shoot;
	unsigned char cb;
	double x,y,rotate;
	unsigned int radius;
	double angle;
	bool gyro;
	bool stop;
};

#endif