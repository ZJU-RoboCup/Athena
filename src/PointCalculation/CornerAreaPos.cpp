/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team：		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  CornerAreaPos.cpp											*/
/* Func:	  球在我半场底角，对手挑球传中时，用于防守对方接应队员头球的点*/
/* Author:	  王群 2012-08-18											*/
/* Refer:	  ###														*/
/* E-mail:	  wangqun1234@zju.edu.cn									*/
/************************************************************************/

#include "CornerAreaPos.h"
#include "param.h"
#include "utils.h"
#include "WorldModel.h"
#include "GDebugEngine.h"
#include <math.h>
#include "defence/DefenceInfo.h"

namespace{
	bool debug = false;	

	const double GoalBuffer = 2;
	CGeoPoint GOAL_LEFT_POS;
	CGeoPoint GOAL_RIGHT_POS;
	CGeoPoint GOAL_CENTRE_POS;

	const double CHANGE_BUFFER = 50;
	const double ABSOLUTELY_IN = 5; //敌人已经彻底进入禁区，不再考虑

	const double extremeDefDist = 7;//一边发角球时，如果头球车位置y值超过该边，则不予盯防
	const double OPP_PRE_TIME = 1.5;//对敌人的带速度位置预测时间
	const double OPP_REAL_PRE_TIME = 0.2;//对敌人真实位置的预测时间
	const double DEAL_HIT_DIST = 35;//待联防的两车较近的时候再进入状态
	const double BOTTOM_EXTREME_ANGLE = Param::Math::PI * 80 / 180;
	const double MIDDLE_EXTREME_ANGLE = Param::Math::PI * 7 / 180;
};

CCornerAreaPos::CCornerAreaPos()
{
	//本算法的初始化
	_CAPos = CGeoPoint(0,0);
	_lastCAPos = CGeoPoint(0,0);
	GOAL_LEFT_POS = CGeoPoint(-Param::Field::PITCH_LENGTH / 2, -Param::Field::GOAL_WIDTH / 2 - GoalBuffer);
	GOAL_RIGHT_POS = CGeoPoint(-Param::Field::PITCH_LENGTH / 2, Param::Field::GOAL_WIDTH / 2 + GoalBuffer);
	GOAL_CENTRE_POS = CGeoPoint(-Param::Field::PITCH_LENGTH / 2,0);
	_lastCycle = 0;
	//_forMarkingPos = CGeoPoint(0,0);
	//_headAttackEnemy = 0;
}

CCornerAreaPos::~CCornerAreaPos()
{}

CGeoPoint CCornerAreaPos::getCornerAreaPos(const CVisionModule* pVision)
{
	if (pVision->Cycle() == _lastCycle) {
		return _lastCAPos;
	} else{
		_lastCycle = pVision->Cycle();
	}
	//生成中卫点
	setPos(generatePos(pVision));	
	return _CAPos;
}

//bool CCornerAreaPos::isUsed(const CVisionModule* pVision)
//{
//	if (pVision->Cycle() - _lastCycle > 3)
//	{
//		_headAttackEnemy = 0;
//		_dealWithHit = false;
//		return false;
//	} else return true;
//}

CGeoPoint CCornerAreaPos::generatePos(const CVisionModule* pVision)
{
	_CAPos = CGeoPoint(-340,0); //已修改为Brazil zhyaic
	//以下部分没有反向计算！！
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	static posSide ballSide = POS_SIDE_RIGHT;
	const string refMsg = WorldModel::Instance()->CurrentRefereeMsg();
	if ("theirIndirectKick" == refMsg || "theirDirectKick" == refMsg || "theirKickOff" == refMsg || "gameStop" == refMsg)
	{
		if (ballPos.y() > 0)
		{
			ballSide = POS_SIDE_RIGHT;
		} else ballSide = POS_SIDE_LEFT;
	}	
	int attackNum = DefenceInfo::Instance()->getAttackNum();
	//求取破防成功的最近队员
	int enemyBreakMe = 0;
	double minDist = 1000;
	for (int i = 0;i < attackNum;++i)
	{
		int enemyNum = DefenceInfo::Instance()->getAttackOppNumByPri(i);
		if (!Utils::PlayerNumValid(enemyNum)) {
			continue;
		}
		const PlayerVisionT& opp = pVision->TheirPlayer(enemyNum);
		if (opp.Valid() && !Utils::InOurPenaltyArea(opp.Pos(),ABSOLUTELY_IN))
		{
			CGeoPoint oppPos = opp.Pos();
			CGeoPoint oppPrePos = oppPos + Utils::Polar2Vector(opp.Vel().mod()*OPP_PRE_TIME,opp.Vel().dir());
			if (POS_SIDE_LEFT == ballSide)
			{
				if (oppPos.y() < extremeDefDist)
				{
					continue;
				}
			} else if (POS_SIDE_RIGHT == ballSide)
			{
				if (oppPos.y() > -1 * extremeDefDist)
				{
					continue;
				}
			}
			//判断是否盯防良好
			bool markWell = true;
			double markerDistFactor = DefenceInfo::Instance()->getOppPlayerByNum(enemyNum)->getAttributeValue("AMarkerDistFactor");
			double markerDirFactor = DefenceInfo::Instance()->getOppPlayerByNum(enemyNum)->getAttributeValue("AMarkerDirFactor");
			if (markerDirFactor > Param::Math::PI * 100 / 180 || markerDistFactor > 250 || sin(markerDirFactor) * markerDistFactor > 30)
			{
				markWell = false;
			}
			if (false == markWell)
			{
				double nowDist = oppPos.dist(GOAL_CENTRE_POS);
				if (nowDist < minDist)
				{
					minDist = nowDist;
					enemyBreakMe = enemyNum;
				}
			}
		}
	}
	//取点
	CGeoPoint targetPos = CGeoPoint(0,0);
	if (POS_SIDE_LEFT == ballSide)
	{
		targetPos = CGeoPoint(-(Param::Field::PITCH_LENGTH/2-Param::Field::PENALTY_AREA_DEPTH),Param::Field::PENALTY_AREA_WIDTH/2); // 已修改为Brazil zhyaic
	} else if (POS_SIDE_RIGHT == ballSide)
	{
		targetPos = CGeoPoint(-(Param::Field::PITCH_LENGTH/2-Param::Field::PENALTY_AREA_DEPTH),-Param::Field::PENALTY_AREA_WIDTH/2); // 已修改为Brazil zhyaic
	}

	if (0 != enemyBreakMe)//敌人存在
	{
		const PlayerVisionT& headOpp = pVision->TheirPlayer(enemyBreakMe);
		targetPos = headOpp.Pos() + Utils::Polar2Vector(headOpp.Vel().mod()*OPP_REAL_PRE_TIME,headOpp.Vel().dir());
		double angle_goal2target = CVector(targetPos - GOAL_CENTRE_POS).dir();
		if (POS_SIDE_LEFT == ballSide)
		{
			if (angle_goal2target >= BOTTOM_EXTREME_ANGLE)
			{
				targetPos = GOAL_CENTRE_POS + Utils::Polar2Vector(150*Param::Field::RATIO,BOTTOM_EXTREME_ANGLE);
			}else if (angle_goal2target <= MIDDLE_EXTREME_ANGLE)
			{
				targetPos = GOAL_CENTRE_POS + Utils::Polar2Vector(150*Param::Field::RATIO,MIDDLE_EXTREME_ANGLE);
			}
		}else if (POS_SIDE_RIGHT == ballSide)
		{
			if (angle_goal2target <= -1*BOTTOM_EXTREME_ANGLE)
			{
				targetPos = GOAL_CENTRE_POS + Utils::Polar2Vector(150*Param::Field::RATIO,-1*BOTTOM_EXTREME_ANGLE);
			}else if (angle_goal2target >= -1*MIDDLE_EXTREME_ANGLE)
			{
				targetPos = GOAL_CENTRE_POS + Utils::Polar2Vector(150*Param::Field::RATIO,-1*MIDDLE_EXTREME_ANGLE);
			}
		}		
	}
	//开始反向计算，注意！！
	CGeoPoint RTargetPos = DefendUtils::reversePoint(targetPos);
	//GDebugEngine::Instance()->gui_debug_x(targetPos,COLOR_BLACK);
	CGeoPoint RblockPos = DefendUtils::calcDefenderPointV2(RTargetPos,CVector(DefendUtils::reversePoint(GOAL_CENTRE_POS) - RTargetPos).dir(),POS_SIDE_MIDDLE,0,1.5);
	_CAPos = DefendUtils::reversePoint(RblockPos);
	//解决本点与盯人相冲突的问题
	//if (WorldModel::Instance()->getEnemySituation().queryMarked(enemyBreakMe))//有人盯防
	//{
	//	if (Utils::InOurPenaltyArea(targetPos,CHANGE_BUFFER))//在我禁区外一圈
	//	{
	//		int defenderNum = WorldModel::Instance()->getOurMarkDenfender(enemyBreakMe);
	//		if (pVision->OurPlayer(defenderNum).Valid() && _CAPos.dist(pVision->OurPlayer(defenderNum).Pos()) < DEAL_HIT_DIST)//而且盯人点和防头球点距离很近
	//		{
	//			_headAttackEnemy = enemyBreakMe;
	//			_dealWithHit = true;
	//			const PlayerVisionT& me = pVision->OurPlayer(defenderNum);
	//			if (/*me.Y() < _CAPos.y()*/CVector(me.Pos() - GOAL_CENTRE_POS).dir() - CVector(_CAPos - GOAL_CENTRE_POS).dir() < 0)//盯人者 在 防头球者左边 ，防头球者走到右边
	//			{
	//				RblockPos = DefendUtils::calcDefenderPoint(RTargetPos,CVector(DefendUtils::reversePoint(GOAL_CENTRE_POS) - RTargetPos).dir(),POS_SIDE_LEFT);//注意这里的POS_SIDE_LEFT没有错
	//				_forMarkingPos = DefendUtils::reversePoint(DefendUtils::calcDefenderPoint(RTargetPos,CVector(DefendUtils::reversePoint(GOAL_CENTRE_POS) - RTargetPos).dir() + 0.05,POS_SIDE_RIGHT));
	//				_CAPos = DefendUtils::reversePoint(RblockPos);
	//			} else 
	//			{
	//				RblockPos = DefendUtils::calcDefenderPoint(RTargetPos,CVector(DefendUtils::reversePoint(GOAL_CENTRE_POS) - RTargetPos).dir(),POS_SIDE_RIGHT);
	//				_forMarkingPos = DefendUtils::reversePoint(DefendUtils::calcDefenderPoint(RTargetPos,CVector(DefendUtils::reversePoint(GOAL_CENTRE_POS) - RTargetPos).dir() - 0.05,POS_SIDE_LEFT));
	//				_CAPos = DefendUtils::reversePoint(RblockPos);
	//			}
	//		}
	//	}			
	//}
	_lastCAPos = _CAPos;
	return _CAPos;
}
