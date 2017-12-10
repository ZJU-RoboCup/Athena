/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team：		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  IndirectDefender.cpp										*/
/* Func:	  防守对方定位球开球小车的站位点							*/
/* Author:	  王群 2012-08-18											*/
/* Refer:	  ###														*/
/* E-mail:	  wangqun1234@zju.edu.cn									*/
/************************************************************************/

#include "IndirectDefender.h"
#include "param.h"
#include "utils.h"
#include "defence/DefenceInfo.h"
#include "BestPlayer.h"
#include "GDebugEngine.h"
#include "geometry.h"
#include "MarkingPosV2.h"

namespace{
	bool debug = true;
	bool CORNERMODE = false;
	CGeoPoint goalCenter;
	double RADIUS;
	const double FIELD_BUFFER = 10;
	//CGeoPoint leftUp;
	//CGeoPoint leftDown;
	//CGeoPoint rightUp;
	//CGeoPoint rightDown;
}

CIndirectDefender::CIndirectDefender()
{
	clearPos();
	_lastCycle = 0;
	_lastTwoKickCycle = 0;
	minnum=1;
	maxnum=1;
	for (int i=0;i<=Param::Field::MAX_PLAYER+1;i++){
		_targetDir[i]=0;
	}
	RADIUS = 60*Param::Field::RATIO;
	goalCenter = CGeoPoint(-Param::Field::PITCH_LENGTH/2,0);
	_radius = 0.0;
	_leftUp = CGeoPoint(0,0);
	_rightDown = CGeoPoint(0,0);
	
	_twoKick.onePos = CGeoPoint(0,0);
	_twoKick.anotherPos = CGeoPoint(0,0);
	_oneArea._Left = CGeoPoint(0,0);
	_oneArea._Right = CGeoPoint(0,0);
	_anotherArea._Right =CGeoPoint(0,0);
	_anotherArea._Left = CGeoPoint(0,0);
	//leftUp = CGeoPoint(Param::Field::PITCH_LENGTH/2 - FIELD_BUFFER,-Param::Field::PITCH_WIDTH/2 + FIELD_BUFFER);
	//leftDown = CGeoPoint(-Param::Field::PITCH_LENGTH/2 + FIELD_BUFFER,-Param::Field::PITCH_WIDTH/2 + FIELD_BUFFER);
	//rightUp = CGeoPoint(Param::Field::PITCH_LENGTH/2 - FIELD_BUFFER,Param::Field::PITCH_WIDTH/2 - FIELD_BUFFER);
	//rightDown = CGeoPoint(-Param::Field::PITCH_LENGTH/2 + FIELD_BUFFER,Param::Field::PITCH_WIDTH/2 - FIELD_BUFFER);
}

CIndirectDefender::~CIndirectDefender()
{}

CGeoPoint CIndirectDefender::getDefPos(const CVisionModule* pVision, double radius, CGeoPoint leftUp, CGeoPoint rightDown, int mode)
{
	//同一帧内多次调用则只计算一次
	if (pVision->Cycle() == _lastCycle)
	{
		return getPos();
	} else _lastCycle = pVision->Cycle();
	_radius = radius;
	_leftUp = leftUp;
	_rightDown = rightDown;
	_mode = mode;
	//if (_mode == 3){
	//	KickOffArea kickOff = KickOffArea(_leftUp,_rightDown);
	//	vector<KickOffArea>::iterator ir;
	//	bool result = false;
	//	for (ir = kickOffList.begin();ir!=kickOffList.end();ir++){
	//		if (fabs((*ir)._Left.x()-_leftUp.x())<0.1 &&fabs((*ir)._Left.y()-_leftUp.y())<0.1 && fabs((*ir)._Right.x() - _rightDown.x())<0.1
	//			&& fabs((*ir)._Right.y() - _rightDown.y())<0.1){
	//				result = true;
	//				break;
	//		}
	//	}
	//	if (result == false){
	//		kickOffList.push_back(kickOff);
	//	}
	//}
	//cout<<"mode is "<<mode<<endl;
	setPos(generatePos(pVision));
	return getPos();
}

TwoKickPos CIndirectDefender::getTwoDefPos(const CVisionModule* pVision,double radius1,CGeoPoint left1,CGeoPoint right1,double radius2,CGeoPoint left2,CGeoPoint right2)
{

	if (pVision->Cycle() == _lastTwoKickCycle)
	{
		return _twoKick;
	} else _lastTwoKickCycle = pVision->Cycle();
		_oneArea._Left = left1;
		_oneArea._Right = right1;
		_oneArea.radius = radius1;
		_anotherArea._Left = left2;
		_anotherArea._Right = right2;
		_anotherArea.radius = radius2;
		generateTwoDefPos(pVision);
		return _twoKick;
}

void CIndirectDefender::generateTwoDefPos(const CVisionModule* pVision)
{
	double targetDir;
	const CGeoPoint ballPos = pVision->Ball().Pos();
	const int enemyNumber = DefenceInfo::Instance()->getAttackOppNumByPri(0);
	if (enemyNumber == 0){
		_twoKick.anotherPos = ballPos+Utils::Polar2Vector(60,(goalCenter - ballPos).dir());
		_twoKick.onePos = ballPos + Utils::Polar2Vector(100,(goalCenter - ballPos).dir());
	}else{
	const PlayerVisionT& enemy = pVision->TheirPlayer(enemyNumber);
	const double enemy2ballDir = CVector(ballPos - enemy.Pos()).dir();
	double oneDir = 0;
	double anotherDir = 0;
	if (fabs(Utils::Normalize(enemy.Dir() - enemy2ballDir)) < Param::Math::PI / 12.0)
	{
		targetDir = enemy.Dir();
	} else targetDir = enemy2ballDir; 
	double o2leftDir = (_oneArea._Left - ballPos).dir();
	double o2rightDir = (_oneArea._Right - ballPos).dir();
	_oneArea.baseDir =  Utils::Normalize(o2rightDir+fabs(Utils::Normalize(o2leftDir - o2rightDir))/2);
	double a2leftDir = (_anotherArea._Left - ballPos).dir();
	double a2rightDir = (_anotherArea._Right - ballPos).dir();
	_anotherArea.baseDir =  Utils::Normalize(a2rightDir+fabs(Utils::Normalize(a2leftDir - a2rightDir))/2);
	GDebugEngine::Instance()->gui_debug_line(ballPos,ballPos+Utils::Polar2Vector(300,a2leftDir),COLOR_PURPLE);
	GDebugEngine::Instance()->gui_debug_line(ballPos,ballPos+Utils::Polar2Vector(300,a2rightDir),COLOR_PURPLE);
	GDebugEngine::Instance()->gui_debug_line(ballPos,ballPos+Utils::Polar2Vector(300,o2leftDir),COLOR_PURPLE);
	GDebugEngine::Instance()->gui_debug_line(ballPos,ballPos+Utils::Polar2Vector(300,o2rightDir),COLOR_PURPLE);
	GDebugEngine::Instance()->gui_debug_line(ballPos,ballPos+Utils::Polar2Vector(100,targetDir),COLOR_BLACK);
	GDebugEngine::Instance()->gui_debug_line(ballPos,ballPos+Utils::Polar2Vector(200,_anotherArea.baseDir),COLOR_YELLOW);
	if (ballPos.y()>0){
		if (Utils::AngleBetween(targetDir,a2rightDir,_anotherArea.baseDir,0)){
			anotherDir = targetDir;
			oneDir = o2rightDir;
		}else if(Utils::AngleBetween(targetDir,_anotherArea.baseDir,a2leftDir,0)){
			anotherDir = targetDir;
			oneDir = Utils::Normalize(targetDir + Utils::Normalize(3*fabs(asin(Param::Vehicle::V2::PLAYER_SIZE/60))));
		}else if (Utils::AngleBetween(targetDir,o2leftDir,o2rightDir,0)){
			oneDir = targetDir;
			anotherDir = (goalCenter - ballPos).dir();
		}else{
			anotherDir = (goalCenter - ballPos).dir();
			oneDir = _oneArea.baseDir;
		}
	}else{
		if (Utils::AngleBetween(targetDir,a2leftDir,_anotherArea.baseDir,0)){
			anotherDir = targetDir;
			oneDir = o2leftDir;
		}else if(Utils::AngleBetween(targetDir,_anotherArea.baseDir,a2rightDir,0)){
			anotherDir = targetDir;
			oneDir = Utils::Normalize(targetDir - Utils::Normalize(3*fabs(asin(Param::Vehicle::V2::PLAYER_SIZE/60))));
		}else if (Utils::AngleBetween(targetDir,o2leftDir,o2rightDir,0)){
			oneDir = targetDir;
			anotherDir = (goalCenter - ballPos).dir();
		}else{
			anotherDir = (goalCenter - ballPos).dir();
			oneDir = _oneArea.baseDir;
		}

	}
	_twoKick.onePos = DefendPos(ballPos,oneDir,_oneArea.radius);
	_twoKick.anotherPos = DefendPos(ballPos,anotherDir,_anotherArea.radius);
	}
}

CGeoPoint CIndirectDefender::generatePos(const CVisionModule* pVision)
{
	CGeoPoint defendPos;
	double targetDir;
	const CGeoPoint ballPos = pVision->Ball().Pos();
	//const int enemyNumber = BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num;
	const int enemyNumber = DefenceInfo::Instance()->getAttackOppNumByPri(0);
	if (enemyNumber == 0){
		return ballPos+Utils::Polar2Vector(60,(goalCenter - ballPos).dir());
	}
	const PlayerVisionT& enemy = pVision->TheirPlayer(enemyNumber);
	 const double enemy2ballDir = CVector(ballPos - enemy.Pos()).dir();
	 //cout<<"in indirect defend!!!"<<endl;
	if (fabs(Utils::Normalize(enemy.Dir() - enemy2ballDir)) < Param::Math::PI / 12.0)
	{
		targetDir = enemy.Dir();
	} else targetDir = enemy2ballDir; 
	//targetDir = enemy2ballDir; 
	if (debug){
		//GDebugEngine::Instance()->gui_debug_x(defendPos,COLOR_RED);
	}
	//防敌方车转很多圈再开球
	if (_mode == 1)
	{
		MarkField checkField = MarkField(_leftUp,_rightDown);
		int attacknum = DefenceInfo::Instance()->getAttackNum();
		int enemeynum =1;
		int count = 1;
		if (attacknum>=2){
			while (count<attacknum){
				int tempNum = DefenceInfo::Instance()->getAttackOppNumByPri(count);
				CGeoPoint _tempEnemyPos = pVision->TheirPlayer(tempNum).Pos();
				if (_tempEnemyPos.dist(ballPos)>60*Param::Field::RATIO && false == MarkingPosV2::Instance()->isInSpecialAreaBackLineMode(pVision,tempNum)
					&& DefenceInfo::Instance()->checkInRecArea(tempNum,pVision,checkField)){
					_targetDir[enemeynum]=(_tempEnemyPos-ballPos).dir();
					enemeynum++;
				}
				count++;
			}
			if (CORNERMODE){
				if (false){
					_targetDir[enemeynum]=(goalCenter-ballPos).dir();
				}else{
					enemeynum-- ;
				}
			}else{
				_targetDir[enemeynum]=(goalCenter-ballPos).dir();
			}
			FindMinMax(enemeynum);
			if (ballPos.y()<0){
				if (_targetDir[minnum]<(-1)*Param::Math::PI/2){
					_targetDir[minnum]=Param::Math::PI;
				}
			}else{
				//cout<<_targetDir[maxnum]<<"  "<<(-1)*Param::Math::PI/2<<endl;
				if (_targetDir[maxnum]>Param::Math::PI/2){
					_targetDir[maxnum]=(-1)*Param::Math::PI;
				}
			}
			FindMinMax(enemeynum);
			if (!(targetDir>=_targetDir[minnum] && targetDir<=_targetDir[maxnum])){
				double totalAngleDiff =fabs(Utils::Normalize(_targetDir[maxnum]-_targetDir[minnum]))/2;
				double toMaxDiff =fabs(Utils::Normalize(targetDir-_targetDir[maxnum]));
				double toMinDiff = fabs(Utils::Normalize(_targetDir[minnum]-targetDir));
				if (toMaxDiff<toMinDiff){
					double ratio = toMaxDiff/(Param::Math::PI - totalAngleDiff);
					targetDir = Utils::Normalize(_targetDir[maxnum]-ratio*totalAngleDiff);
				}else{
					double ratio = toMinDiff/(Param::Math::PI - totalAngleDiff);
					targetDir = Utils::Normalize(_targetDir[minnum]+ratio*totalAngleDiff);
				}
				//cout<<"  "<<targetDir<<endl;
			}
		}else{
			targetDir = (goalCenter -ballPos).dir();
		}
		//cout<<pVision->Cycle()<<"  "<<Utils::AngleBetween(targetDir,_targetDir[minnum],_targetDir[maxnum],0)<<"  "<<targetDir<<endl;
		defendPos = DefendPos(ballPos,targetDir,_radius);
		const string refMsg = pVision->GetCurrentRefereeMsg();
		//////////////////////////////////////////////////////////////////////////
		//先跑这个
		if (Utils::OutOfField(defendPos,FIELD_BUFFER) || enemy.Pos().dist(ballPos) > Param::Field::FREE_KICK_AVOID_BALL_DIST) {
			const CVector elseVector = Utils::Polar2Vector(_radius,CVector(goalCenter - ballPos).dir());
			defendPos = ballPos + elseVector;
			//GDebugEngine::Instance()->gui_debug_line(ballPos,defendPos,COLOR_RED);
		}
	}else if (_mode ==2)
	{
		//GDebugEngine::Instance()->gui_debug_line(ballPos,_leftUp,COLOR_ORANGE);
		//GDebugEngine::Instance()->gui_debug_line(ballPos,_rightDown,COLOR_ORANGE);
		//GDebugEngine::Instance()->gui_debug_line(ballPos,ballPos+Utils::Polar2Vector(200,targetDir),COLOR_BLACK);
		double _target2LeftDir = (_leftUp - pVision->Ball().Pos()).dir();
		double _target2RightDir = (_rightDown - pVision->Ball().Pos()).dir();
		//cout<<_target2LeftDir*180.0/Param::Math::PI<<" "<<_target2RightDir*180.0/Param::Math::PI<<endl;
		if (!Utils::AngleBetween(targetDir,_target2RightDir,_target2LeftDir,0)){
			double totalAngleDiff =fabs(Utils::Normalize(_target2LeftDir-_target2RightDir))/2;
			double toMaxDiff =fabs(Utils::Normalize(targetDir-_target2RightDir));
			if(toMaxDiff > Param::Math::PI)
			{
				//toMaxDiff = 2*Param::Math::PI - toMaxDiff;
			}
			double toMinDiff = fabs(Utils::Normalize(_target2LeftDir-targetDir));
			if (toMinDiff > Param::Math::PI)
			{
				//toMinDiff = 2*Param::Math::PI - toMinDiff;
			}
			//cout<<_target2LeftDir*180.0/Param::Math::PI<<" "<<_target2RightDir*180.0/Param::Math::PI<<" "<<toMaxDiff*180/Param::Math::PI<<" "<<toMinDiff*180/Param::Math::PI<<" "<<totalAngleDiff*180/Param::Math::PI<<endl;
			if (toMaxDiff<toMinDiff){
				double ratio = toMaxDiff/(Param::Math::PI - totalAngleDiff);
				targetDir = Utils::Normalize(_target2RightDir+ratio*totalAngleDiff);
			}else{
				double ratio = toMinDiff/(Param::Math::PI - totalAngleDiff);
				targetDir = Utils::Normalize(_target2LeftDir-ratio*totalAngleDiff);
			}
		}
		defendPos = DefendPos(ballPos,targetDir,_radius);
		if (Utils::OutOfField(defendPos,FIELD_BUFFER) || enemy.Pos().dist(ballPos) > Param::Field::FREE_KICK_AVOID_BALL_DIST)
		{
			const CVector elseVector = Utils::Polar2Vector(_radius,CVector(goalCenter - ballPos).dir());
			defendPos = ballPos + elseVector;
		}
	}
	//	}else if (_mode == 3){
	//		double _target2LeftDir = (_leftUp - pVision->Ball().Pos()).dir();
	//		double _target2RightDir = (_rightDown - pVision->Ball().Pos()).dir();
	//		//边界的处理
	//		bool result;
	//		vector<KickOffArea>::iterator ir;
	//		//std::sort(kickOffList.begin(),kickOffList.end());
	//		for (ir = kickOffList.begin();ir!=kickOffList.end();ir++){
	//			if (fabs((*ir)._Left.x()-_leftUp.x())<0.1 &&fabs((*ir)._Left.y()-_leftUp.y())<0.1 && fabs((*ir)._Right.x() - _rightDown.x())<0.1
	//				&& fabs((*ir)._Right.y() - _rightDown.y())<0.1){
	//					result = (*ir)._inArea;
	//					break;
	//			}
	//		}
	//		if (result == false){
	//			if (Utils::AngleBetween(targetDir,_target2RightDir,_target2LeftDir,0)){
	//				result = true;
	//			}
	//		}else if (result == true){
	//			if (!Utils::AngleBetween(targetDir,_target2RightDir,_target2LeftDir,0) && min(fabs(Utils::Normalize(targetDir - _target2LeftDir)),fabs(Utils::Normalize(targetDir - _target2RightDir)))>0.2){
	//				result = false;
	//			}
	//		}
	//		if (result == true && ir!=kickOffList.begin()){
	//			for(vector<KickOffArea>::iterator pr = kickOffList.begin();pr!=ir;pr++){
	//				if ((*pr)._inArea == true && pr!=ir){
	//					result = false;
	//					break;
	//				}
	//			}
	//		}
	//		(*ir)._inArea = result;
	//		for (vector<KickOffArea>::iterator pr = kickOffList.begin();pr!=kickOffList.end();pr++){
	//			cout<<(*pr)._Left.x()<<" "<<(*pr)._inArea<<endl;
	//		}
	//		if (result == false){
	//			targetDir = Utils::Normalize(_target2RightDir+fabs(Utils::Normalize(_target2LeftDir - _target2RightDir))/2);
	//		}else if (result == true){
	//			if (!Utils::AngleBetween(targetDir,_target2RightDir,_target2LeftDir,0)){
	//				if (fabs(Utils::Normalize(targetDir - _target2LeftDir))>fabs(Utils::Normalize(targetDir - _target2RightDir))){
	//					targetDir = _target2RightDir;
	//				}else{
	//					targetDir = _target2LeftDir;
	//				}
	//			}
	//		}
	//		GDebugEngine::Instance()->gui_debug_line(ballPos,_leftUp,COLOR_ORANGE);
	//		GDebugEngine::Instance()->gui_debug_line(ballPos,_rightDown,COLOR_ORANGE);
	//	}
	//	//GDebugEngine::Instance()->gui_debug_line(ballPos,ballPos+Utils::Polar2Vector(300,targetDir),COLOR_PURPLE);
	//	defendPos = DefendPos(ballPos,targetDir,_radius);
	//	if (Utils::OutOfField(defendPos,FIELD_BUFFER) || (enemy.Pos().dist(ballPos) > Param::Field::FREE_KICK_AVOID_BALL_DIST && _mode!=3))
	//	{
	//		const CVector elseVector = Utils::Polar2Vector(_radius,CVector(goalCenter - ballPos).dir());
	//		defendPos = ballPos + elseVector;
	//		//cout<<"33333333"<<endl;
	//	}
	//}
	//////////////////////////////////////////////////////////////////////////
	//if (Utils::OutOfField(defendPos,FIELD_BUFFER))
	//{
	//	CGeoPoint points[8];
	//	int pointCount = 0;
	//	const CGeoCirlce standardCircle = CGeoCirlce(ballPos,RADIUS);
	//	CGeoLine standardLine = CGeoLine(leftUp,leftDown);
	//	CGeoLineCircleIntersection intersect = CGeoLineCircleIntersection(standardLine,standardCircle);
	//	if (intersect.intersectant())
	//	{
	//		if (!Utils::OutOfField(intersect.point1(),FIELD_BUFFER - 2 ))
	//		{
	//			points[pointCount] = intersect.point1();
	//			pointCount++;
	//		}
	//		if (!Utils::OutOfField(intersect.point2(),FIELD_BUFFER - 2 ))
	//		{
	//			points[pointCount] = intersect.point2();
	//			pointCount++;
	//		}
	//	}
	//	standardLine = CGeoLine(leftUp,rightUp);
	//	intersect = CGeoLineCircleIntersection(standardLine,standardCircle);
	//	if (intersect.intersectant())
	//	{
	//		if (!Utils::OutOfField(intersect.point1(),FIELD_BUFFER - 2 ))
	//		{
	//			points[pointCount] = intersect.point1();
	//			pointCount++;
	//		}
	//		if (!Utils::OutOfField(intersect.point2(),FIELD_BUFFER - 2 ))
	//		{
	//			points[pointCount] = intersect.point2();
	//			pointCount++;
	//		}
	//	}
	//	standardLine = CGeoLine(rightUp,rightDown);
	//	intersect = CGeoLineCircleIntersection(standardLine,standardCircle);
	//	if (intersect.intersectant())
	//	{
	//		if (!Utils::OutOfField(intersect.point1(),FIELD_BUFFER - 2 ))
	//		{
	//			points[pointCount] = intersect.point1();
	//			pointCount++;
	//		}
	//		if (!Utils::OutOfField(intersect.point2(),FIELD_BUFFER - 2 ))
	//		{
	//			points[pointCount] = intersect.point2();
	//			pointCount++;
	//		}
	//	}
	//	standardLine = CGeoLine(leftDown,rightDown);
	//	intersect = CGeoLineCircleIntersection(standardLine,standardCircle);
	//	if (intersect.intersectant())
	//	{
	//		if (!Utils::OutOfField(intersect.point1(),FIELD_BUFFER - 2 ))
	//		{
	//			points[pointCount] = intersect.point1();
	//			pointCount++;
	//		}
	//		if (!Utils::OutOfField(intersect.point2(),FIELD_BUFFER - 2 ))
	//		{
	//			points[pointCount] = intersect.point2();
	//			pointCount++;
	//		}
	//	}
	//	//排序
	//	double minAngleDiff = 2 * Param::Math::PI;
	//	for (int i = 0;i < pointCount;i++)
	//	{
	//		double nowDir = CVector(points[i] - ballPos).dir();
	//		double nowAngleDiff = fabs(Utils::Normalize(targetDir - nowDir));
	//		if (nowAngleDiff < minAngleDiff)
	//		{
	//			minAngleDiff = nowAngleDiff;
	//			defendPos = points[i];
	//		}
	//	}
	//}

	if (debug)
	{
		/*GDebugEngine::Instance()->gui_debug_x(defendPos,COLOR_RED);
		GDebugEngine::Instance()->gui_debug_line(leftUp,leftDown,COLOR_WHITE);
		GDebugEngine::Instance()->gui_debug_line(leftUp,rightUp,COLOR_WHITE);
		GDebugEngine::Instance()->gui_debug_line(rightUp,rightDown,COLOR_WHITE);
		GDebugEngine::Instance()->gui_debug_line(rightDown,leftDown,COLOR_WHITE);*/
	}
	return defendPos;
}

CGeoPoint CIndirectDefender::DefendPos(const CGeoPoint ballPos,const double dir,double radius)
{
	GDebugEngine::Instance()->gui_debug_line(ballPos,ballPos+Utils::Polar2Vector(radius,dir),COLOR_ORANGE);
	return ballPos + Utils::Polar2Vector(radius,dir);
}

void CIndirectDefender::FindMinMax(int num)
{
	maxnum=1;
	minnum=1;
	for (int j=2;j<=num;++j){
		if (_targetDir[j]>_targetDir[maxnum]){
			maxnum = j;
		}
		if (_targetDir[j]<_targetDir[minnum]){
			minnum=j;
		}
	}
}