#ifndef _GETLUADATA_
#define _GETLUADATA_
/*get Compensate Data from Lua*/
/*Test Only!!*/

#include "singleton.h"
#include "geometry.h"
#include "Compensate.h"
#include "KickParam.h"


class CGetLuaData{

public:

	CGetLuaData();
	~CGetLuaData() { }
	void getData(double _speed_in, double _shoot_dir, double _out_dir);
	void getAdjustData(double _v_out_dir, double _player2pos, double _speed_in, double _shoot_dir);
	double getvOut() { return v_out_dir; };
	double getPlayer2pos(){ return player2pos; };
	void getKickParamData(bool isFinished, int playerNum);
	int updateKickParamPower(int newPower);
	int getKickParamPower();

private:
	double speed_in;
	double shoot_dir;
	double out_dir;
	double v_out_dir;
	double player2pos;
	int kickParamPower;
};

typedef NormalSingleton<CGetLuaData> GetLuaData;
	
#endif