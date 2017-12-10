#ifndef _PLAYER_COMMAND_H_
#define _PLAYER_COMMAND_H_
#include <ServerInterface.h>
#include <misc_types.h>
#include <string>
#include <ostream>
#include <tinyxml/ParamReader.h>

class CPlayerCommand{
public:
	CPlayerCommand(int number = 0, unsigned char dribble = 0) : _number(number), _dribble(dribble)
	{}
	virtual ~CPlayerCommand() { }
	// ִ�ж���
	virtual void execute(bool isSimulation) = 0; // team mode
	virtual void execute(bool isSimulation, int realNum){}
	// ������Ч��
	virtual CVector getAffectedVel() const { return CVector(0,0); } // ������Ч��
	virtual double getAffectedRotateSpeed() const { return 0; } // ������Ч��
	virtual void toStream(std::ostream& os) const { os << "Invalid Command"; } // ���Ϊ�ַ���
	friend std::ostream& operator << (std::ostream& os, const CPlayerCommand& cmd)
	{
		cmd.toStream(os);
		return os;
	}
	int number() const { return _number; } // ��Ա�ĺ���
	unsigned char dribble() const { return _dribble; } // �Ƿ����
private:
	int _number; ///<ִ���ߵĺ���
	unsigned char _dribble; ///<�Ƿ�ͬʱ����
};
#endif
