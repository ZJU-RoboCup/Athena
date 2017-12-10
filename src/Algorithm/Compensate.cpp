#include "Compensate.h"
#include "Global.h"
#include <./tinyxml/ParamReader.h>
#include "PlayInterface.h"
#include "Eigen\Dense"

#define COMPENSATION_TEST_MODE false
#define DEBUG if(COMPENSATION_TEST_MODE)

using namespace Utils;


namespace{
	bool record = false;
	bool IS_SIMULATION = false;
}

CCompensate::CCompensate(){
	DECLARE_PARAM_READER_BEGIN(General)
	READ_PARAM(IS_SIMULATION)
	DECLARE_PARAM_READER_END
	readCompensateTable();
}

void CCompensate::readCompensateTable(){
	const string path = "play_books\\";
	string sCarNum;

	//cout << "reading playbooks:" << endl;

	stringstream fullname;
	fullname << path << COMPENSATE_FILE_NAME;
	cout << fullname.str() << endl;
	//cout << fullname.str()<<endl;

	ifstream infile(fullname.str().c_str());
	if (!infile) {
		cerr << "error opening file data" << endl;
		exit(-1);
	}
	string line;
	int rowcount;
	int columncount;
	getline(infile, line);
	istringstream lineStream(line);
	lineStream >> rowcount >> columncount;
	for (int i = 0; i < rowcount; i++)
	{
		getline(infile, line);
		istringstream lineStream(line);
		for (int j = 0; j < columncount; j++)
		{
			lineStream >> compensatevalue[i][j];
		}
	}
}

//lua 调用接口
double CCompensate::getKickDir(int playerNum, CGeoPoint kickTarget){
	/*cout << "start" << endl;
	cout << (int)ourRobotIndex[0] <<endl;
	cout << (int)ourRobotIndex[1] << endl;
	cout << (int)ourRobotIndex[2] << endl;
	cout << (int)ourRobotIndex[3] << endl;
	cout << (int)ourRobotIndex[4] << endl;
	cout << (int)ourRobotIndex[5] << endl;
	*/
	CVisionModule* pVision = vision;
	const MobileVisionT & ball = pVision->Ball();
	const PlayerVisionT & kicker = pVision->OurPlayer(playerNum);
	double rawkickdir = (kickTarget - kicker.Pos()).dir();
	double ballspeed =ball.Vel().mod();
	double tempdir = (Normalize(Normalize(pVision->Ball().Vel().dir()+Param::Math::PI)-(kickTarget - kicker.Pos()).dir()))*180/Param::Math::PI;
	int ratio = 0;
	if (tempdir>0){
		ratio = 1;
	}else{
		ratio = -1;
	}
	double compensatevalue;
	double rawdir=
		fabs((Normalize(Normalize(pVision->Ball().Vel().dir()+Param::Math::PI)-(kickTarget - kicker.Pos()).dir()))
		*180/Param::Math::PI);


	//cout << pVision->Cycle() << endl;
	//cout << "A" << Normalize(pVision->Ball().Vel().dir() + Param::Math::PI) << endl;
	//cout << "B" << (kickTarget - kicker.Pos()).dir() << endl;
	//cout << "rawdir:"<<rawdir << endl;

	if (rawdir > 80){
		rawdir = 80;
		//cout<<"kickdirection rawdir changed to 80"<<endl;
	}
	
	int realPlayerNum = PlayInterface::Instance()->getRealIndexByNum(playerNum);
	//int realPlayerNum = ourRobotIndex[playerNum-1];

	compensatevalue = checkCompensate(realPlayerNum, ballspeed, rawdir);
	if (pVision->Ball().Vel().mod()<10){
		//cout<<"Compensate 0 because the ball's velocity is too small (<10)"<<endl;
		compensatevalue = 0;
	}

	//if(IS_SIMULATION){
	//	compensatevalue = 0;
	//}

	DEBUG cout << "----------Current Compensation---------" << endl;
	DEBUG cout << "Car Num:" << realPlayerNum << "\tTable Value:" << compensatevalue << endl;
	DEBUG cout << "Ball Speed:" << ballspeed << "\tRaw Dir:" << rawdir << endl;
	DEBUG cout << "Real Fix Value:" << ratio*compensatevalue << endl;

	double realkickdir= Utils::Normalize(Utils::Normalize(ratio*compensatevalue*Param::Math::PI/180)+rawkickdir);
	//cout<<vision->Cycle()<<" "<<ballspeed<<" "<<rawdir<<" "<<compensatevalue<<endl;
	return realkickdir;
}

//核心接口
double CCompensate::checkCompensate(int playernum,double ballspeed,double rawdir){
	double compensate = 0;
	if (ballspeed<195){
		ballspeed = 195;
	}
	if (ballspeed > 650){
		ballspeed = 650;
	}
	int column = ceil(rawdir/5);
	int  row =ceil((ballspeed-195)/5);
	if (row<1){
		row = 1;
	}
	if (column < 1){
		column = 1;
	}
	double distleft = rawdir -(column-1)*5;
	double distright = column*5 - rawdir;
	double distup = ballspeed - ((row -1)*5+195);
	double distdown = row*5+195 - ballspeed;
	double leftfactor = distright/5;
	double rightfactor = distleft/5;
	double upfactor = distdown/5;
	double downfactor = distup/5;

	/*for testing: start
	std::cout << "distleft" << distleft << "\t"
		<< "distright" << distright << "\t"
		<< "distup" << distup << "\t"
		<< "distdown" << distdown << std::endl;
	for testing: end*/
		
	compensate = (leftfactor*compensatevalue[row - 1][column - 1] + rightfactor*compensatevalue[row - 1][column])*upfactor+ (leftfactor*compensatevalue[row][column - 1] + rightfactor*compensatevalue[row][column])*downfactor;
	
	

	//cout << "C++ compensate:" << compensate<<endl;
	if (record == true){
		compensate = 0;
	}

	return compensate*COMPENSATE_RATION;
}

void CCompensate::setOurRobotIndex(unsigned char*_ourRobotIndex)
{
	//std::cout << "in set ourrobotindex()" << std::endl;
	for (int i = 0; i < 6; i++) {
		ourRobotIndex[i] = _ourRobotIndex[i];
		//std::cout << (int)ourRobotIndex[i] << std::endl;
	}
}



//---------------------------------------------------------------------------------------
//
//---------------------------------------------------------------------------------------
//
//---------------------------------------------------------------------------------------
//
//---------------------------------------------------------------------------------------
//
//---------------------------------------------------------------------------------------
//
//---------------------------------------------------------------------------------------
//



CCompensateNew::CCompensateNew():A0(0), B0(0), cmpSampleNum(0), initSampleNum(0), sigmaInit(25), sigmaCmp(25), adjustNum(0), adjustAngle(0){
	DECLARE_PARAM_READER_BEGIN(General)
	READ_PARAM(IS_SIMULATION)
	DECLARE_PARAM_READER_END
	readCompensateTable();
	// 这个有啥用？ 
	// initialize compensateData

	lastA[0] = 10000;
	lastA[1] = 10000;
}

void CCompensateNew::readCompensateTable(){
	// 这个表playgame里面表示的是参数
	const string path = "play_books\\";
	string sCarNum;

	stringstream fullname;
	fullname << path << COMPENSATE_FILE_NAME;
	cout << fullname.str() << endl;

	ifstream infile(fullname.str().c_str());
	if (!infile) {
		cerr << "error opening file data" << endl;
		exit(-1);
	}
	string line;
	int rowcount;
	int columncount;
	getline(infile, line);
	istringstream lineStream(line);
	lineStream >> rowcount >> columncount >> A0 >> B0;
	for (int i = 0; i < rowcount; i++)
	{
		getline(infile, line);
		istringstream lineStream(line);
		for (int j = 0; j < columncount; j++)
		{
			lineStream >> compensatevalue[i][j];
		}
	 }
}

//lua 调用接口
double CCompensateNew::getKickDir(int playerNum, CGeoPoint kickTarget){
	
	CVisionModule* pVision = vision;
	const MobileVisionT & ball = pVision->Ball();
	const PlayerVisionT & kicker = pVision->OurPlayer(playerNum);
	double rawkickdir = (kickTarget - kicker.Pos()).dir();
	// 算出来的，应该的dir

	double ballspeed =ball.Vel().mod();
	double tempdir = 
	(Normalize(Normalize(pVision->Ball().Vel().dir() +Param::Math::PI)-(kickTarget - kicker.Pos()).dir()))
	*180/Param::Math::PI;

	// 球速方向与rawKickdir的夹角
	int ratio = 0;

	if (tempdir>0){
		ratio = 1;
	}else{
		ratio = -1;
	}

	double compensatevalue;
	double rawdir=
		fabs((Normalize(Normalize(pVision->Ball().Vel().dir()+Param::Math::PI)-(kickTarget - kicker.Pos()).dir()))
		*180/Param::Math::PI);

	if (rawdir > 80){
		rawdir = 80;
		//cout<<"kickdirection rawdir changed to 80"<<endl;
	}
	
	int realPlayerNum = PlayInterface::Instance()->getRealIndexByNum(playerNum);

	// added by FTQ

	double outdir = 0;
	compensatevalue = checkCompensate(realPlayerNum, ballspeed, outdir, rawdir);
	// 在这里计算了补偿角是多少
	// 输入里面有一个是：rawdir，意思是球速-车-射击点的夹角，theta_in + theta_out
	
	if (pVision->Ball().Vel().mod()<10){
		//cout<<"Compensate 0 because the ball's velocity is too small (<10)"<<endl;
		compensatevalue = 0;
	}

	DEBUG cout << "----------Current Compensation---------" << endl;
	DEBUG cout << "Car Num:" << realPlayerNum << "\tTable Value:" << compensatevalue << endl;
	DEBUG cout << "Ball Speed:" << ballspeed << "\tRaw Dir:" << rawdir << endl;
	DEBUG cout << "Real Fix Value:" << ratio*compensatevalue << endl;

	double realkickdir= Utils::Normalize(Utils::Normalize(ratio*compensatevalue*Param::Math::PI/180)+rawkickdir);
	//cout<<vision->Cycle()<<" "<<ballspeed<<" "<<rawdir<<" "<<compensatevalue<<endl;
	return realkickdir;
}

//核心接口
double CCompensateNew::checkCompensate(int playernum,double ballspeed,double rawdir, double outdir, bool showFlag){
	
	double compensate = 0;
	double compensateOriginal = 0;
	if (ballspeed<195){
		ballspeed = 195;
	}
	if (ballspeed > 650){
		ballspeed = 650;
	}
	// cout << "checkCompensate :: " << ballspeed << ' ';
	// 奇妙的经验参数
	
	int column = ceil(rawdir/5); // 返回大于.. 的最小整数
	int  row =ceil((ballspeed-195)/5); // 这个是查表用的吧
	if (row<1){
		row = 1;
	}
	if (column < 1){
		column = 1;
	}
	double distleft = rawdir -(column-1)*5;
	double distright = column*5 - rawdir;
	double distup = ballspeed - ((row -1)*5+195);
	double distdown = row*5+195 - ballspeed;
	double leftfactor = distright/5;
	double rightfactor = distleft/5;
	double upfactor = distdown/5;
	double downfactor = distup/5;

	
	// here for test!!!!!
	compensateOriginal = (leftfactor*compensatevalue[row - 1][column - 1] + rightfactor*compensatevalue[row - 1][column])*upfactor+ (leftfactor*compensatevalue[row][column - 1] + rightfactor*compensatevalue[row][column])*downfactor;
	compensate = std::asin( (A0 * ballspeed * std::sin(rawdir * Param::Math::PI / 180) + B0) / SHOOT_SPEED ) * 180 / Param::Math::PI;
	
	//double a = adjustCmpParam();
	if (adjustNum <= 1) adjustAngle = 0;
	compensate += adjustAngle;
	// 模型要改
	//cout << compensateOriginal <<' '<< compensate << endl;
	if (showFlag){
		cout << "A0:" << A0 << " B0:" << B0 << endl;
		cout << "ballspeed " << ballspeed << ' ' << "rawdir" << rawdir << endl;
		cout << "C++ compensate:" << compensate<<endl;
		cout << "C++ compensateOriginal:" << compensateOriginal<<endl;
	}
	
	if (record == true){
		compensate = 0;
	}

	return compensate * COMPENSATE_RATION;
}

double CCompensateNew::adjustCmpParam(double v_out_dir, double player2pos, double shoot_vel, double shoot_dir) {
	// 这里传进来的shoot_dir 是弧度
	adjustingData[adjustNum][0] = v_out_dir;
	adjustingData[adjustNum][1] = player2pos;
	double a = 0;

	adjustAngle = a / adjustNum; 
	cout << adjustAngle << " " << " " << adjustNum << endl;
	// 修改参数A B
	if (abs(adjustingData[adjustNum][0] - adjustingData[adjustNum][1]) >= 8 * Param::Math::PI / 180)
		a = a + (adjustingData[adjustNum][0] - adjustingData[adjustNum][1]) / abs(adjustingData[adjustNum][0] - adjustingData[adjustNum][1]) * 8 * Param::Math::PI / 180;
	else if (abs(adjustingData[adjustNum][0] - adjustingData[adjustNum][1]) <= 3 * Param::Math::PI / 180)
		a = a + (adjustingData[adjustNum][0] - adjustingData[adjustNum][1]);
	// a 是这次踢球的偏差

	double compensate = std::asin((A0 * shoot_vel * std::sin(shoot_dir )) / SHOOT_SPEED); // 弧度
	A0 = std::sin(compensate + a / 8) * SHOOT_SPEED / (shoot_vel * std::sin(shoot_dir));  // 根据偏差值补偿
	// 更新
	cout << "A0 = " << A0 << " a = " << a << " compensate = " << compensate << endl;
	//compensate = compensate + a;
	adjustNum++;
	return adjustAngle * 180 / Param::Math::PI;
}

void CCompensateNew::setOurRobotIndex(unsigned char*_ourRobotIndex)
{
	//std::cout << "in set ourrobotindex()" << std::endl;
	for (int i = 0; i < 6; i++) {
		ourRobotIndex[i] = _ourRobotIndex[i];
		//std::cout << (int)ourRobotIndex[i] << std::endl;
	}
}

void CCompensateNew::getData(double shoot_vel, double shoot_dir, double out_dir)
{
	// adjust INIT_CMP_PARAM when needed
	if (!ISTESTRANSAC) {
		// init compensate parameters
		if (INIT_CMP_PARAM) {
			initCmpData[initSampleNum][0] = shoot_vel;
			initCmpData[initSampleNum][1] = shoot_dir;
			initCmpData[initSampleNum][2] = out_dir;
			if (initSampleNum >= 5) {
				lastA[0] = lastA[1]; lastB[0] = lastB[1];
				lastA[1] = A;		 lastB[1] = B;
				initParam(initSampleNum);
				if (initConverge()) {
					cout << "Initialize Converge, Stop!!!!" << endl;
					A0 = A; 
					B0 = B;
				}
			}
			initSampleNum++;
		}
		// adjust compensate parameter
		else {
			// 调整补偿
			compensateData[cmpSampleNum][0] = shoot_vel;
			compensateData[cmpSampleNum][1] = shoot_dir;
			compensateData[cmpSampleNum][2] = out_dir;
			cout << shoot_vel << ' ' << shoot_dir << ' ' << out_dir << endl;
			cmpSampleNum++;
			if (cmpSampleNum >= 5) {
				lastA[0] = lastA[1]; lastB[0] = lastB[1];
				lastA[1] = A;		 lastB[1] = B;
				checkModel(cmpSampleNum);
				if (initConverge()) {
					cout << "Param Checking Converge, see if model changes" << endl;
					// judge if model changes
					// checkParam(A, B);
					double newA = A; double newB = B;
					double maxXaxis = 300;
					if (abs(300 * A0 - 300 * newA) > 0.5 * sigmaCmp) {
						cout << "warning!! Compensate Parameters may change!!" << endl;
						cout << newA << ' ' << newB << endl;
						A0 = A;  B0 = B;
						// 内层闭环的补偿更新
					}
				}
			}
		}
	}
	// test mode
	else {
		//  test RANSAC, load data from txt file
		if (INIT_CMP_PARAM) {
			initCmpData[initSampleNum][0];
			static stringstream fullname("play_books\\testKickData.txt");

			static ifstream infile(fullname.str().c_str());
			if (!infile) {
				cerr << "error opening file data" << endl;
				exit(-1);
			}
			string line;
			getline(infile, line);
			istringstream lineStream(line);
			lineStream >> initCmpData[initSampleNum][0] >> initCmpData[initSampleNum][1] >> initCmpData[initSampleNum][2];
			initSampleNum++;
			lineStream >> initCmpData[initSampleNum][0] >> initCmpData[initSampleNum][1] >> initCmpData[initSampleNum][2];
			//cout << "Read from testKickData.txt"<<initCmpData[initSampleNum][0] << ' ' << initCmpData[initSampleNum][1] << ' ' << initCmpData[initSampleNum][2] << endl;;
			if (initSampleNum >= 5) {
				lastA[0] = lastA[1]; lastB[0] = lastB[1];
				lastA[1] = A; lastB[1] = B;
				checkModel(initSampleNum);
				if (!initConverge()) {
					//lastA[0] = 1000000;
					//lastA[1] = 1000000;
				}
				else {
					cout << "Initialize Converge, stop!!!" << endl;
				}
			}

			if (initSampleNum >= 40) {
				cout << "Data is out" << endl;
			}

			initSampleNum++;
		}
		else {
			// check Parameter test: new data in Japan
			compensateData[cmpSampleNum][0];

			static stringstream fullname("play_books\\testCheckingModel.txt");

			static ifstream infile(fullname.str().c_str());
			if (!infile) {
				cerr << "error opening file data" << endl;
				exit(-1);
			}
			string line;
			getline(infile, line);
			istringstream lineStream(line);
			lineStream >> compensateData[cmpSampleNum][0] >> compensateData[cmpSampleNum][1] >> compensateData[cmpSampleNum][2];
			cmpSampleNum++;
			lineStream >> compensateData[cmpSampleNum][0] >> compensateData[cmpSampleNum][1] >> compensateData[cmpSampleNum][2];
			//cout << "Read from testKickData.txt"<<compensateData[cmpSampleNum][0] << ' ' << compensateData[cmpSampleNum][1] << ' ' << compensateData[cmpSampleNum][2] << endl;;

			if (cmpSampleNum >= 5) {
				lastA[0] = lastA[1]; lastB[0] = lastB[1];
				lastA[1] = A; lastB[1] = B;
				checkModel(cmpSampleNum);
				if (!initConverge()) {
					//lastA[0] = 1000000;
					//lastA[1] = 1000000;
				}
				else {
					cout << "checking model Converge, see if params changes" << endl;
					// checkParam(A, B);  check if paramters change. send warning message if does
					double newA = A; double newB = B;
					double maxXaxis = 300;
					if (abs(300 * A0 - 300 * newA) > 0.5 * sigmaCmp) {
						cout << "warning!! Compensate Parameters may change!!" << endl;
						cout << newA << ' ' << newB << endl;
					}
				}
			}
			if (cmpSampleNum >= 40) {
				cout << "Data is out" << endl;
			}
			cmpSampleNum++;
		}
	}
}

// check if init parameters converge
bool CCompensateNew::initConverge() {
	double avgA = (lastA[0] + lastA[1] + A) / 3;
	cout << "avgA = " << avgA << "A " << A << endl;
	if (abs(avgA - A) / A <= 0.02) return true;
	else return false;
}

// check if parameters change
// 貌似这个没啥用
void CCompensateNew::checkModel(int num)
{
	if (BAYESINIT) {
		// Bayes Linear Method:
		typedef Eigen::Matrix<double, 2, 2> Matrix;
		typedef Eigen::Matrix<double, 2, 1> Vector;// 列向量

		Vector w; w(0) = 0.5; w(1) = 0;
		Matrix sigma(1000000 * Matrix::Identity()), last_sigma, sigma_0(10 * Matrix::Identity());
		double sigma_d;
		Vector mu; mu(0) = 0; mu(1) = 0;

		Vector X;
		double Y;// temp 

		 /* Bayes Iteration*/
		sigma_d = w.transpose() * sigma_0 * w + 5;
		//sigma = sigma_0 / 10000;

		for (int i = 0; i < num; i++) {
			X(0) = compensateData[i][0] * std::sin(compensateData[i][1]) / SHOOT_SPEED;
			X(1) = 1;
			Y = std::sin(compensateData[i][2]);
			last_sigma = sigma;
			sigma = (X / sigma_d * X.transpose() + last_sigma.inverse()).inverse();
			mu = ((Y / sigma_d * X.transpose() + mu.transpose() * last_sigma.inverse()) * sigma).transpose();
		}

		/* get parameter wNew, see if it differs dramatically*/

		cout << "check model" << mu(0) << ' ' << mu(1) << endl;
		
		double newA = mu(0); double newB = mu(1);
		double maxXaxis = 300;
		if (abs(300 * A0 - 300 * newA) > 0.5 * sigmaCmp) {
			cout << "warning!! Compensate Parameters may change!!" << endl;
			cout << newA << ' ' << newB << endl;
			A0 = newA; B0 = newB;
		}
	}
	if (RANSACINIT) {
		calcParamRANSAC();
		cout << "RANSAC Check" << endl;
	}
}


void CCompensateNew::initParam(int num) {
	const string path = "play_books\\";

	stringstream fullname;
	fullname << path << "ftqTestCmpInit.txt";

	static ofstream outfile(fullname.str().c_str());
	if (!outfile) {
		cerr << "error opening file data" << endl;
		exit(-1);
	}
	if (outfile.is_open()) {
		outfile << initCmpData[num][0] << ' ' << initCmpData[num][1] << ' ' << initCmpData[num][2] << endl;
	}

	if (BAYESINIT) {
		// Bayes Linear Method:
		typedef Eigen::Matrix<double, 2, 2> Matrix;
		typedef Eigen::Matrix<double, 2, 1> Vector;// 列向量

		Vector w; w(0) = 0.5; w(1) = 0;
		Matrix sigma(1000000 * Matrix::Identity()), last_sigma, sigma_0(10 * Matrix::Identity());
		double sigma_d;
		Vector mu; mu(0) = 0; mu(1) = 0;

		Vector X;
		double Y;// temp 

				 /* Bayes Iteration*/
		sigma_d = w.transpose() * sigma_0 * w + 5;
		//sigma = sigma_0 / 10000;

		for (int i = 0; i < num; i++) {
			X(0) = initCmpData[i][0] * std::sin(initCmpData[i][1]) / SHOOT_SPEED;
			X(1) = 1;
			Y = std::sin(initCmpData[i][2]);
			last_sigma = sigma;
			sigma = (X / sigma_d * X.transpose() + last_sigma.inverse()).inverse();
			mu = ((Y / sigma_d * X.transpose() + mu.transpose() * last_sigma.inverse()) * sigma).transpose();
		}
		cout << "Bayes method, init compensate parameters" << mu(0) << ' ' << mu(1) << endl;
	}
	if (RANSACINIT) {
		calcParamRANSAC();
		cout << "RANSAC INIT" << endl;
	}
}
/*------------https://nlopt.readthedocs.io/en/latest/NLopt_Tutorial/------------*/
/*--------------------------------nlopt syntax----------------------------------*/

double CCompensateNew::calcObjFunc(const double *x) {
	double res = 0;
	if (INIT_CMP_PARAM) {
		for (int i = 0; i < initSampleNum; i++) {
			double temp = x[0] * (initCmpData[i][0] * std::sin(initCmpData[i][1])) + x[1] - std::sin(initCmpData[i][2]) * SHOOT_SPEED;
			if (isInterPoint[i]) 	res = res + pow(temp, 2);
			else	res = res + 2 * pow(sigmaInit, 2);
		}
	}
	else {
		for (int i = 0; i < cmpSampleNum; i++) {
			double temp = x[0] * (compensateData[i][0] * std::sin(compensateData[i][1])) + x[1] - std::sin(compensateData[i][2]) * SHOOT_SPEED;
			if (isInterPoint[i]) 	res = res + pow(temp, 2);
			else	res = res + 2 * pow(sigmaCmp, 2);
		}
	}
	return res;
}

void CCompensateNew::calcInitParam() {
	// calculate internal point 
	// compensate = std::asin( (A * ballspeed * std::sin(rawdir * Param::Math::PI / 180) + B) / SHOOT_SPEED ) * 180 / Param::Math::PI;
	// update sigma^2 ;
	if (INIT_CMP_PARAM) {
		int maxNum = 0;
		for (int i = 0; i <= initSampleNum * (initSampleNum - 1) / 2; i++) {
			int sp1 = rand() % (initSampleNum);
			int sp2 = rand() % (initSampleNum);
			while (sp1 == sp2) sp2 = rand() % (initSampleNum);
			double X1 = (initCmpData[sp1][0] * std::sin(initCmpData[sp1][1]));
			double X2 = (initCmpData[sp2][0] * std::sin(initCmpData[sp2][1]));
			double Y1 = std::sin(initCmpData[sp1][2]) * SHOOT_SPEED;
			double Y2 = std::sin(initCmpData[sp2][2]) * SHOOT_SPEED;
			double AA = (Y2 - Y1) / (X2 - X1);
			double BB = (X2 * Y1 - X1 * Y2) / pow((X2 - X1), 2);

			int num = 0;
			double tempSigma = 0;
			// get internal point for sp1 and sp2
			for (int j = 0; j < initSampleNum; j++) {
				double Y = std::sin(initCmpData[j][2]) * SHOOT_SPEED;
				double X = (initCmpData[j][0] * std::sin(initCmpData[j][1]));
				if (pow(Y - (AA * X + BB), 2) <= 2 * pow(sigmaInit, 2)) {
					num = num + 1;
				}
			}
			if (num > maxNum) {
				maxNum = num;
				x_init[0] = AA;
				x_init[1] = BB;
				// update Inter Point
				for (int k = 0; k <= initSampleNum; k++) {
					double Y = std::sin(initCmpData[k][2]) * SHOOT_SPEED;
					double X = (initCmpData[k][0] * std::sin(initCmpData[k][1]));
					if (pow(Y - (AA * X + BB), 2) <= 2 * pow(sigmaInit, 2)) {
						isInterPoint[k] = true;
						tempSigma = tempSigma + pow(Y - (AA * X + BB), 2);
					}
					else {
						isInterPoint[k] = false;
						tempSigma = tempSigma + 2 * pow(sigmaInit, 2);
					}
				}
				sigmaInit = 1.2 * sqrt(tempSigma / initSampleNum);
				//cout << "sigma = " << sigmaInit << " ";
			}
		}
		cout << "init parameter: A " << x_init[0] << " B: " << x_init[1] << endl;
	}
	else {
		//check compensate
		int maxNum = 0;
		for (int i = 0; i <= cmpSampleNum * (cmpSampleNum - 1) / 2; i++) {
			int sp1 = rand() % (cmpSampleNum);
			int sp2 = rand() % (cmpSampleNum);
			while (sp1 == sp2) sp2 = rand() % (cmpSampleNum);
			double X1 = (compensateData[sp1][0] * std::sin(compensateData[sp1][1]));
			double X2 = (compensateData[sp2][0] * std::sin(compensateData[sp2][1]));
			double Y1 = std::sin(compensateData[sp1][2]) * SHOOT_SPEED;
			double Y2 = std::sin(compensateData[sp2][2]) * SHOOT_SPEED;
			double AA = (Y2 - Y1) / (X2 - X1);
			double BB = (X2 * Y1 - X1 * Y2) / pow((X2 - X1), 2);

			int num = 0;
			double tempSigma = 0;
			// get internal point for sp1 and sp2
			for (int j = 0; j < cmpSampleNum; j++) {
				double Y = std::sin(compensateData[j][2]) * SHOOT_SPEED;
				double X = (compensateData[j][0] * std::sin(compensateData[j][1]));
				if (pow(Y - (AA * X + BB), 2) <= 2 * pow(sigmaCmp, 2)) {
					num = num + 1;
				}
			}
			if (num > maxNum) {
				maxNum = num;
				x_init[0] = AA;
				x_init[1] = BB;
				// update Inter Point
				for (int k = 0; k <= cmpSampleNum; k++) {
					double Y = std::sin(compensateData[k][2]) * SHOOT_SPEED;
					double X = (compensateData[k][0] * std::sin(compensateData[k][1]));
					if (pow(Y - (AA * X + BB), 2) <= 2 * pow(sigmaCmp, 2)) {
						isInterPoint[k] = true;
						tempSigma = tempSigma + pow(Y - (AA * X + BB), 2);
					}
					else {
						isInterPoint[k] = false;
						tempSigma = tempSigma + 2 * pow(sigmaCmp, 2);
					}
				}
				sigmaCmp = 1.2 * sqrt(tempSigma / cmpSampleNum);
				//cout << "sigma = " << sigmaCmp << " ";
			}
		}
		cout << "init parameter: A " << x_init[0] << " B: " << x_init[1] << endl;
	}
 }

/* objective function */
/* my_func_data: data needed for the obj function. */
double myfunc(unsigned n, const double *x, double *grad, void *my_func_data)
{
	double res = CompensateNew::Instance()->calcObjFunc(x);	
	return res;
}

/* constraints function */
/*double myconstraint(unsigned n, const double *x, double *grad, void *data)
{
	my_constraint_data *d = (my_constraint_data *)data;
	double a = d->a, b = d->b;
	if (grad) {
		grad[0] = 3 * a * (a*x[0] + b) * (a*x[0] + b);
		grad[1] = -1.0;
	}
	// constraint(x) <= 0
	return ((a*x[0] + b) * (a*x[0] + b) * (a*x[0] + b) - x[1]);
}*/

void CCompensateNew::calcParamRANSAC() {

	nlopt_opt opt;
	double lb[2] = { 0, -2 };  /* lower bounds */

	opt = nlopt_create(NLOPT_LN_COBYLA, 2);  /* algorithm and dimensionality */
	nlopt_set_lower_bounds(opt, lb);
	nlopt_set_min_objective(opt, myfunc, NULL);

	//my_constraint_data data[2] = { {2, 0}, {-1, 1} };

	/* add the two inequality constraints // in our case, no constraints */
	//nlopt_add_inequality_constraint(opt, myconstraint, &data[0], 1e-8);
	//nlopt_add_inequality_constraint(opt, myconstraint, &data[1], 1e-8);
	
	/* a relative tolerance on the optimization parameters x */
	nlopt_set_xtol_rel(opt, 1e-4);

	calcInitParam(); /* some initial guess, A = 0.3, B = 0 */
	double minf; 

	if (nlopt_optimize(opt, x_init, &minf) < 0) {
		cout << "error finding minimum" << endl;
	}
	else {
		cout << "found min at A: " << x_init[0] << " B: " << x_init[1] << endl;
		A = x_init[0]; B = x_init[1];
	}
	nlopt_destroy(opt);
}
/*--------------------------------nlopt syntax----------------------------------*/