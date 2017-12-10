/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team：		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  cuGeometry.h												*/
/* Func:	  定义了CUDA预算中的几何运算								*/
/* Author:	  王群 2012-08-18											*/
/* Refer:	  ###														*/
/* E-mail:	  wangqun1234@zju.edu.cn									*/
/* Version:	  0.0.1														*/
/************************************************************************/
/**
@note 机器人视觉valid设定：发来数据若为(1000,1000),则valid设置为false*/

#ifndef _CU_GEOMETRY_H_
#define _CU_GEOMETRY_H_

#include <stdio.h>
#include <math.h>

//常量
#define FLOAT_EPS 0.001
#define PI 3.1415926

//常用函数

/** 
@brief	判断两个浮点数是否相等*/
inline   bool cuEqual(const float x,const float y){ 
	return fabs(x-y) < FLOAT_EPS;
}
/** 
@brief	将角度的弧度值限制在-PI到PI之内*/
inline   float cuNormalize(float angle)
{
	float M_2PI = PI * 2;
	if( fabs(angle) > M_2PI ){
		angle = angle - floor(angle / M_2PI) * M_2PI;
	}
	while( angle > PI ){
		angle -= 2*PI;
	}
	while( angle <= -PI ){
		angle += 2*PI;
	}
	return angle;
}
/** 
@brief	判断一个浮点数是否在其余两个浮点数之间*/
inline   bool cuBetween(float v,float v1,float v2){ return (v > v1 && v < v2) || (v < v1 && v > v2); }
typedef struct  
{
	float x,y;
}float2;
typedef float2 cuPoint;
typedef float2 cuVector;
inline float2 make_float2(float x, float y)
{
	float2 t; t.x = x; t.y = y; return t;
}
inline float dot(float2 a, float2 b)
{ 
	return a.x * b.x + a.y * b.y;
}
inline float length(float2 v)
{
	return sqrtf(dot(v, v));
}
inline float2 operator-(float2 b, float2 a)
{
	return make_float2(b.x - a.x, b.y - a.y);
}
inline float2 operator+(float2 b, float2 a)
{
	return make_float2(b.x + a.x, b.y + a.y);
}
/************************************************************************/
/* cuPoint                                                              */
/************************************************************************/
/** 
@brief	生成一个点*/
inline   cuPoint cuMakePoint(float x,float y){return make_float2(x,y);}
/** 
@brief	获得一个点的x坐标*/
inline   float cuGetPointX(const cuPoint& p){return p.x;}
/** 
@brief	获得一个点的y坐标*/
inline   float cuGetPointY(const cuPoint& p){return p.y;}
/** 
@brief	求两个点之间的距离*/
inline   float cuDist(const cuPoint& p,const cuPoint& q){return length(p-q);}
/** 
@brief	求两个点之间的距离的平方*/
inline   float cuDist2(const cuPoint& p,const cuPoint& q){return cuDist(p,q)*cuDist(p,q);}

/************************************************************************/
/* cuVector                                                             */
/************************************************************************/
/** 
@brief	生成一个二维的向量，输入参数是该向量的直角坐标系中的x和y坐标*/
inline   cuVector cuMakeVector(float x,float y){return make_float2(x,y);}
/** 
@brief	生成一个二维的向量，输入参数是向量的模和极坐标中的方向角度*/
inline   cuVector cuMakeVector2(float m,float angle){return cuMakeVector(m*cos(angle),m*sin(angle));}
/** 
@brief	求向量相对于直角坐标系中y轴的对称向量*/
inline   cuVector cuPerp(cuVector& v){return make_float2(v.y,-v.x);}
/** 
@brief	求向量在直角坐标系中的x值*/
inline   float cuGetVectorX(const cuVector v){return v.x;}
/** 
@brief	求向量在直角坐标系中的y值*/
inline   float cuGetVectorY(const cuVector v){return v.y;}
/** 
@brief	求向量的模*/
inline   float cuMod(const cuVector& v){return length(v);}
/** 
@brief	求两个向量的内积*/
inline   float cuDot(const cuVector& a,const cuVector& b){return dot(a,b);}
/** 
@brief	求向量在极坐标系中的方向*/
inline   float cuDir(const cuVector& v)
{
	if( cuEqual(v.x,0) && cuEqual(v.y,0) ){
		return 0.0;
	}
	return atan2f(v.y,v.x);
}
/** 
@brief	求两个向量的夹角(小于180度的角)*/
inline   float cuAngleDiff(const cuVector& v1, const cuVector& v2){return fabs(cuNormalize(cuDir(v1) - cuDir(v2)));}

/************************************************************************/
/* cuLine                                                               */
/************************************************************************/
typedef struct {
	cuPoint p1;
	cuPoint p2;
}cuLine;
/** 
@brief	生成一条直线，输入参数是两个点*/
inline   cuLine cuMakeLine(const cuPoint& p1,const cuPoint& p2){cuLine l; l.p1 = p1; l.p2 = p2; return l;}
/** 
@brief	生成一条直线，输入参数是一个点和一个方向角度*/
inline   cuLine cuMakeLine2(const cuPoint& p,const float angle){return cuMakeLine(p,cuMakePoint(p.x + cosf(angle),p.y + sinf(angle)));}
/** 
@brief	求点到直线的垂点*/
inline   cuPoint cuProjection(const cuLine& l,const cuPoint& p){
	float longdist = cuDist(l.p1,p);
	float angleDiff = cuAngleDiff(p-l.p1,l.p2-l.p1);
	float dist = longdist*fabs(cosf(angleDiff));
	if (angleDiff < PI/2.0){
		return l.p1 + cuMakeVector2(dist,cuDir(l.p2-l.p1));
	} else return l.p1 + cuMakeVector2(dist,cuDir(l.p1-l.p2));
}
/** 
@brief	求点到直线的距离*/
inline   float cuPoint2LineDist(const cuLine& l,const cuPoint& p){return cuDist(p,cuProjection(l,p));}

/************************************************************************/
/* 队员定位函数                                    */
/************************************************************************/
typedef float2 player;
/** 
@brief	判断一名队员是否存在*/
inline   bool cuPlayerValid(const player p)
{
	cuPoint centrePoint = cuMakePoint(0,0);
    float dist2centrePoint = cuDist(p,centrePoint);
	if (dist2centrePoint > 1000)
	{
		return false;
	} else return true;
}


#endif