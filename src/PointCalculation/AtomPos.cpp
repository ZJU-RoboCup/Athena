/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team：		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  AtomPos.cpp												*/
/* Func:	  计算一个策略点											*/
/* Author:	  王群 2012-08-18											*/
/* Refer:	  ###														*/
/* E-mail:	  wangqun1234@zju.edu.cn									*/
/************************************************************************/

#include "AtomPos.h"
#include "utils.h"
#include "GDebugEngine.h"
#include "WorldModel.h"

namespace {
	const bool VERBOSE_MODE = false;
}
//虚函数的缺省实现
CAtomPos::CAtomPos():_pos(Pos::invalidPos) 
{}

CAtomPos::~CAtomPos()
{}

void CAtomPos::setPos(const CGeoPoint& pos)
{
	_pos = pos;
}

void CAtomPos::clearPos()
{
	_pos = Pos::invalidPos;
}

CGeoPoint CAtomPos::getPos()
{
	return _pos;	
}

bool CAtomPos::isAvailable()
{
	return !(Utils::OutOfField(_pos,0));
}

void CAtomPos::printDebug()
{
	if (this->isAvailable())	
		GDebugEngine::Instance()->gui_debug_x(_pos,COLOR_ORANGE);
	else 
		GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(280,120),"Pos invalid!",COLOR_RED);
}