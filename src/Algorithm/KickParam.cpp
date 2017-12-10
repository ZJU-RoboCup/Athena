#include "KickParam.h"
#include "Global.h"


CKickParam::CKickParam():A_flatKick(0), B_flatKick(0),A_chipKick(0), B_chipKick(0), C_chipKick(0) {
	// nothing
}

void CKickParam::getData(KICKTYPE kickType, int playerNum) {
	// get Data from Lua
	// overall interface
	if (kickType == FLAT) {
		cout << " CKickParam::getData" << endl;
		static stringstream fullname(".txt");
		fullname << "kickParam_flat_"  << playerNum << ".txt";
		static ifstream infile(fullname.str().c_str());
		if (!infile) {
			cerr << "error opening file data " << fullname.str() << endl;
			exit(-1);
		}
		int i = 0;
		while(!infile.eof()) {
			string line;
			if(!getline(infile, line)) break;
			istringstream lineStream(line);
			lineStream >> flatShootData[i][0] >> flatShootData[i][1];
			cout << "KickParam::" << flatShootData[i][0] << " " << flatShootData[i][1] << endl;
			i++;
		}
		flatSampleNum = i;
		// calculate parameters
		KickParam::Instance()->calcFlatParam(playerNum);
	}
	else if (kickType == CHIP) {
		// TODO
		
	}
}

/* --------nlopt functions--------*/
double CKickParam::calcFlatFunction(const double *x) {
	// power = 0.00001 * A * speed^2 + B * speed + C
	double res = 0;
	for (int i = 0; i < flatSampleNum; i++) {
		//double A = 0.14; double B = 4;
		// previous model:
		// res = res + pow( 0.00001 * x[0] * pow(flatShootData[i][1], 2) + x[1] * flatShootData[i][1] + x[2] - flatShootData[i][0], 2 ); 
		res = res + pow(x[0] * flatShootData[i][1] + x[1] - flatShootData[i][0], 2);
		//res = res + pow(0.00001 * A * pow(flatShootData[i][1], 2) + B * flatShootData[i][1] + C - flatShootData[i][0], 2);
	}
	return res / 2 / flatSampleNum;
}

void CKickParam::calcFlatGrad(const double *x, double *grad) {
	grad[0] = 0; grad[1] = 0;
	for (int i = 0; i < flatSampleNum; i++) {
		//grad[0] = grad[0] + (0.00001 * x[0] * pow(flatShootData[i][1], 2) + x[1] * flatShootData[i][1] + x[2] - flatShootData[i][0]) * pow(flatShootData[i][1], 2);
		//grad[1] = grad[1] + (0.00001 * x[0] * pow(flatShootData[i][1], 2) + x[1] * flatShootData[i][1] + x[2] - flatShootData[i][0]) * flatShootData[i][1];
		grad[0] = grad[0] + 2 * (x[0] * flatShootData[i][1] + x[1] - flatShootData[i][0]) * flatShootData[i][1];
		grad[1] = grad[1] + 2 * (x[0] * flatShootData[i][1] + x[1] - flatShootData[i][0]);
	}
	grad[0] = grad[0] / 2 /flatSampleNum;
	grad[1] = grad[1] / 2 /flatSampleNum;
}

double costFunKickParam(unsigned n, const double *x, double *grad, void *my_func_data)
{
	// cost function. quadric
	if (grad) {
		KickParam::Instance()->calcFlatGrad(x, grad);
	}
	return KickParam::Instance()->calcFlatFunction(x);
}

void CKickParam::calcFlatParam(int playerNum) {
	nlopt_opt opt;
	double lb[2] = { -0.01, 0 };  /* lower bounds */

	opt = nlopt_create(NLOPT_LN_NEWUOA, 2);  /* algorithm and dimensionality */
	nlopt_set_lower_bounds(opt, lb);
	nlopt_set_min_objective(opt, costFunKickParam, NULL);


	nlopt_set_xtol_rel(opt, 1e-8);
	double x[2] = { 0.14  ,  4  };
	double minf;
	cout << "nlopt running" << endl;
	if (nlopt_optimize(opt, x, &minf) < 0) {
		std::cout << "error finding minimum" << std::endl;
	}
	else {
		std::cout << "found min at A: " << x[0] << " B: " << x[1] << ", min = " << minf <<std::endl;
		A_flatKick = x[0]; B_flatKick = x[1]; 
		// 写入文件中：
		ofstream outFile("KickParamNew.txt", ios::out|ios::app);
		outFile<< "flat: playerNumber: " << playerNum << " " << A_flatKick << " " << B_flatKick << endl;
		outFile.close();
	}
	nlopt_destroy(opt);
}


/* --------nlopt functions--------*/
