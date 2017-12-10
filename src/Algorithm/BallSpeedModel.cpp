#include "BallSpeedModel.h"
#include <param.h>
#include <cmath>
#include <Windows.h>
#include <string>
#include <iostream>
#include <sstream> 

using namespace std;

namespace{
	//double cal_acc = 31.68;
	const double vel_factor = 1;
	//const double acc_factor = 1;
	//const double time_factor = 1;
	//const double posfortime_factor = 1;
	//const int MAXERRORCOUNT = 15;
	//CGeoPoint lastBallPos = CGeoPoint(0,0);
	const int NOR_MODE = 0;
	const int CHECK_TABLE_MODE = 1;
	const int FRICTION_MODE = 2;
	const int MODE = FRICTION_MODE;
	static int FRICTION;
}
static const double oneFromFive(const double x1,const double y1,const double x2,const double y2,const double x3);

CBallSpeedModel::CBallSpeedModel():_ballVel(CVector(0,0)),_lastBallVel(CVector(0,0)),_ballPos(CGeoPoint(-9999,-9999)),cal_acc(20),_last_dist(0){
	static string TABLE_FILE;
	switch(MODE){
	case CHECK_TABLE_MODE:
		{
			DECLARE_PARAM_READER_BEGIN(BallSpeedModel)
				READ_PARAM(TABLE_FILE)
			DECLARE_PARAM_READER_END
			readTableFile_CheckTable(TABLE_FILE);
		}
		break;
	case FRICTION_MODE: 
		{
			DECLARE_PARAM_READER_BEGIN(FieldParam)
				READ_PARAM(FRICTION)
			DECLARE_PARAM_READER_END
		}
		break;
	default:
		break;
	}
}
CBallSpeedModel::~CBallSpeedModel(){
	delete[] _speed_data;
	delete[] _dist_data;
}

//todo
CVector CBallSpeedModel::speedForTime(double frame, const CVisionModule* pVision ){
	update(pVision);
	switch (MODE) {
	case CHECK_TABLE_MODE:
		break;
	case FRICTION_MODE:
		return speedForTime_FM(frame, pVision);
		break;
	default:
		double my_time = frame / Param::Vision::FRAME_RATE;
		double vel_mod = _ballVel.mod() - cal_acc * my_time;
		int max_time = _ballVel.mod() / cal_acc;
		if (max_time > my_time)
			return _ballVel * vel_mod / (vel_factor * _ballVel.mod());
		else
			return CVector(0, 0);
	}
	return CVector(0, 0);
}

//todo
CVector CBallSpeedModel::speedForDist(double dist, const CVisionModule* pVision) {
	update(pVision);
	switch (MODE) {
	case CHECK_TABLE_MODE:
		break;
	case FRICTION_MODE:
		return speedForDist_FM(dist,pVision);
		break;
	default:
		double vel_mod = _ballVel.mod() * _ballVel.mod() - 2 * cal_acc * dist;
		return _ballVel * vel_mod / (vel_factor * _ballVel.mod());
	}
}

//use new speed mode
double CBallSpeedModel::timeForDist(double dist, const CVisionModule* pVision ){
	update(pVision);
	switch(MODE){
	case CHECK_TABLE_MODE:{
			return timeForDist_CheckTable(dist);
			break;
		}
	case FRICTION_MODE:
		return timeForDist_FM(dist, pVision);
		break;
	default:{
			double final_dist = _ballVel.mod() * _ballVel.mod() / (2 * cal_acc);
			if(final_dist > dist){
				return (_ballVel.mod() - sqrt(_ballVel.mod() * _ballVel.mod() - 2 * cal_acc * dist)) / cal_acc;
			}
			else
				return -1;
			break;
		}
	}
}

//use new speed mode
CGeoPoint CBallSpeedModel::posForTime(double frame, const CVisionModule* pVision ){
	update(pVision);
	switch(MODE){
	case CHECK_TABLE_MODE:
		return _ballPos + Utils::Polar2Vector(distForTime_CheckTable(frame),_ballVel.dir());
		break;
	case FRICTION_MODE:
		return posForTime_FM(frame, pVision);
		break;
	default:
		{
			if (frame < 0) {
				return pVision->Ball().Pos();
			}
			double my_time = frame / Param::Vision::FRAME_RATE;
			double dist = _ballVel.mod() * my_time - 0.5 * cal_acc * my_time * my_time;
			dist = (dist + _last_dist) / 2;
			_last_dist = dist;
			CGeoPoint final_pos;
			if (_ballVel.mod() < 0.5) {
				final_pos = _ballPos;
			}
			else {
				final_pos = _ballPos + _ballVel * dist / _ballVel.mod();
			}
			return final_pos;
		}
	}
}
CVector CBallSpeedModel::speedForTime_FM(double frame, const CVisionModule* pVision) {
	auto vel = _ballVel.mod() - 0.5 * FRICTION * frame*1.0 / Param::Vision::FRAME_RATE;
	if (vel < 0) vel = 0;
	return _ballVel / _ballVel.mod() * vel;
}
CVector CBallSpeedModel::speedForDist_FM(double dist, const CVisionModule* pVision) {
	auto v = _ballVel.mod();
	auto t = v*v - FRICTION*dist;
	if (t <= 0) return CVector(0, 0);
	auto vel = std::sqrt(t);
	return _ballVel / _ballVel.mod() * vel;
}
double CBallSpeedModel::timeForDist_FM(double dist, const CVisionModule* pVision) {
	auto v = _ballVel.mod();
	auto a = 0.25*FRICTION;
	auto b = -v;
	auto c = dist;
	auto delta = b*b - 4 * a*c;
	if (delta < 0) return -1;
	return 1.0 / (2 * a)*(-b - std::sqrt(delta)) * Param::Vision::FRAME_RATE;
}
CGeoPoint CBallSpeedModel::posForTime_FM(double frame, const CVisionModule* pVision) {
	auto v0 = _ballVel.mod();
	auto v1 = v0 - 2 * FRICTION*frame/Param::Vision::FRAME_RATE;
	double d;
	if (v1 < 0)
		d = v0*v0 / FRICTION;
	else
		d = (v0 + v1)*frame / Param::Vision::FRAME_RATE / 2;
	return _ballPos + (_ballVel / _ballVel.mod()*d);
}
//todo
double CBallSpeedModel::CalKickInitSpeed(const double dist)
{
	double vt = 100;
	if (dist < 100) {
		vt = 120;
	} else if (dist >= 100 && dist < 200) {
		vt = 200;
	} else if (dist >= 200 && dist < 300) {
		vt = 350;
	} else if (dist >= 300 && dist < 400) {
		vt = 350;
	} else if (dist >= 400) {
		vt = 350;
	}
	vt = vt+100;//让传球速度变小一点
	double ballVO = sqrt(vt * vt + 2 * 20 * dist);
	//迭代法计算初速度
	for (int i = 1;i<=3;i++){
		if(ballVO > 240){
			cal_acc = 10;
		} else if(ballVO > 190 && ballVO <= 240){
			cal_acc = ( 0.0025 * ballVO + 0.18 ) * 60;
		} else if(ballVO > 140 && ballVO <= 190){
			cal_acc = ( (0.0025 + ( 190 - ballVO) *0.00001)  * ballVO + 0.18 ) * 60;
		} else if (ballVO > 10 && ballVO <= 140){
			cal_acc = ( 0.003  * ballVO + 0.18 ) * 60;
		}
		ballVO = sqrt(vt * vt + 2 * cal_acc * dist);
	}
	return ballVO;
}
void CBallSpeedModel::update( const CVisionModule* pVision ){
	//lastBallPos = _ballPos;
	_ballPos = pVision->Ball().RawPos();
	if(pVision->Ball().Vel().mod() > 240){
		cal_acc = 10;
	} else {
		double cal_norm = 0.34 * 60;
		//double cal_cali = 0.0025 * pVision->Ball().Vel().mod() + 0.18;
		/*if(cal_norm){
			
		}*/
		cal_acc = cal_norm;
	}
	// TODO
	cal_acc = 64;
	_ballVel = pVision->Ball().Vel() * vel_factor;
}

//double CBallSpeedModel::getAcc(double speed) const{
//	if(speed >= 500) return 1.7;
//	if(speed > 300) return speed/200.0 - 0.8;	//300 - 500 line : (300,0.7) , (500,1.7)
//	if(speed >= 0.7) return 0.7;
//	if(speed >= 0) return speed;
//	cout << "BallSpeedModel.cpp : Speed Mod < 0 in getAcc Function" << endl;
//	return -1;
//}

const double CBallSpeedModel::timeForDist_CheckTable(const double dist) const{
	double startTime,endTime;
	double startDist,endDist;
	startTime	= checkTime_CheckTable(_speed_data,_ballVel.mod());
	startDist	= checkData_CheckTable(_dist_data,startTime);
	endDist		= startDist + dist;
	endTime		= checkTime_CheckTable(_dist_data, endDist);
	return endTime - startTime;
}
const double CBallSpeedModel::distForTime_CheckTable(const int frame) const{
	double startTime,endTime;
	double startDist,endDist;
	startTime	= checkTime_CheckTable(_speed_data,_ballVel.mod());
	startDist	= checkData_CheckTable(_dist_data,startTime);
	endTime		= startTime + frame;
	endDist		= checkData_CheckTable(_dist_data,endTime);
	return endDist - startDist;
}
const double CBallSpeedModel::checkTime_CheckTable(const double const* _array,const double data) const{
	//use method of bisection
	//falling data ↓
	//return the index of the not-larger data
	int start = 0;
	int end = _num;
	int mid;
	if(data >= _array[0]){
		end = 1;
	}else {
		while(end - start > 1){
			mid = (start + end)/2;
			if(_array[mid] > data){
				start = mid;
			}else{
				end = mid;
			}
		}
	}
	return oneFromFive(_array[end],end,_array[start],start,data);
}
const double CBallSpeedModel::checkData_CheckTable(const double const* _array,const double time) const{
	int index = (int)time;
	index = index == 0 ? index + 1 : index;
	return oneFromFive(index,_array[index],index+1,_array[index+1],time);
}
const bool CBallSpeedModel::readTableFile_CheckTable(const string& filename){
	const string fullname = string("params\\") + filename + string(".cfg");
	HANDLE hFile = CreateFile(fullname.data(),GENERIC_WRITE | GENERIC_READ,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	DWORD dwFileSize=GetFileSize(hFile,NULL);
	HANDLE hFileMap=CreateFileMapping(hFile,NULL,PAGE_READWRITE,0,dwFileSize+sizeof(char),NULL);
	PVOID pvFile=MapViewOfFile(hFileMap,FILE_MAP_WRITE,0,0,0);
	PSTR pch=(PSTR)pvFile;
	stringstream inputStream(pch);
	inputStream >> _num;

	_speed_data	= new double[_num];
	_dist_data	= new double[_num];
	for(int i=0;i<_num;i++){
		inputStream >> _speed_data[i];// >> _distance_data[i];
	}

	UnmapViewOfFile(pvFile);
	CloseHandle(hFileMap);
	SetFilePointer(hFile,dwFileSize,NULL,FILE_BEGIN);
	SetEndOfFile(hFile);
	CloseHandle(hFile);
	return true;
}


static const double oneFromFive(const double x1,const double y1,const double x2,const double y2,const double x3){
	return (y2-y1)/(x2-x1)*(x3-x2)+y2;
}