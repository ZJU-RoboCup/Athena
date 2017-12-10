/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team：		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  DefPos1G2D.cpp											*/
/* Func:	  1个门将和2个后卫联合封门									*/
/* Author:	  王群 2012-08-18											*/
/* Refer:	  ###														*/
/* E-mail:	  wangqun1234@zju.edu.cn									*/
/************************************************************************/

#include "DefPos1G2D.h"
#include "param.h"
#include "utils.h"
#include "WorldModel.h"
#include "BestPlayer.h"
#include "GDebugEngine.h"
#include <math.h>
#include "BallSpeedModel.h"

CDefPos1G2D::CDefPos1G2D()
{
	
}

CDefPos1G2D::~CDefPos1G2D()
{

}
 
CGeoPoint CDefPos1G2D::generatePos(const CVisionModule* pVision)
{
	return CGeoPoint(0,0);
}