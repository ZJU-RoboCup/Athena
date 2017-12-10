/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team��		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  cuGeometry.h												*/
/* Func:	  ������CUDAԤ���еļ�������								*/
/* Author:	  ��Ⱥ 2012-08-18											*/
/* Refer:	  ###														*/
/* E-mail:	  wangqun1234@zju.edu.cn									*/
/* Version:	  0.0.1														*/
/************************************************************************/
/**
@note �������Ӿ�valid�趨������������Ϊ(1000,1000),��valid����Ϊfalse*/

#ifndef _CU_GEOMETRY_H_
#define _CU_GEOMETRY_H_

#include <stdio.h>
#include <math.h>

//����
#define FLOAT_EPS 0.001
#define PI 3.1415926

//���ú���

/** 
@brief	�ж������������Ƿ����*/
inline   bool cuEqual(const float x,const float y){ 
	return fabs(x-y) < FLOAT_EPS;
}
/** 
@brief	���ǶȵĻ���ֵ������-PI��PI֮��*/
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
@brief	�ж�һ���������Ƿ�����������������֮��*/
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
@brief	����һ����*/
inline   cuPoint cuMakePoint(float x,float y){return make_float2(x,y);}
/** 
@brief	���һ�����x����*/
inline   float cuGetPointX(const cuPoint& p){return p.x;}
/** 
@brief	���һ�����y����*/
inline   float cuGetPointY(const cuPoint& p){return p.y;}
/** 
@brief	��������֮��ľ���*/
inline   float cuDist(const cuPoint& p,const cuPoint& q){return length(p-q);}
/** 
@brief	��������֮��ľ����ƽ��*/
inline   float cuDist2(const cuPoint& p,const cuPoint& q){return cuDist(p,q)*cuDist(p,q);}

/************************************************************************/
/* cuVector                                                             */
/************************************************************************/
/** 
@brief	����һ����ά����������������Ǹ�������ֱ������ϵ�е�x��y����*/
inline   cuVector cuMakeVector(float x,float y){return make_float2(x,y);}
/** 
@brief	����һ����ά�����������������������ģ�ͼ������еķ���Ƕ�*/
inline   cuVector cuMakeVector2(float m,float angle){return cuMakeVector(m*cos(angle),m*sin(angle));}
/** 
@brief	�����������ֱ������ϵ��y��ĶԳ�����*/
inline   cuVector cuPerp(cuVector& v){return make_float2(v.y,-v.x);}
/** 
@brief	��������ֱ������ϵ�е�xֵ*/
inline   float cuGetVectorX(const cuVector v){return v.x;}
/** 
@brief	��������ֱ������ϵ�е�yֵ*/
inline   float cuGetVectorY(const cuVector v){return v.y;}
/** 
@brief	��������ģ*/
inline   float cuMod(const cuVector& v){return length(v);}
/** 
@brief	�������������ڻ�*/
inline   float cuDot(const cuVector& a,const cuVector& b){return dot(a,b);}
/** 
@brief	�������ڼ�����ϵ�еķ���*/
inline   float cuDir(const cuVector& v)
{
	if( cuEqual(v.x,0) && cuEqual(v.y,0) ){
		return 0.0;
	}
	return atan2f(v.y,v.x);
}
/** 
@brief	�����������ļн�(С��180�ȵĽ�)*/
inline   float cuAngleDiff(const cuVector& v1, const cuVector& v2){return fabs(cuNormalize(cuDir(v1) - cuDir(v2)));}

/************************************************************************/
/* cuLine                                                               */
/************************************************************************/
typedef struct {
	cuPoint p1;
	cuPoint p2;
}cuLine;
/** 
@brief	����һ��ֱ�ߣ����������������*/
inline   cuLine cuMakeLine(const cuPoint& p1,const cuPoint& p2){cuLine l; l.p1 = p1; l.p2 = p2; return l;}
/** 
@brief	����һ��ֱ�ߣ����������һ�����һ������Ƕ�*/
inline   cuLine cuMakeLine2(const cuPoint& p,const float angle){return cuMakeLine(p,cuMakePoint(p.x + cosf(angle),p.y + sinf(angle)));}
/** 
@brief	��㵽ֱ�ߵĴ���*/
inline   cuPoint cuProjection(const cuLine& l,const cuPoint& p){
	float longdist = cuDist(l.p1,p);
	float angleDiff = cuAngleDiff(p-l.p1,l.p2-l.p1);
	float dist = longdist*fabs(cosf(angleDiff));
	if (angleDiff < PI/2.0){
		return l.p1 + cuMakeVector2(dist,cuDir(l.p2-l.p1));
	} else return l.p1 + cuMakeVector2(dist,cuDir(l.p1-l.p2));
}
/** 
@brief	��㵽ֱ�ߵľ���*/
inline   float cuPoint2LineDist(const cuLine& l,const cuPoint& p){return cuDist(p,cuProjection(l,p));}

/************************************************************************/
/* ��Ա��λ����                                    */
/************************************************************************/
typedef float2 player;
/** 
@brief	�ж�һ����Ա�Ƿ����*/
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