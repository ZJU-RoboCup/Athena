#pragma once
#include "Matrix2d.h"
class KalmanFilter {
public:
	KalmanFilter();
	~KalmanFilter();
	void init(double,double,double,double);
	const Matrix2d& update(double, double);
	const Matrix2d& follow(double, double);
private:
	Matrix2d K;
	Matrix2d P;
	Matrix2d x;
	Matrix2d A;
	Matrix2d Q;
	Matrix2d H;
	Matrix2d R;
	Matrix2d x_;
	Matrix2d P_;
	Matrix2d z;
	bool ifInit;
};