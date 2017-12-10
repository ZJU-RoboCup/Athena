#include "GetLuaData.h"

CGetLuaData:: CGetLuaData():speed_in(0), shoot_dir(0), out_dir(0), v_out_dir(0), player2pos(0), kickParamPower(999050) { }

void CGetLuaData::getData(double _speed_in, double _shoot_dir, double _out_dir)
{
	speed_in = _speed_in;
	shoot_dir = _shoot_dir;
	out_dir = _out_dir;
	std::cout<<"C++ recording data from lua:" << speed_in << ' ' << shoot_dir << ' ' << out_dir << std::endl;
	CompensateNew::Instance()->getData(speed_in, shoot_dir, out_dir);
}

void CGetLuaData::getAdjustData(double _v_out_dir, double _player2pos, double _speed_in, double _shoot_dir) {
	v_out_dir = _v_out_dir;
	player2pos = _player2pos;
	CompensateNew::Instance()->getData(_speed_in, _shoot_dir, abs(_v_out_dir - _player2pos)); // 先内闭环，修正系数
	CompensateNew::Instance()->adjustCmpParam(_v_out_dir, _player2pos, _speed_in, _shoot_dir);
	std::cout << "C++ recording adjusting data from lua" << std::endl;
}

void CGetLuaData::getKickParamData(bool isFinished, int playerNum) {
	// call KickParam calculate module
	std::cout << "in CGetLuaData" << std::endl;
	if (isFinished) {
		KickParam::Instance()->getData(FLAT, playerNum);
	}
}

int CGetLuaData::updateKickParamPower(int newPower)
{
	kickParamPower = newPower;
	//std::cout << "updatekickParamPower:" << kickParamPower << std::endl;
	return kickParamPower;
}

int CGetLuaData::getKickParamPower()
{
	//std::cout << "getKickParamPower" << kickParamPower << std::endl;
	return kickParamPower;
}