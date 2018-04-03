#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>


/************************************************************************/
/*                        CVector                                       */
/************************************************************************/
//class CVector{
//public:
//	CVector() : _vector(0,0){}
//	CVector(double x, double y) : _vector(x,y){ }
//	CVector(const R::Vector_2& v) : _vector(v){ }
//	double mod() const { return std::sqrt(_vector.squared_length());}
//	double mod2() const { return _vector.squared_length(); }
//	double dir() const ;
//	double x() const { return _vector.x();	}
//	double y() const { return _vector.y();	}
//	double value(double angle) const { return mod()*std::cos(dir()-angle); } ///<在角度angle的分量
//	const R::Vector_2& CGALVector() const { return _vector; }
//	CVector operator +(const CVector& v) const { return _vector + v.CGALVector(); }
//	CVector operator -(const CVector& v) const { return _vector - v.CGALVector(); }
//	CVector operator *(double a) const { return _vector * a; }
//	double operator *(CVector a) const { return (_vector.x() * a.x() + _vector.y() * a.y()); }
//	CVector operator /(double a) const { return _vector /a;	}
//	CVector operator - () const { return - _vector;	}
//	CVector rotate(double angle) const;
//	CVector perp(){return CVector(_vector.y(),-_vector.x());}
//	friend std::ostream& operator << (std::ostream& os,const CVector& v){ return os << "(" << v.x() << ":" << v.y() << ")";	}
//private:
//	R::Vector_2 _vector;
//};
double CNormalize(double angle);
class CVector {
public:
	CVector() :	_x(0), _y(0) {}
	CVector(double x, double y) : _x(x), _y(y) {}
	CVector(const CVector& v) :	_x(v.x()), _y(v.y()) {}
	double mod() const { return std::sqrt(_x * _x + _y * _y); }
	double mod2() const { return (_x * _x + _y * _y); }
	double dir() const { return std::atan2(y(), x());}
	CVector rotate(double angle) const;
	double x() const { return _x; }
	double y() const { return _y; }
	double value(double angle) const { return mod() * std::cos(dir() - angle); }
	CVector operator +(const CVector& v) const { return CVector(_x + v.x(), _y + v.y()); }
	CVector operator -(const CVector& v) const { return CVector(_x - v.x(), _y - v.y()); }
	CVector operator *(double a) const { return CVector(_x * a, _y * a); }
	double operator *(CVector b) const { return double(_x*b.x() + _y*b.y()); } //向量点乘
	CVector operator /(double a) const { return CVector(_x / a, _y / a); }
	CVector operator -() const { return CVector(-1 * _x, -1 * _y); }
	friend std::ostream& operator <<(std::ostream& os, const CVector& v) {
		return os << "(" << v.x() << ":" << v.y() << ")";
	}

private:
	double _x, _y;
};

/************************************************************************/
/*                       CGeoPoint                                      */
/************************************************************************/
//class CGeoPoint{
//public:
//	CGeoPoint() : _point(0,0){}
//	CGeoPoint(double x,double y) : _point(x,y){ }
//	CGeoPoint(const R::Point_2& p) : _point(p) { }
//	const R::Point_2& CGALPoint() const { return _point; }
//	double x() const { return _point.x(); }
//	double y() const { return _point.y(); }
//	double dist(const CGeoPoint& p) const { return CVector(p.CGALPoint()- _point).mod(); }
//	double dist2(const CGeoPoint& p) const { return CVector(p.CGALPoint()- _point).mod2(); }
//	CGeoPoint operator+( const CVector& v) const { return CGeoPoint( _point + v.CGALVector() );	}
//	CVector operator-( const CGeoPoint& p) const { return CVector( _point - p.CGALPoint()); }
//	friend std::ostream& operator << (std::ostream& os,const CGeoPoint& v){ return os << "(" << v.x() << ":" << v.y() << ")";	}
//	bool operator==(const CGeoPoint& p) const {return p.x() == _point.x() && p.y() == _point.y(); }
//private:
//	R::Point_2 _point;
//};
class CGeoPoint {
public:
	CGeoPoint() : _x(0), _y(0) {}
	~CGeoPoint(){}
	CGeoPoint(double x, double y) : _x(x), _y(y) {}
	CGeoPoint(const CGeoPoint& p) : _x(p.x()), _y(p.y()) {}
	bool operator==(const CGeoPoint& rhs) { return ((this->x() == rhs.x()) && (this->y() == rhs.y())); }
	double x() const { return _x; }
	double y() const { return _y; }
	void setX(double x) { _x = x; }   // 2014/2/28 新增 设置x坐标 yys
	void setY(double y) { _y = y; }   // 2014/2/28 新增 设置y坐标 yys
	double dist(const CGeoPoint& p) const { return CVector(p - CGeoPoint(_x, _y)).mod(); }
	double dist2(const CGeoPoint& p) const { return CVector(p - CGeoPoint(_x, _y)).mod2(); }
	CGeoPoint operator+(const CVector& v) const { return CGeoPoint(_x + v.x(), _y + v.y()); }
	CGeoPoint operator*(const double& a) const { return CGeoPoint(_x * a, _y * a); }
	CVector operator-(const CGeoPoint& p) const { return CVector(_x - p.x(), _y - p.y()); }
	CGeoPoint midPoint(const CGeoPoint& p) const{ return CGeoPoint((_x + p.x()) / 2, (_y + p.y()) / 2); }
	friend std::ostream& operator <<(std::ostream& os, const CGeoPoint& v) {
		return os << "(" << v.x() << ":" << v.y() << ")";
	}

private:
	double _x, _y;
};

/************************************************************************/
/*                        CGeoLine                                      */
/************************************************************************/
//class CGeoLine{
//public:
//	CGeoLine(){}
//	CGeoLine( const CGeoPoint& p1,const CGeoPoint& p2) : _line(p1.CGALPoint(),p2.CGALPoint()){
//	_point1 = p1; _point2 = p2;}
//	CGeoLine( const CGeoPoint& p,double angle) 
//		: _line(p.CGALPoint(),R::Point_2( p.x() + std::cos(angle),p.y() + std::sin(angle))){
//	_point1 = p; _point2 = CGeoPoint( p.x() + std::cos(angle),p.y() + std::sin(angle));}
//	const R::Line_2& CGALLine() const { return _line; }
//	CGeoPoint projection(const CGeoPoint& p) const { return _line.projection(p.CGALPoint()); }
//	const CGeoPoint& point1() const { return _point1; }
//	const CGeoPoint& point2() const { return _point2; }
//private:
//	friend class CGeoLineLineIntersection;
//	friend class CGeoLineSegmentIntersection;
//	CGeoPoint _point1;
//	CGeoPoint _point2;
//	R::Line_2 _line;
//};
class CGeoLine {
public:
	CGeoLine() {}
	CGeoLine(const CGeoPoint& p1, const CGeoPoint& p2) : _p1(p1), _p2(p2) {
		calABC();
	}
	CGeoLine(const CGeoPoint& p, double angle) : _p1(p), _p2(p.x() + std::cos(angle), p.y() + std::sin(angle)) {
		calABC();
	}
	void calABC(){
		if(_p1.y() == _p2.y()){
			_a = 0;
			_b = 1;
			_c = -1.0 * _p1.y();
		} else{
			_a = 1;
			_b = -1.0 * (_p1.x() - _p2.x()) / (_p1.y() - _p2.y());
			_c = (_p1.x()*_p2.y() - _p1.y()*_p2.x()) / (_p1.y() - _p2.y());
		}
	}

	//投影点
	CGeoPoint projection(const CGeoPoint& p) const {
		if (_p2.x() == _p1.x()) {
			return CGeoPoint(_p1.x(), p.y());
		} else {
			// 如果该线段不平行于X轴也不平行于Y轴，则斜率存在且不为0。设线段的两端点为pt1和pt2，斜率为：
			double k = (_p2.y() - _p1.y()) / (_p2.x() - _p1.x());
			// 该直线方程为:					y = k* ( x - pt1.x) + pt1.y
			// 其垂线的斜率为 -1/k,垂线方程为:	y = (-1/k) * (x - point.x) + point.y
			// 联立两直线方程解得:
			double x = (k * k * _p1.x() + k * (p.y() - _p1.y()) + p.x()) / (k * k + 1);
			double y = k * (x - _p1.x()) + _p1.y();
			return CGeoPoint(x, y);
		}
	}
	CGeoPoint point1() const { return _p1; }
	CGeoPoint point2() const { return _p2; }
	bool operator==(const CGeoLine& rhs) 
	{
		return ((this->point1().x() == rhs.point1().x()) && (this->point1().y() == rhs.point1().y())
			&& (this->point2().x() == rhs.point2().x()) && (this->point2().y() == rhs.point2().y()));
	}
	const double& a() const{ return _a; }
	const double& b() const{ return _b; }
	const double& c() const{ return _c; }
private:
	CGeoPoint _p1;
	CGeoPoint _p2;

	// 直线的解析方程 a*x+b*y+c=0 为统一表示，约定 a>= 0
	double _a;
	double _b;
	double _c;
};

class CGeoLineLineIntersection{
public:
	CGeoLineLineIntersection(const CGeoLine& line_1, const CGeoLine& line_2);
	bool Intersectant() const { return _intersectant;	}
	const CGeoPoint& IntersectPoint() const { return _point; }
private:
	bool _intersectant;
	CGeoPoint _point;
};
/************************************************************************/
/*                       CGeoSegment / 线段                             */
/************************************************************************/
//class CGeoSegment : public CGeoLine{
//public:
//	CGeoSegment(){}
//	CGeoSegment(const CGeoPoint& p1, const CGeoPoint& p2) : CGeoLine(p1, p2), _start(p1), _end(p2),
//		_segment(p1.CGALPoint(), p2.CGALPoint())
//	{ 
//		_compareX = std::abs(p1.x() - p2.x()) > std::abs(p1.y() - p2.y());
//	}
//	bool IsPointOnLineOnSegment(const CGeoPoint& p) const // 直线上的点是否在线段上
//	{
//		if( _compareX ){
//			return p.x() > (std::min)(_start.x(), _end.x()) && p.x() < (std::max)(_start.x(), _end.x());
//		}
//		return p.y() > (std::min)(_start.y(), _end.y()) && p.y() < (std::max)(_start.y(), _end.y());
//	}
//	const CGeoPoint& start() const { return _start; }
//	const CGeoPoint& end() const { return _end; }
//	const R::Segment_2& CGALSegment() const { return _segment; }
//private:
//	friend class CGeoLineSegmentIntersection;
//	friend class CGeoSegmentSegmentIntersection;
//	CGeoPoint _start;
//	CGeoPoint _end;
//	bool _compareX;
//	R::Segment_2 _segment;
//};

class CGeoSegment: public CGeoLine {
public:
	CGeoSegment() {}
	CGeoSegment(const CGeoPoint& p1, const CGeoPoint& p2) : CGeoLine(p1, p2), _start(p1), _end(p2) {
		_compareX = std::abs(p1.x() - p2.x()) > std::abs(p1.y() - p2.y());
		_center=CGeoPoint((p1.x()+p2.x())/2,(p1.y()+p2.y())/2);
	}
	bool IsPointOnLineOnSegment(const CGeoPoint& p) const // 直线上的点是否在线段上
	{
		if(_compareX){
			return p.x() > (std::min)(_start.x(), _end.x()) && p.x() < (std::max)(_start.x(), _end.x());
		}
		return p.y() > (std::min)(_start.y(), _end.y()) && p.y() < (std::max)(_start.y(), _end.y());
	}
	bool IsSegmentsIntersect(const CGeoSegment& p) const
	{
		CVector AC((start().x() - p.start().x()), (start().y() - p.start().y()));
		CVector AD((start().x() - p.end().x()), (start().y() - p.end().y()));
		CVector BC((end().x() - p.start().x()), (end().y() - p.start().y()));
		CVector BD((end().x() - p.end().x()), (end().y() - p.end().y()));
		/*std::cout << start() << end() <<"\t"<< p.start()<<p.end()<<"\n"<<
			AC<<" "<<AD<<" "<< BC << " " << BD << "\n"
			<< (AC * AD) <<" "<< (BC * BD) << "\t" << (AC * BC) <<" "<< (AD * BD);*/
		return (((AC * AD) * (BC * BD) <= 0) && ((AC * BC) * (AD * BD) <= 0));
	}
	const CGeoPoint& start() const { return _start; }
	const CGeoPoint& end() const { return _end; }
	const CGeoPoint& center() {return _center;}

private:
	CGeoPoint _start;
	CGeoPoint _end;
	CGeoPoint _center;
	bool _compareX;
};

/************************************************************************/
/*                        CGeoShape                                     */
/************************************************************************/
class CGeoShape{
public:
	virtual ~CGeoShape() { }
	virtual bool HasPoint( const CGeoPoint& p) const = 0;
};
/************************************************************************/
/*                        CGeoRectangle                                 */
/************************************************************************/
class CGeoRectangle : public CGeoShape{
public:
	CGeoRectangle( const CGeoPoint& leftTop, const CGeoPoint& rightDown) { 
		calPoint(leftTop.x(), leftTop.y(), rightDown.x(), rightDown.y()); 
	}
	CGeoRectangle( double x1,double y1,double x2,double y2){ calPoint(x1, y1, x2, y2); }
	void calPoint(double x1, double y1, double x2, double y2){
		_point[0] = CGeoPoint(x1, y1);
		_point[1] = CGeoPoint(x1, y2);
		_point[2] = CGeoPoint(x2, y2);
		_point[3] = CGeoPoint(x2, y1);
	}
	virtual bool HasPoint(const CGeoPoint& p) const;
	CGeoPoint _point[4];
};

/************************************************************************/
/*                        CGeoLineRectangleIntersection                 */
/************************************************************************/
class CGeoLineRectangleIntersection{
public:
	CGeoLineRectangleIntersection(const CGeoLine& line, const CGeoRectangle& rect);
	bool intersectant() const { return _intersectant; }
	const CGeoPoint& point1() const { return _point[0]; }
	const CGeoPoint& point2() const { return _point[1]; }
private:
	bool _intersectant;
	CGeoPoint _point[2];
};

/************************************************************************/
/*                        CGeoCircle                                    */
/************************************************************************/
class CGeoCirlce : public CGeoShape{
public:
	CGeoCirlce(){ }
	CGeoCirlce(const CGeoPoint& c,double r) : _center(c), _radius(r){ }
	virtual bool HasPoint(const CGeoPoint& p) const ;
	CGeoPoint Center() const { return _center; }
	double Radius() const { return _radius; }
	double Radius2() const { return _radius*_radius; }
private:
	double _radius;
	CGeoPoint _center;
};

/************************************************************************/
/*                      CGeoLineCircleIntersection                      */
/************************************************************************/
class CGeoLineCircleIntersection{
public:
	CGeoLineCircleIntersection(const CGeoLine& line,const CGeoCirlce& circle);
	bool intersectant() const { return _intersectant;	}
	const CGeoPoint& point1() const { return _point1; }
	const CGeoPoint& point2() const { return _point2; }
private:
	bool _intersectant;
	CGeoPoint _point1;
	CGeoPoint _point2;
};

/************************************************************************/
/*                        CGeoEllipse,此椭圆的轴与坐标轴垂直  ,方程为(x-c.x())^2/m^2+(y-c.y())^2/n^2 =1                                 */
/************************************************************************/
class CGeoEllipse:CGeoShape{
public:
	CGeoEllipse(){ }
	CGeoEllipse(CGeoPoint c,double m,double n) : _center(c), _xaxis(m), _yaxis(n){ }
	CGeoPoint Center() const { return _center; }
	virtual bool HasPoint(const CGeoPoint& p) const ;
	double Xaxis()const{return _xaxis;}
	double Yaxis()const{return _yaxis;}
private:
	double _xaxis;
	double _yaxis;
	CGeoPoint _center;
};

/************************************************************************/
/*                      CGeoLineCircleIntersec

tion                      */
/************************************************************************/
class CGeoLineEllipseIntersection{
public:
	CGeoLineEllipseIntersection(const CGeoLine& line,const CGeoEllipse& circle);
	bool intersectant() const { return _intersectant;	}
	const CGeoPoint& point1() const { return _point1; }
	const CGeoPoint& point2() const { return _point2; }
private:
	bool _intersectant;
	CGeoPoint _point1;
	CGeoPoint _point2;
};

/*********************************************************************/
/*                     CGeoSegmentCircleIntersection                 */
/********************************************************************/
class CGeoSegmentCircleIntersection
{
public:
	CGeoSegmentCircleIntersection(const CGeoSegment& line,const CGeoCirlce& circle);
	bool intersectant() const { return _intersectant;	}
	const CGeoPoint& point1() const { return _point1; }
	const CGeoPoint& point2() const { return _point2; }
	int size() {return intersection_size;}
private:
	bool _intersectant;
	int intersection_size;
	CGeoPoint _point1;
	CGeoPoint _point2;
};
#endif