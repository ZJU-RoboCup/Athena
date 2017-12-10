#ifndef _KICKPARAM_
#define _KICKPARAM_

#include <geometry.h>
#include "utils.h"
#include "singleton.h"
//非线性优化库
#include "nlopt/nlopt.hpp"

typedef enum {
	CHIP = 0,
	FLAT
}KICKTYPE;


class CKickParam{
public:
	CKickParam();
	~CKickParam(){}
	void getData(KICKTYPE kickType, int playerNum); // get data from lua
	int getFlatSampleNum() { return flatSampleNum; }
	double calcFlatFunction(const double *x);
	void calcFlatParam(int playerNum);
	void calcFlatGrad(const double *x, double *grad);

private:
	double A_flatKick;  // power = A * ballSpeed + B
	double B_flatKick;
	
	double A_chipKick;
	double B_chipKick;
	double C_chipKick;

	// power - speed data
	double flatShootData[20][2]; // 50 - 120, first row: power; second row: real speed
	int flatSampleNum;
	double chipShootData[20][2];
	int chipSampleNum;

};

typedef NormalSingleton< CKickParam > KickParam;





#endif 