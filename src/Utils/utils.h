/**
* @file utils.h
* 此文件为一些工具函数的声明.
* @date $Date: 2004/05/04 04:11:49 $
* @version $Revision: 1.20 $
* @author peter@mail.ustc.edu.cn
*/
#ifndef _UTILS_H_
#define _UTILS_H_
#include <geometry.h>
#include <param.h>

#define EPSILON (1.0E-10)

//#define STD_DEBUG_ENABLE

#ifdef STD_DEBUG_ENABLE
	#define STD_DEBUG_OUT(header,content) \
	std::cout << header << " : " << content << std::endl;
#else
	#define STD_DEBUG_OUT(header,content) 
#endif
class CVisionModule;
struct PlayerPoseT;
namespace Utils{
	extern double Normalize(double angle);///<把角度规范化到(-PI,PI]
	extern CVector Polar2Vector(double m,double angle);///<极坐标转换到直角坐标
	extern double VectorDot(const CVector& v1, const CVector& v2);  // 向量点乘
	extern double dirDiff(const CVector& v1, const CVector& v2) { return fabs(Normalize(v1.dir() - v2.dir()));} // 2014/2/28 新增 计算两个向量的夹角 yys
	extern bool InBetween(const CGeoPoint& p,const CGeoPoint& p1,const CGeoPoint& p2);// 判断p是否在p1,p2之间
	extern bool InBetween(double v,double v1,double v2);// 判断v是否在v1和v2之间
	// 三个均为向量的方向弧度, 判断是否满足v的方向夹在v1和v2之间
	// 如果v和v1或v2中的任意一个夹角小于buffer, 则也认为满足条件.
	extern bool AngleBetween(double d, double d1, double d2, double buffer = Param::Math::PI/30); 
	inline CGeoPoint CenterOfTwoPoint(const CGeoPoint& p1,const CGeoPoint& p2){return CGeoPoint((p1.x()+p2.x())/2,(p1.y()+p2.y())/2);}
	// 判断三个共起点向量, v的方向是否夹在v1和v2之间, 
	// buffer表示余量, 表示当v不在v1,v2之间时, 
	// 如果v和v1或v2中的任意一个夹角小于buffer, 则也认为满足条件.
	extern bool InBetween(const CVector& v, const CVector& v1, const CVector& v2, double buffer = Param::Math::PI/30);
	/*@brief	判断一个浮点数是否在其余两个浮点数之间*/
	inline   bool CBetween(float v,float v1,float v2){ return (v > v1 && v < v2) || (v < v1 && v > v2); }
	/*@brief	计算点到直线的距离*/
	inline double pointToLineDist(const CGeoPoint& p, const CGeoLine& l) { return p.dist(l.projection(p));}
	inline double Deg2Rad(double angle){ return angle * Param::Math::PI / 180; }
	inline double Rad2Deg(double angle){ return angle * 180 / Param::Math::PI; }
	extern CGeoPoint MakeInField(const CGeoPoint& p,const double buffer = 0);///<让点在场内
	extern bool OutOfField(const CGeoPoint& p,const double buffer = 0); ///<判断点是否在场内
	inline double FieldLeft(){ return -Param::Field::PITCH_LENGTH/2 + Param::Field::PITCH_MARGIN; }
	inline double FieldRight(){ return Param::Field::PITCH_LENGTH/2 - Param::Field::PITCH_MARGIN; }
	inline double FieldTop(){ return -Param::Field::PITCH_WIDTH/2 + Param::Field::PITCH_MARGIN; }
	inline double FieldBottom(){ return Param::Field::PITCH_WIDTH/2 - Param::Field::PITCH_MARGIN; }
	inline CGeoPoint LeftTop(){ return CGeoPoint(FieldLeft(),FieldTop()); }
	inline CGeoPoint RightBottom(){ return CGeoPoint(FieldRight(),FieldBottom()); }
	inline int Sign(double d){ return (d >= 0) ? 1 : -1; }
	extern CGeoPoint MakeOutOfOurPenaltyArea(const CGeoPoint& p,const double buffer);
	extern CGeoPoint MakeOutOfTheirPenaltyArea(const CGeoPoint& p, const double buffer);
	extern CGeoPoint MakeOutOfCircleAndInField(const CGeoPoint& center,const double radius,const CGeoPoint& p,const double buffer); // 确保点在圆外
	
	extern bool InOurPenaltyArea(const CGeoPoint& p,const double buffer);
	extern bool InTheirPenaltyArea(const CGeoPoint& p,const double buffer);
	extern bool PlayerNumValid(int num);
	extern CGeoPoint GetOutSidePenaltyPos(double dir, double delta, const CGeoPoint targetPoint = CGeoPoint(-(Param::Field::PITCH_LENGTH)/2, 0));
	extern CGeoPoint GetOutTheirSidePenaltyPos(double dir, double delta, const CGeoPoint& targetPoint);
	extern CGeoPoint GetInterPos(double dir, const CGeoPoint targetPoint = CGeoPoint(-(Param::Field::PITCH_LENGTH)/2, 0));
	extern CGeoPoint GetTheirInterPos(double dir, const CGeoPoint& targetPoint);
	extern float SquareRootFloat(float number); 
	extern bool canGo(const CVisionModule* pVision, const int num, const CGeoPoint& target, const int flag, const double avoidBuffer);
}
#endif
