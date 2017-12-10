#ifndef _PLAYER_COMMAND_V2_H_
#define _PLAYER_COMMAND_V2_H_
#include <PlayerCommand.h>
/************************************************************************/
/*                         Speed                                        */
/************************************************************************/
class CPlayerSpeedV2 : public CPlayerCommand{
public:
	CPlayerSpeedV2(int number, double xspeed, double yspeed, double rspeed, unsigned char dribble);
	virtual void execute(bool isSimulation);
	virtual void execute(bool isSimulation, int realNum);	// 2015/7/11 add by bykang
	virtual CVector getAffectedVel() const;
	virtual double getAffectedRotateSpeed() const;
	virtual void toStream(std::ostream& os) const;
protected:
	double xspeed() const{ return _xspeed; }
	double yspeed() const{ return _yspeed; }
	double rspeed() const{ return _rspeed; }
	int commandType() const;
private:
	double _xspeed;
	double _yspeed;
	double _rspeed;
};

/************************************************************************/
/*                         Kick                                         */
/************************************************************************/
class CPlayerKickV2 : public CPlayerCommand{
public:
	CPlayerKickV2(int number, double normal, double chip, double pass, unsigned char dribble)
		: CPlayerCommand(number, dribble), _normal(normal), _chip(chip), _pass(pass){ }
	virtual void execute(bool isSimulation);
	virtual void toStream(std::ostream& os) const;
private:
	double _normal; // ��ͨ���������
	double _chip; // ����ľ���
	double _pass; // ����ľ���
};
/************************************************************************/
/*                         Gyro(������)                                 */
/************************************************************************/
class CPlayerGyroV2 : public CPlayerCommand{
public:
	CPlayerGyroV2(int number, unsigned char dribble, unsigned int radius, double angle,  double rspeed)
		: CPlayerCommand(number, dribble), _radius(radius), _angle(angle), _rspeed(rspeed){ }
	virtual void execute(bool isSimulation);
	virtual void toStream(std::ostream& os) const;
protected:
	double radius() const { return _radius; }
	double angle() const { return _angle; }
	double rspeed() const { return _rspeed; }
private:
	unsigned int _radius;	// ��ת�뾶
	double _angle;			// ��ת�Ƕ�
	double _rspeed;			// ��ת�ٶ�
};
#endif
