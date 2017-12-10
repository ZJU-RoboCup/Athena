/*
 * Matrix2d.h
 */

#ifndef __Matrix2d_KF_H__
#define __Matrix2d_KF_H__

#include <iostream>

class Matrix2d {
    public:
        Matrix2d(int, int);
		Matrix2d(int, int, std::initializer_list<double>);
        Matrix2d();
        ~Matrix2d();
        Matrix2d(const Matrix2d&);
        Matrix2d& operator=(const Matrix2d&);

        inline double& operator()(int x, int y) { return _value[x][y]; }
        inline const double operator()(int x,int y) const {return _value[x][y];}
        inline const int row() const { return _row; }
        inline const int column() const { return _column; }
        Matrix2d& operator+=(const Matrix2d&);
        Matrix2d& operator-=(const Matrix2d&);
        Matrix2d& operator*=(const Matrix2d&);
        Matrix2d& operator*=(double);
        Matrix2d& operator/=(double);
        Matrix2d  operator^(int);
        
        friend std::ostream& operator<<(std::ostream&, const Matrix2d&);
        friend std::istream& operator>>(std::istream&, Matrix2d&);

        void swapRows(int, int);
        Matrix2d transpose();
        Matrix2d inverse();

        static Matrix2d createIdentity(int);
		static Matrix2d createIdentity(int, int);
        static Matrix2d solve(Matrix2d, Matrix2d);
        static Matrix2d bandSolve(Matrix2d, Matrix2d, int);

        // functions on vectors
        static double dotProduct(Matrix2d, Matrix2d);

        // functions on augmented matrices
        static Matrix2d augment(Matrix2d, Matrix2d);
        Matrix2d gaussianEliminate();
        Matrix2d rowReduceFromGaussian();
        void readSolutionsFromRREF(std::ostream& os);

    private:
        int _row, _column;
        double **_value;

        void allocSpace();
        Matrix2d expHelper(const Matrix2d&, int);
};

Matrix2d operator+(const Matrix2d&, const Matrix2d&);
Matrix2d operator-(const Matrix2d&, const Matrix2d&);
Matrix2d operator*(const Matrix2d&, const Matrix2d&);
Matrix2d operator*(const Matrix2d&, double);
Matrix2d operator*(double, const Matrix2d&);
Matrix2d operator/(const Matrix2d&, double);

#endif  //__Matrix2d_KF_H__
