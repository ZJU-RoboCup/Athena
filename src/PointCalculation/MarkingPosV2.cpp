/************************************************************************/
/* Copyright (c) CSC-RL, Zhejiang University							*/
/* Team：		SSL-ZJUNlict											*/
/* HomePage:	http://www.nlict.zju.edu.cn/ssl/WelcomePage.html		*/
/************************************************************************/
/* File:	  MarkingPosV2.cpp											*/
/* Func:	  盯人	attacker death marked								*/
/* Author:	  王群 2012-08-18											*/
/* Refer:	  ###														*/
/* E-mail:	  wangqun1234@zju.edu.cn									*/
/************************************************************************/
#include "MarkingPosV2.h"
#include "param.h"
#include "utils.h"
#include "WorldModel.h"
#include "GDebugEngine.h"
#include "CornerAreaPos.h"
#include "defence/DefenceInfo.h"
#include <atlstr.h>

namespace NameSpaceMarkingPosV2{
	// const
	CGeoPoint ourGoal;
	const double REF_AVOID_BALL = Param::Field::FREE_KICK_AVOID_BALL_DIST + Param::Vehicle::V2::PLAYER_SIZE*3.5;
	const double PUSH_ENEMY_BUFFER = 0;//original : PUSH_ENEMY_BUFFER = 15;
	const double CHASE_JUDGE_VALUE = 300;//判断chase的属性值阈值
	const double CHASE_MARK_BUFFER = 20.0;//向后追逐chase的
	
	const double NORMAL_DIST = 50;//50 gty 7.20
	const double ENEMY_SIN_MARK_BUFFER = 25;//！！！！！横向移动的收缩程度

	//const for DENY_BALL
	const double DENY_TIME_LIMIT = 0.5;//0.9	!!绕前的时间判断
	const double DENY_ANGLE_LIMIT = Param::Math::PI * 120 / 180;// !!绕前的可行动角度判断
	const double DENY_DIST_LIMIT_1 = 25.0;// !!
	const double DENY_DIST_LIMIT_2 = 10;// !!
	const double DENY_BUFFER = 3.0;
	const double EXIT_DENY_DIST = 25;

	const double NO_ADVANCE_BALL_VEL = 200;//!!!!!绕前时不使用advanceball的对方最小传球速度

	//const for BACK_LINE
	//const double BACK_BUFFER_IN = 160;//!!!!!
	//const double BACK_BUFFER_OUT = 180;//!!!!!
	const double BACK_BUFFER_IN = 210;//!!!!!
	const double BACK_BUFFER_OUT = 240;//!!!!!
	const double SPECIAL_AREA_PRE_TIME = 0.25;//特殊BACK_LINE模式下，对方的预测时间
	//const double SPECIAL_AREA_X_BUFFER = -100;//!!!!!特殊区域BACK_LINE模式下，x方向的阈值 -120
	const double SPECIAL_AREA_X_BUFFER = -280;

	//静态变量
	bool DENY_LOG[Param::Field::MAX_PLAYER+1] = {false,false,false,false,false,false,false};
	bool BACK_LOG[Param::Field::MAX_PLAYER+1] = {false,false,false,false,false,false,false};
	int DEFENDER_NUM_LOG[Param::Field::MAX_PLAYER+1] = {0,0,0,0,0,0,0};
	//关键开关：
	bool DENY_BALL_MODE = false;//绕前防守
	bool BACK_LINE_MODE = false;//回退防守
	bool SPECIAL_AREA_BACK_LINE_MODE = false;//针对角球，特殊区域的盯人全部采用BACK_LINE模式

	double RECEIVER_MARK_BUFFER = -3.0;//receiver 盯人距离

	//调试开关
	bool debug = false;
};

using namespace NameSpaceMarkingPosV2;

CMarkingPosV2::CMarkingPosV2()
{
	DECLARE_PARAM_READER_BEGIN(Defence)
	READ_PARAM(DENY_BALL_MODE)
	READ_PARAM(BACK_LINE_MODE)
	READ_PARAM(SPECIAL_AREA_BACK_LINE_MODE)
	DECLARE_PARAM_READER_END
	for (int i = 1;i <= Param::Field::MAX_PLAYER;++i)logCycle[i] = 0;
	for (int i = 1;i <= Param::Field::MAX_PLAYER;++i)markingPoint[i] = CGeoPoint(0,0);
	oppPriority = 0;
	oppNum = 0;
	clearPos();
	ourGoal = CGeoPoint(-Param::Field::PITCH_LENGTH/2,0);
	_logCycle = 0;
}

CMarkingPosV2::~CMarkingPosV2()
{}

//根据防守优先级选敌,在出现receiver时会出现优先级顺减
CGeoPoint CMarkingPosV2::getMarkingPos(const CVisionModule* pVision, const int pri)
{
	//oppPriority = pri;
	//int bestEnemy = DefenceInfo::Instance()->getAttackOppNumByPri(0);
	////当receiver为最高优先级的时候，这句话可以理解为场上对方是否有receiver
	//if (DefenceInfo::Instance()->getOppPlayerByNum(bestEnemy)->isTheRole("RReceiver"))
	//{
	//	if (oppPriority > 0)
	//	{
	//		oppPriority -= 1;
	//	}		
	//}
	//oppNum = DefenceInfo::Instance()->getSteadyAttackOppNumByPri(oppPriority);
	//if (pVision->Cycle() > logCycle[oppNum])
	//{
	//	logCycle[oppNum] = pVision->Cycle();
	//	markingPoint[oppNum] = generatePos(pVision);
	//}
	//return markingPoint[oppNum];
	checkAllMarkingPos(pVision);
	oppPriority = pri;
	//for (int i = 0; i <= 6; i++)
		//cout << i<<":"<<markingPoint[i].x()<<" "<<markingPoint[i].y()<< endl;
	//int bestEnemy = DefenceInfo::Instance()->getAttackOppNumByPri(0);
	////当receiver为最高优先级的时候，这句话可以理解为场上对方是否有receiver
	//if (DefenceInfo::Instance()->getOppPlayerByNum(bestEnemy)->isTheRole("RReceiver"))
	//{
	//	if (oppPriority > 0)
	//	{
	//		oppPriority -= 1;
	//	}		
	//}
	oppNum = DefenceInfo::Instance()->getSteadyAttackOppNumByPri(oppPriority);
	return markingPoint[oppNum];
}

CGeoPoint CMarkingPosV2::getMarkingPosByAbsolutePri(const CVisionModule* pVision , const int pri)
{
	/*oppPriority = pri;
	oppNum = DefenceInfo::Instance()->getSteadyAttackOppNumByPri(oppPriority);
	if (pVision->Cycle() > logCycle[oppNum])
	{
	logCycle[oppNum] = pVision->Cycle();
	markingPoint[oppNum] = generatePos(pVision);
	}
	return markingPoint[oppNum];*/
	checkAllMarkingPos(pVision);
	oppNum = DefenceInfo::Instance()->getSteadyAttackOppNumByPri(pri);
	return markingPoint[pri];
}

//根据对手车号选敌
CGeoPoint CMarkingPosV2::getMarkingPosByNum(const CVisionModule* pVision , const int num)
{
	//oppNum = num;
	//if (pVision->Cycle() > logCycle[oppNum])
	//{
	//	logCycle[oppNum] = pVision->Cycle();
	//	markingPoint[oppNum] = generatePos(pVision);
	//}
	//return markingPoint[oppNum];
	checkAllMarkingPos(pVision);
	return markingPoint[num];
}

void CMarkingPosV2::checkAllMarkingPos(const CVisionModule *pVision)
{
	//让防区域内的车不挤
	if (pVision->Cycle() > _logCycle){
		int attackCnt = DefenceInfo::Instance()->getAttackNum();
		areaList.clear();
		for (int i = 0;i<attackCnt;i++){
			int bestEnemy =DefenceInfo::Instance()->getAttackOppNumByPri(0);
			if (DefenceInfo::Instance()->getOppPlayerByNum(bestEnemy)->isTheRole("RReceiver")){
				oppNum = DefenceInfo::Instance()->getAttackOppNumByPri(i);
				//cout<<"oppNum is "<<oppNum<<endl;
				markingPoint[oppNum] = generatePos(pVision);
			}else{
				if (i< attackCnt - 1){//无法marking zero
					oppNum = DefenceInfo::Instance()->getAttackOppNumByPri(i+1);
					//cout<<"oppNum is "<<oppNum<<endl;
					markingPoint[oppNum] = generatePos(pVision);
				}
			}
		}
		vector<int>::iterator ir;
		int cnt = 0;
		//cout<<"areaList size is "<<areaList.size()<<endl;
		if (areaList.size() > 1){
			std::sort(areaList.begin(),areaList.end());
			for (ir = areaList.begin();ir!= areaList.end();ir++){
				markingPoint[*ir] = markingPoint[*ir]+Utils::Polar2Vector(20*cnt,(ourGoal - pVision->TheirPlayer(*ir).Pos()).dir());
				cnt = cnt+1;
				//cout<<*ir<<" "<<cnt<<endl;
			}
		}
		_logCycle = pVision->Cycle();
	}
}

bool CMarkingPosV2::isNearestBallReceiverBeDenied(const CVisionModule* pVision)
{
	double minDist = 1000;
	int receiverNum = 0;
	int attackNum = DefenceInfo::Instance()->getAttackNum();
	if (pVision->Ball().Vel().mod() > NO_ADVANCE_BALL_VEL)
	{
		if (debug)
		{
			cout<<"ball vel mod is  "<<pVision->Ball().Vel().mod()<<endl;
		}
		for (int i = 0;i < attackNum;++i)
		{
			int oppNum = DefenceInfo::Instance()->getAttackOppNumByPri(i);
			if (DefenceInfo::Instance()->getOppPlayerByNum(oppNum)->isTheRole("RReceiver"))
			{
				double dist_opp_ball = pVision->TheirPlayer(oppNum).Pos().dist(pVision->Ball().Pos());
				if (dist_opp_ball < minDist)
				{
					minDist = dist_opp_ball;
					receiverNum = oppNum;
				}
			}
		}
		if (debug)
		{
			cout<<"receiver is  "<<receiverNum<<endl;
			cout<<"deny   is   "<<DENY_LOG[receiverNum]<<endl;
		}
		if (0 != receiverNum)//包含存在receiver这个条件
		{
			if (true == DENY_LOG[receiverNum])
			{
				return true;
			}
		}
	}
	return false;
}

CGeoPoint CMarkingPosV2::generatePos(const CVisionModule* pVision)
{	
	if (pVision->TheirPlayer(oppNum).Valid() && 0 != oppNum)//存在且车号不为0
	{
		const int defenderNum = DefenceInfo::Instance()->getOurMarkDenfender(oppNum);
		const PlayerVisionT& opp = pVision->TheirPlayer(oppNum);
		const MobileVisionT& ball = pVision->Ball();
		const CGeoPoint oppPos = opp.Pos();
		const CVector oppVel = opp.Vel();
		CVector opp2ourGoalVector = CVector(ourGoal - oppPos);

		//盯人点临时量
		CGeoPoint finalPoint = oppPos;
		CGeoPoint basePoint = oppPos;
		double markBuffer = 0;
		//下面是对对手的动态预测
		double predictTime = 0.3;				//对对方小车的动态预测时间
		double angle_oppVel_opp2Goal = fabs(Utils::Normalize(opp2ourGoalVector.dir() - oppVel.dir()));

		if (DefenceInfo::Instance()->getOppPlayerByNum(oppNum)->isTheRole("RReceiver"))//如果对方是receiver
		{		
			//cout<<"1111111111111111"<<endl;
			//immortalKick
			if (DefenceInfo::Instance()->getOppPlayerByNum(oppNum)->getAttributeValue("AChaseAbility") > CHASE_JUDGE_VALUE)
			{
				bool defenderOK = (defenderNum != 0) && pVision->OurPlayer(defenderNum).Valid();
				CGeoPoint preOppPos = oppPos + Utils::Polar2Vector(oppVel.mod()*predictTime,oppVel.dir());//专对于ImmortalKick，不需要根据对手速度朝向修改predictTime
				bool oppFrontMe = preOppPos.dist(ourGoal) - Param::Vehicle::V2::PLAYER_SIZE > pVision->OurPlayer(defenderNum).Pos().dist(ourGoal);
				if (defenderOK && !oppFrontMe)//对方chaseKicker不在我前面
				{
					markBuffer = CHASE_MARK_BUFFER;
					basePoint = preOppPos;
				}else
				{
					markBuffer = RECEIVER_MARK_BUFFER;
					basePoint = oppPos;
				}
				finalPoint = basePoint + Utils::Polar2Vector(2*Param::Vehicle::V2::PLAYER_SIZE+markBuffer,opp2ourGoalVector.dir());
			}
			else//否则一般是touch模式
			{
				markBuffer = RECEIVER_MARK_BUFFER;
				//带速度时basePoint的修正
				if(oppVel.mod() > 30 && angle_oppVel_opp2Goal > Param::Math::PI / 2.0)
					basePoint = oppPos + Utils::Polar2Vector(oppVel.mod()*predictTime,oppVel.dir());//专对于ImmortalKick，不需要根据对手速度朝向修改predictTime
				finalPoint = basePoint + Utils::Polar2Vector(2*Param::Vehicle::V2::PLAYER_SIZE+markBuffer,opp2ourGoalVector.dir());
				if (true == DENY_BALL_MODE && false == BACK_LOG[oppNum])//绕前模式
				{
					CGeoLine ballVelLine = CGeoLine(ball.Pos(),ball.Vel().dir());
					CGeoPoint opp2ballVelLineProj = ballVelLine.projection(oppPos);
					if (debug)
					{
						GDebugEngine::Instance()->gui_debug_x(opp2ballVelLineProj,COLOR_YELLOW);
						GDebugEngine::Instance()->gui_debug_line(ball.Pos(),opp2ballVelLineProj,COLOR_WHITE);
					}
					if (false == DENY_LOG[oppNum])//进入绕前
					{
						//绕前条件
						//球到垂点的时间条件
						bool timeOK = opp2ballVelLineProj.dist(ball.Pos()) / (ball.Vel().mod() + 0.1) > DENY_TIME_LIMIT;
						//角度条件
						double angle_rBallVel_opp2Goal = fabs(Utils::Normalize(opp2ourGoalVector.dir()-Utils::Normalize(ball.Vel().dir()+Param::Math::PI)));
						bool angleOK = angle_rBallVel_opp2Goal < DENY_ANGLE_LIMIT;
						//超严格条件：采取绕前行动之前的位置条件，要求已经达到原防守点才能绕前
						CGeoPoint tempPos = oppPos + Utils::Polar2Vector(2*Param::Vehicle::V2::PLAYER_SIZE,opp2ourGoalVector.dir());
						bool dist1OK = opp2ballVelLineProj.dist(oppPos) < DENY_DIST_LIMIT_1;
						bool dist2OK = tempPos.dist(pVision->OurPlayer(defenderNum).Pos()) < DENY_DIST_LIMIT_2;
						bool ballVelOK = ball.Vel().mod() < 500;
						if (debug)
						{
							//cout << opp2ballVelLineProj.dist(ball.Pos()) / (ball.Vel().mod() + 0.1)<<endl;
							cout<< "deny condition  " << timeOK <<"  " << angleOK <<"  "<< dist1OK <<"  "<< dist2OK <<"  "<< ballVelOK <<endl;;
						}
						if (timeOK && angleOK && dist1OK && dist2OK && ballVelOK)
						{
							DENY_LOG[oppNum] = true;
						}
					}
					if (true == DENY_LOG[oppNum])//绕前进行中
					{
						//绕前动作
						finalPoint = opp2ballVelLineProj + Utils::Polar2Vector(2*Param::Vehicle::V2::PLAYER_SIZE+DENY_BUFFER,Utils::Normalize(ball.Vel().dir()+Param::Math::PI));
						if (debug)
						{
							GDebugEngine::Instance()->gui_debug_x(finalPoint,COLOR_YELLOW);
							CString denyBall;
							denyBall.Format("%d: deny Ball",oppNum);
							GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(25+oppNum*10,-50),denyBall,COLOR_YELLOW);
						}
						//退出绕前判断
						bool defenderChange = DEFENDER_NUM_LOG[oppNum] != defenderNum;
						bool oppLeaves = oppPos.dist(opp2ballVelLineProj) > EXIT_DENY_DIST;
						bool oppNotReceiver = (false == DefenceInfo::Instance()->getOppPlayerByNum(oppNum)->isTheRole("RReceiver"));
						if (defenderChange || oppLeaves || oppNotReceiver)
						{
							DENY_LOG[oppNum] = false;
						}
					}
				}
			}			
		}else{//不是receiver的时候	
			//正常点计算
			//cout<<"22222222222222222"<<endl;
			double sinPre = std::sin(angle_oppVel_opp2Goal);
			double cosPre = std::cos(angle_oppVel_opp2Goal);
			if (oppPos.x() > -NORMAL_DIST && oppVel.mod() > 35 && 
				angle_oppVel_opp2Goal < Param::Math::PI * 75/180.0)//根据位置判断预测量
			{
				//predictTime = (oppPos.x() + NORMAL_DIST) / 120.0;
				predictTime += predictTime * (0.5 + cosPre);
			}
			int tempFlag = Utils::Normalize(opp2ourGoalVector.dir()-oppVel.dir()) > 0 ? -1 : 1;
			//added.2014/6/2 横向盯人的时间差，待测试
			double cosParam  =1.25;
			double sinParam = 0.2;
		/*	if (sinPre >0.8 && Utils::Normalize(opp2ourGoalVector.dir() - oppVel.dir()) <=Param::Math::PI/2){
				sinParam = 2.0;
			}*/
			CVector oppVelModified = Utils::Polar2Vector(oppVel.mod()*cosPre*cosParam,opp2ourGoalVector.dir()) +
				Utils::Polar2Vector(oppVel.mod()*sinPre*sinParam,Utils::Normalize(opp2ourGoalVector.dir()+tempFlag*Param::Math::PI/2.0));
			CGeoPoint oppPrePos = oppPos + Utils::Polar2Vector(oppVelModified.mod() * predictTime,oppVelModified.dir());
			//GDebugEngine::Instance()->gui_debug_x(oppPrePos,COLOR_WHITE);
			//cout<<"sinPre is "<<sinPre<<" cosPre is "<<cosPre<<endl;
			CVector oppPre2ourGoal = ourGoal - oppPrePos;
			basePoint = oppPrePos + Utils::Polar2Vector(2*Param::Vehicle::V2::PLAYER_SIZE+5,oppPre2ourGoal.dir());
			//cout << oppPrePos.x()<<" "<<oppPrePos.y()<< endl;
			if (oppPrePos.dist(CGeoPoint(-450, 0)) > 280) {
				//cout << "in:"<< (oppPrePos.dist(CGeoPoint(-450, 0)) - 200) / 125 << endl;
				markBuffer += (oppPrePos.dist(CGeoPoint(-450, 0)) - 280) / 2;
			}
			//if (oppPrePos.x() > -NORMAL_DIST)//要依据效果仔细调参数
			//{
			//	markBuffer += (NORMAL_DIST+oppPrePos.x()) / 1.5;
			//}
			//当对手有横向速度的时候，markBuffer根据sinPre的增大而增大
			if (oppVel.mod() > 60)//待修改！！
			{
				markBuffer += (ENEMY_SIN_MARK_BUFFER * sinPre * oppVel.mod() / 135.0);
			}
			//因为PUSH_ENEMY_BUFFER == 0所以相当没有使用，需要的时候再使用
			if (pVision->OurPlayer(defenderNum).Valid())
			{
				const PlayerVisionT& defender = pVision->OurPlayer(defenderNum);
				double def2enemy_goal2enemy_Angle = fabs(Utils::Normalize(oppPre2ourGoal.dir() - (defender.Pos() - oppPrePos).dir()));
				if (def2enemy_goal2enemy_Angle < Param::Math::PI / 2.0)
				{
					if (oppPre2ourGoal.mod() < 200)//挤走对方
					{
						markBuffer -= PUSH_ENEMY_BUFFER * oppPre2ourGoal.mod() / 200;
					}
				}			
			}
			//markBuffer = 0;
			GDebugEngine::Instance()->gui_debug_x(oppPrePos,COLOR_WHITE);
			basePoint = basePoint + Utils::Polar2Vector(markBuffer,oppPre2ourGoal.dir());
			GDebugEngine::Instance()->gui_debug_line(oppPrePos,basePoint,COLOR_WHITE);
			//GDebugEngine::Instance()->gui_debug_x(basePoint,COLOR_YELLOW);
			//定位球时的处理!!
			const string refMsg = WorldModel::Instance()->CurrentRefereeMsg();
			const CGeoPoint ballPos = ball.Pos();
			CGeoPoint p1,p2;
			if ("theirIndirectKick" == refMsg || "theirDirectKick" == refMsg || "theirKickOff" == refMsg || "gameStop" == refMsg)
			{
				if (basePoint.dist(ballPos) < REF_AVOID_BALL)
				{
					//GDebugEngine::Instance()->gui_debug_line(ballPos,basePoint);
					CGeoCirlce ballCircle = CGeoCirlce(ballPos,REF_AVOID_BALL);
					CGeoLine markLine = CGeoLine(basePoint,ourGoal);
					CGeoLineCircleIntersection markIntersect = CGeoLineCircleIntersection(markLine,ballCircle);
					if (markIntersect.intersectant())
					{
						p1 = markIntersect.point1();
						p2 = markIntersect.point2();
						if (p1.dist(ourGoal) < p2.dist(ourGoal))
						{
							basePoint = p1;
						} else basePoint = p2;
					}
				}
			}
			finalPoint = basePoint;
			//回退防守模式
			if (true == BACK_LINE_MODE)
			{
				double opp2ourGoalDist = opp2ourGoalVector.mod();
				//opp2ourGoalDist = (ourGoal - oppPrePos).mod();
				if (false == BACK_LOG[oppNum])
				{
					//判断进入
					if (opp2ourGoalDist < BACK_BUFFER_IN || true == isInSpecialAreaBackLineMode(pVision,oppNum))
					{
						BACK_LOG[oppNum] = true;
					}
				}
				if (true == BACK_LOG[oppNum])
				{
					//动作点计算
					if (debug){
						CString backLine;
						backLine.Format("%d: back line",oppNum);
						GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(25+oppNum*10,50),backLine,COLOR_YELLOW);
					}
					CGeoPoint RtargetPoint = DefendUtils::reversePoint(oppPrePos);
					CGeoPoint Rgoal = DefendUtils::reversePoint(ourGoal);
					finalPoint = DefendUtils::reversePoint(DefendUtils::calcDefenderPointV2(RtargetPoint,CVector(Rgoal - RtargetPoint).dir(),POS_SIDE_MIDDLE,0,1.0));
					//判断退出
					if (opp2ourGoalDist > BACK_BUFFER_OUT && false == isInSpecialAreaBackLineMode(pVision,oppNum))
					{
						BACK_LOG[oppNum] = false;
					}
				}
			}
			//GDebugEngine::Instance()->gui_debug_x(finalPoint,COLOR_WHITE);
		}
		DEFENDER_NUM_LOG[oppNum] = defenderNum;
		//markfield的点的补充
		vector<MarkField> fieldList = DefenceInfo::Instance()->getMarkFieldList();
		vector<MarkField>::iterator ir;
		for (ir = fieldList.begin();ir!= fieldList.end(); ir++){
			if (DefenceInfo::Instance()->checkInRecArea(oppNum,pVision,*ir)){
				CGeoRectangle rect = CGeoRectangle((*ir)._upLeft,(*ir)._downRight);
				CGeoLine line = CGeoLine(pVision->TheirPlayer(oppNum).Pos(),ourGoal);
				CGeoLineRectangleIntersection intersect = CGeoLineRectangleIntersection(line,rect);
				if (intersect.intersectant()){
					CGeoPoint tmpPoint;
					if (intersect.point1().dist(ourGoal)>intersect.point2().dist(ourGoal)){
						tmpPoint = intersect.point2();
					}else{
						tmpPoint = intersect.point1();
					}
					tmpPoint = tmpPoint+Utils::Polar2Vector(10,(ourGoal - oppPos).dir());
					if (tmpPoint.dist(oppPos)>finalPoint.dist(oppPos)){
						finalPoint = tmpPoint;
						areaList.push_back(oppNum);
					}
					//GDebugEngine::Instance()->gui_debug_msg(finalPoint,"F",COLOR_WHITE);
				}
			}
		}
		//GDebugEngine::Instance()->gui_debug_x(finalPoint,COLOR_WHITE);
		return finalPoint;
	} 
	return CGeoPoint(0,150);
	//return DefendUtils::reversePoint(DefendUtils::calcDefenderPointV2(DefendUtils::reversePoint(pVision->Ball().Pos()),CVector(pVision->Ball().Pos()-ourGoal).dir(),POS_SIDE_MIDDLE,0));
}



bool CMarkingPosV2::isInSpecialAreaBackLineMode(const CVisionModule *pVision,const int num)
{
	static bool theResult[Param::Field::MAX_PLAYER+1] = {false,false,false,false,false,false,false};
	static int theLogCycle[Param::Field::MAX_PLAYER+1] = {0,0,0,0,0,0,0};
	static int theBallLogCycle = 0;
	static CGeoPoint logBallPos = pVision->Ball().Pos(); 
	if (pVision->Cycle() > theLogCycle[num])
	{
		theResult[num] = false;
		if (true == SPECIAL_AREA_BACK_LINE_MODE)
		{
			const string refMsg = WorldModel::Instance()->CurrentRefereeMsg();
			if ("theirIndirectKick" == refMsg || "theirDirectKick" == refMsg || "gameStop" == refMsg)//条件:裁判盒方面
			{
				if (pVision->Cycle() > theBallLogCycle)
				{
					if (logBallPos.dist(pVision->Ball().Pos()) > 5)
					{
						logBallPos = pVision->Ball().Pos();
					}
					theBallLogCycle = pVision->Cycle();
				}
				if (logBallPos.x() < -Param::Field::PITCH_LENGTH / 2 + Param::Field::PENALTY_AREA_DEPTH + 10)//条件：我方角球
				{
					const PlayerVisionT& opp = pVision->TheirPlayer(num);
					CGeoPoint oppPrePos = opp.Pos() + Utils::Polar2Vector(opp.Vel().mod() * SPECIAL_AREA_PRE_TIME,opp.Vel().dir());
					theResult[num] = checkInSpecialArea_A(oppPrePos,logBallPos);
				}
			}
		}
		theLogCycle[num] = pVision->Cycle();
	}
	return theResult[num];
}

bool CMarkingPosV2::checkInSpecialArea_A(const CGeoPoint p,const CGeoPoint ballPos)
{
	int theFlag = ballPos.y() > 0 ? -1 : 1;
	CGeoPoint p1 = CGeoPoint(-Param::Field::PITCH_LENGTH/2.0 + Param::Field::PENALTY_AREA_DEPTH,theFlag*Param::Field::PENALTY_AREA_L/2.0);
	double refDir = CVector(p1 - CGeoPoint(-Param::Field::PITCH_LENGTH/2.0,0)).dir();
	double goal2oppDir = CVector(p - CGeoPoint(-Param::Field::PITCH_LENGTH/2.0,0)).dir();
	if (p.x() < SPECIAL_AREA_X_BUFFER && (goal2oppDir*theFlag > refDir*theFlag))
	{
		if (debug){
			CString backLine;
			backLine.Format("%d: special line",oppNum);
			GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(125+oppNum*10,50),backLine,COLOR_YELLOW);
		}
		return true;
	}
	return false;
}

