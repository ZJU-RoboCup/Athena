#include "OppAttributesFactory.h"
#include "AttributeSet.h"
#include "geometry.h"
#include "param.h"
#include "VisionModule.h"
#include "BestPlayer.h"
#include "ShootRangeList.h"
#include "defence/DefenceInfo.h"
#include "PlayInterface.h"
#include "TaskMediator.h"
#include "WorldModel.h"

namespace{
	//�������������Ҫ�ľ�̬�����ͳ���
	CGeoPoint	OUR_GOAL_CENTER;
	CGeoPoint	THEIR_GOAL_CENTER ;
	CGeoLine OUR_GOAL_LINE ;
	CGeoPoint LEFTGUDGE;
	CGeoPoint RIGHTGUDGE;
	double	OPP_PASS_VEL = 50;				//�жϵ��˿��ٴ��������
};

//attribute define
DEFINE_CLASS_EX_CPP(ATest);//for test 
DEFINE_CLASS_EX_CPP(ATest2);//for test
DEFINE_CLASS_EX_CPP(ACanBeLeader);
DEFINE_CLASS_EX_CPP(ADist2TheirGoal);//return double ���Է��������ĵ�ľ���
DEFINE_CLASS_EX_CPP(ADist2OurGoal);//return double ���ҷ��������ĵ�ľ���
DEFINE_CLASS_EX_CPP(ADist2Ball);	//return double	 ����ľ���
DEFINE_CLASS_EX_CPP(AValid);//return 1 or 0 �Է��Ƿ����
DEFINE_CLASS_EX_CPP(ABestPlayer);//return 1 or 0 �Ƿ��ǶԷ�BestPlayer
DEFINE_CLASS_EX_CPP(AShootRange);//return double ���ŽǶ�
DEFINE_CLASS_EX_CPP(AShootRangeWithoutMarker);//return double ���ҷ�Marker�ų�����ʱ�����ŽǶ�
DEFINE_CLASS_EX_CPP(AFillingInDist);//return double ��������
DEFINE_CLASS_EX_CPP(AKeeperDirFactor);//return double �Է�BestPlayer��������Է�BestPlayer�ͶԷ����߳���֮��
DEFINE_CLASS_EX_CPP(AMarkerDistFactor);//return double ���ҷ�Marker�ľ��룬���˶���ʱ��ֵ����500���ݶ���
DEFINE_CLASS_EX_CPP(AMarkerDirFactor);	//retrun double �Է����ų���͵��ҷ�Marker�ĳ���֮��,���˶���ʱ��ֵ����PI���ݶ���
DEFINE_CLASS_EX_CPP(AShootReflectAngle);//return double �Է����������
DEFINE_CLASS_EX_CPP(AReceiveReflectAngle);//return double or PI��������н����receiver�������
DEFINE_CLASS_EX_CPP(AImmortalFactor);//return double �������������������򵽸ö�Ա�����ļн�,��̬����
DEFINE_CLASS_EX_CPP(APassBlock);//return double����·���赲���룬���ҷ��赲�ľ��� ����ע�⣺����200cmʱ��������200
DEFINE_CLASS_EX_CPP(APosX);//return double λ�õ�Xֵ
DEFINE_CLASS_EX_CPP(APosY);//return double λ�õ�Yֵ
DEFINE_CLASS_EX_CPP(AVelX);//return double �ٶȵ�Xֵ
DEFINE_CLASS_EX_CPP(AVelY);//return double �ٶȵ�Yֵ
DEFINE_CLASS_EX_CPP(AGoalie);//return 1 or 0 �Է��Ƿ���goalie
DEFINE_CLASS_EX_CPP(ARecive);//�������ã�����
DEFINE_CLASS_EX_CPP(ATouchAbility);// return MAX_VALUE or double
DEFINE_CLASS_EX_CPP(AChaseAbility);// return MAX_VALUE or double 

CAttributeFactory::CAttributeFactory()
{
	_attrSet = new CAttributeSet();
	configuration();
	OUR_GOAL_CENTER = CGeoPoint(-1*Param::Field::PITCH_LENGTH/2,0);
	THEIR_GOAL_CENTER = CGeoPoint(Param::Field::PITCH_LENGTH/2,0);
	//modified for Brazil by thj
	LEFTGUDGE = CGeoPoint(-Param::Field::PITCH_LENGTH/2, -(Param::Field::PITCH_WIDTH/2 - Param::Vehicle::V2::PLAYER_SIZE*2));
	RIGHTGUDGE = CGeoPoint(-Param::Field::PITCH_LENGTH/2, Param::Field::PITCH_WIDTH/2 - Param::Vehicle::V2::PLAYER_SIZE*2);
	OUR_GOAL_LINE = CGeoLine(CGeoPoint(-Param::Field::PITCH_LENGTH/2.0,-Param::Field::PITCH_WIDTH/2.0),CGeoPoint(-Param::Field::PITCH_LENGTH/2.0,Param::Field::PITCH_WIDTH/2.0));
}

CAttributeFactory::~CAttributeFactory()
{
	delete _attrSet;
}

void CAttributeFactory::configuration()
{
	//cout<<NameArrayAKeeperDirFacto<<endl;
//	_attrSet->add(new ATest());
//	_attrSet->add(new ATest2());
	_attrSet->add(new ACanBeLeader());
	_attrSet->add(new ADist2TheirGoal());
	_attrSet->add(new ADist2OurGoal());
	_attrSet->add(new ADist2Ball());
	_attrSet->add(new AValid());
	_attrSet->add(new ABestPlayer());
	_attrSet->add(new AShootRange());
	_attrSet->add(new AShootRangeWithoutMarker());
	_attrSet->add(new AFillingInDist());
	_attrSet->add(new AKeeperDirFactor());
	_attrSet->add(new AMarkerDistFactor());
	_attrSet->add(new AMarkerDirFactor());
	_attrSet->add(new APosX());
	_attrSet->add(new APosY());
	_attrSet->add(new AVelX());
	_attrSet->add(new AVelY());
	_attrSet->add(new AGoalie());
	_attrSet->add(new ARecive());
	_attrSet->add(new AShootReflectAngle());
	_attrSet->add(new AReceiveReflectAngle());
	_attrSet->add(new APassBlock());
	_attrSet->add(new AImmortalFactor());
	_attrSet->add(new ATouchAbility());
	_attrSet->add(new AChaseAbility());
}

//for test
EVALUATE_ATTRIBUTE(ATest)
{
	double tempValue = 0;//����ϰ�ߣ�tempValue��ʼ��ʱ���������ֵ
	tempValue = pVision->TheirPlayer(num).Pos().x();
	setValue(tempValue);
}

//for test
EVALUATE_ATTRIBUTE(ATest2)
{
	double tempValue = pVision->TheirPlayer(num).Pos().y();
	setValue(tempValue);
}

EVALUATE_ATTRIBUTE(ACanBeLeader)
{
	double tempValue = 0;
	if (num == BestPlayer::Instance()->getTheirBestPlayer())
	{
		tempValue = 1;
		const string refMsg = WorldModel::Instance()->CurrentRefereeMsg();
		if (("theirIndirectKick" == refMsg || "theirDirectKick" == refMsg)
			&& pVision->Ball().X() > 240
			&& pVision->Ball().X() > pVision->TheirPlayer(num).X())
		{
			for (int i = 1;i <= Param::Field::MAX_PLAYER; i++)
			{
				if (i != num)
				{
					double othertoBallDir = (pVision->Ball().Pos() - pVision->TheirPlayer(i).Pos()).dir();
					double toLeftGudgeDir = (LEFTGUDGE - pVision->TheirPlayer(i).Pos()).dir();
					double toRightGudgeDir = (RIGHTGUDGE - pVision->TheirPlayer(i).Pos()).dir();
					if (pVision->TheirPlayer(i).Pos().dist(pVision->Ball().Pos())<50
						&& othertoBallDir >= toRightGudgeDir
						&& othertoBallDir <= toLeftGudgeDir)
					{
						tempValue = 0;
					}
				}
			}
		}
	}
	else if (num != BestPlayer::Instance()->getTheirBestPlayer())
	{
		int best = BestPlayer::Instance()->getTheirBestPlayer();
		const string refMsg = WorldModel::Instance()->CurrentRefereeMsg();
		double besttoBallDir = (pVision->Ball().Pos() - pVision->TheirPlayer(best).Pos()).dir();
		if (("theirIndirectKick" == refMsg || "theirDirectKick" == refMsg)
			&& pVision->TheirPlayer(best).X() < pVision->Ball().X())
		{
			double metoBallDir = (pVision->Ball().Pos() - pVision->TheirPlayer(num).Pos()).dir();
			double toLeftGudgeDir = (LEFTGUDGE - pVision->TheirPlayer(num).Pos()).dir();
			double toRightGudgeDir = (RIGHTGUDGE - pVision->TheirPlayer(num).Pos()).dir();
			if (pVision->TheirPlayer(num).Pos().dist(pVision->Ball().Pos()) < 50
				&& metoBallDir >= toRightGudgeDir
				&& metoBallDir <= toLeftGudgeDir)
			{
				tempValue = 1;
			}
		}
	}
	setValue(tempValue);
}

//return double		���Է��������ĵ�ľ���
EVALUATE_ATTRIBUTE(ADist2TheirGoal)
{
	double tempValue = pVision->TheirPlayer(num).Pos().dist(THEIR_GOAL_CENTER);
	setValue(tempValue);
}
//return double		���ҷ��������ĵ�ľ���
EVALUATE_ATTRIBUTE(ADist2OurGoal)
{
	double tempValue = pVision->TheirPlayer(num).Pos().dist(OUR_GOAL_CENTER);
	setValue(tempValue);
}
//return double		����ľ���
EVALUATE_ATTRIBUTE(ADist2Ball)
{
	double tempValue = pVision->TheirPlayer(num).Pos().dist(pVision->Ball().Pos());
	setValue(tempValue);
}
//return 1 or 0		�Է��Ƿ����
EVALUATE_ATTRIBUTE(AValid)
{
	if (pVision->TheirPlayer(num).Valid())
	{
		setValue(1.00);
	} else setValue(0.00);
}
//return 1 or 0		�Ƿ��ǶԷ�BestPlayer
EVALUATE_ATTRIBUTE(ABestPlayer)
{
	if (num == BestPlayer::Instance()->getTheirBestPlayer())
	{
		setValue(1.00);
	}else setValue(0.00);
}
//return double		���ŽǶ�
EVALUATE_ATTRIBUTE(AShootRange)
{
	CShootRangeList shootRangeList(pVision,
													   SR_DEFENCE,
													   0,
													   pVision->TheirPlayer(num).Pos());
	const CValueRangeList& shootRange = shootRangeList.getShootRange();
	CValueRange* bestRange = NULL;
	double biggestRange = 0;
	if ( shootRange.size() > 0 ){
		bestRange = (CValueRange*)shootRange.getMaxRangeWidth();
		biggestRange = bestRange->getWidth();
	}
	setValue(biggestRange);
}
//return double		���ҷ�Marker�ų�����ʱ�����ŽǶ�
EVALUATE_ATTRIBUTE(AShootRangeWithoutMarker)
{
	CShootRangeList shootRangeList(pVision,
													   SR_DEFENCE,
													   DefenceInfo::Instance()->getOurMarkDenfender(num),
													   pVision->TheirPlayer(num).Pos());
	const CValueRangeList& shootRange = shootRangeList.getShootRange();
	CValueRange* bestRange = NULL;
	double biggestRange = 0;
	if ( shootRange.size() > 0 ){
		bestRange = (CValueRange*)shootRange.getMaxRangeWidth();
		biggestRange = bestRange->getWidth();
	}
	setValue(biggestRange);
}
//return double		��������
EVALUATE_ATTRIBUTE(AFillingInDist)
{
	const int ourGoalie = PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie());
	const int defender = DefenceInfo::Instance()->getOurMarkDenfender(num);
	const PlayerVisionT& opp = pVision->TheirPlayer(num);
	double minDist = 500;
	double tempDist = 0;
	for (int i = 1;i <= Param::Field::MAX_PLAYER; i++)
	{
		if (ourGoalie == i || defender == i)
		{
			continue;
		}
		const PlayerVisionT& replacer = pVision->OurPlayer(i);
		double angle_opp2replacer_opp2goal = fabs(Utils::Normalize(CVector(replacer.Pos()-opp.Pos()).dir()-CVector(OUR_GOAL_CENTER-opp.Pos()).dir()));
		if (replacer.X() > opp.X() + 50 || angle_opp2replacer_opp2goal > Param::Math::PI * 90 / 180)//X����˦��70cm�޷�����
		{
			continue;
		} else
		{
			tempDist = replacer.Pos().dist(opp.Pos());
			if (tempDist < minDist)
			{
				minDist = tempDist;
			}
		}
	}
	setValue(minDist);
}
//return double		�Է�BestPlayer��������Է�BestPlayer�ͶԷ����߳���֮��
EVALUATE_ATTRIBUTE(AKeeperDirFactor)
{
	double tempValue = Param::Math::PI;
	int keeperNum = BestPlayer::Instance()->getTheirBestPlayer();
	if (num != keeperNum)
	{
		const PlayerVisionT& theirKeeper = pVision->TheirPlayer(keeperNum);
		const PlayerVisionT& opp = pVision->TheirPlayer(num);
		tempValue = std::fabs(Utils::Normalize(CVector(opp.Pos() - theirKeeper.Pos()).dir() - theirKeeper.Dir()));
	}
	setValue(tempValue);
}
//return double		���ҷ�Marker�ľ��룬���˶���ʱ��ֵ����500���ݶ���
EVALUATE_ATTRIBUTE(AMarkerDistFactor)
{
	double tempValue = 500;
	const int defender = DefenceInfo::Instance()->getOurMarkDenfender(num);
	if (0 != defender)
	{
		tempValue = pVision->TheirPlayer(num).Pos().dist(pVision->OurPlayer(defender).Pos());
	} 
	setValue(tempValue);
}
////retrun double		�Է����ų���͵��ҷ�Marker�ĳ���֮��,���˶���ʱ��ֵ����PI���ݶ���
EVALUATE_ATTRIBUTE(AMarkerDirFactor)
{
	double tempValue = Param::Math::PI;
	const int defender = DefenceInfo::Instance()->getOurMarkDenfender(num);
	if (0 != defender)
	{
		const PlayerVisionT& me = pVision->OurPlayer(defender);
		const PlayerVisionT& opp = pVision->TheirPlayer(num);
		tempValue = fabs(Utils::Normalize(CVector(me.Pos() - opp.Pos()).dir() - CVector(OUR_GOAL_CENTER - opp.Pos()).dir()));
	} 
	setValue(tempValue);
}

EVALUATE_ATTRIBUTE(AShootReflectAngle)
{
	double tempValue = Param::Math::PI;
	CGeoPoint ballPos = pVision->Ball().Pos();
	CGeoPoint oppPos = pVision->TheirPlayer(num).Pos();
	tempValue = fabs(Utils::Normalize(CVector(ballPos - oppPos).dir() - CVector(OUR_GOAL_CENTER - oppPos).dir()));
	setValue(tempValue);
}

EVALUATE_ATTRIBUTE(AReceiveReflectAngle)
{
	double tempValue = Param::Math::PI;
	const MobileVisionT& ball = pVision->Ball();
	if (ball.Vel().mod() > OPP_PASS_VEL)
	{
		CGeoPoint rBallVelPos = ball.Pos() + Utils::Polar2Vector(100,Utils::Normalize(ball.Vel().dir()+Param::Math::PI));
		CGeoPoint oppPos = pVision->TheirPlayer(num).Pos();
		CGeoLine ballLine = CGeoLine(ball.Pos(),rBallVelPos);
		CGeoPoint projectP = ballLine.projection(oppPos);
		tempValue = fabs(Utils::Normalize(CVector(rBallVelPos - ball.Pos()).dir() - CVector(OUR_GOAL_CENTER - projectP).dir()));

    if (fabs(ball.Vel().dir()) > Param::Math::PI/3*2 && ball.X() > oppPos.x() + Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER) {
      double toProjectDist = oppPos.dist(projectP);
      if (toProjectDist < Param::Vehicle::V2::PLAYER_SIZE)
        toProjectDist = Param::Vehicle::V2::PLAYER_SIZE;
      toProjectDist /= toProjectDist;
      double oppFaceDir = pVision->TheirPlayer(num).Dir();
      tempValue = fabs(Utils::Normalize((rBallVelPos - ball.Pos()).dir() - oppFaceDir) * toProjectDist);
    }
	}
	setValue(tempValue);
}

//posX
EVALUATE_ATTRIBUTE(APosX)
{
	double tempValue = pVision->TheirPlayer(num).X();
	setValue(tempValue);
}
//posY
EVALUATE_ATTRIBUTE(APosY)
{
	double tempValue = pVision->TheirPlayer(num).Y();
	setValue(tempValue);
}
//velX
EVALUATE_ATTRIBUTE(AVelX)
{
	double tempValue = pVision->TheirPlayer(num).VelX();
	setValue(tempValue);
}
//velY
EVALUATE_ATTRIBUTE(AVelY)
{
	double tempValue = pVision->TheirPlayer(num).VelY();
	setValue(tempValue);
}

//is goalie,���޸�
EVALUATE_ATTRIBUTE(AGoalie)
{
	double tempValue = 0;
	double dist = 1000;
	int goalieNum = 0;
	for (int i = 1;i <= Param::Field::MAX_PLAYER;++i)
	{
		double myDist = pVision->TheirPlayer(i).Pos().dist(THEIR_GOAL_CENTER);
		if (myDist < dist)
		{
			dist = myDist;
			goalieNum = i;
		}
	}
	//modified for Brazil by thj
	if (num == goalieNum && pVision->TheirPlayer(num).Pos().x() > Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_DEPTH)
	{
		tempValue = 1.0;
	}
	setValue(tempValue);
}
//  ����Ϊ���������ʸ���ķ��������ҵķ���ģ��ǶȲ�
EVALUATE_ATTRIBUTE(ARecive)
{
	
	//for chasekick
	//	{
	//		double angleD = fabs(Utils::Normalize(ballSpeedDir - opp.Vel().dir()));
	//		double ballVel2ourGoal = fabs(Utils::Normalize(ball.Vel().dir() - CVector(OUR_GOAL_CENTER - ballPos).dir()));
	//		if (angleD < Param::Math::PI / 2.0 && opp.Vel().mod() >= ball.Vel().mod()
	//			&& ballVel2ourGoal < Param::Math::PI / 4.0)
	//		{
	//			tempValue = oppPos.dist(ballPos);
	//		}
	//	}
	//
	//setValue(tempValue);
}

EVALUATE_ATTRIBUTE(APassBlock)
{
	double tempValue = 90;
	CGeoPoint oPos = pVision->TheirPlayer(num).Pos();
	CGeoPoint ballPos = pVision->Ball().Pos();
	for (int i = 1;i <= Param::Field::MAX_PLAYER;++i)
	{
		int defender = DefenceInfo::Instance()->getOurMarkDenfender(num);
		if (pVision->OurPlayer(i).Valid() && i != defender)
		{
			CGeoPoint myPos = pVision->OurPlayer(i).Pos();
			double tempAngle1 = fabs(Utils::Normalize(CVector(myPos - oPos).dir() - CVector(ballPos - oPos).dir()));
			double tempAngle2 = fabs(Utils::Normalize(CVector(myPos - ballPos).dir() - CVector(oPos - ballPos).dir()));
			if (tempAngle1 < Param::Math::PI * 80 / 180 && tempAngle2 < Param::Math::PI * 80 / 180	&& myPos.dist(oPos) < 100)
			{
				CGeoLine tempLine = CGeoLine(ballPos,oPos);
				double dist = tempLine.projection(myPos).dist(myPos);
				if (dist < tempValue)
				{
					tempValue = dist;
				}
			}
		}
	}
	setValue(tempValue);
}

EVALUATE_ATTRIBUTE(AImmortalFactor)//���ڷ���Զ����immortal
{
	static CGeoPoint lastBallPos = pVision->Ball().Pos();
	static int lastCycle = pVision->Cycle();
	double tempValue = 0;
	const string refMsg = WorldModel::Instance()->CurrentRefereeMsg();
	if ("theirIndirectKick" == refMsg || "theirDirectKick" == refMsg)
	{
		if (pVision->Cycle() > lastCycle)
		{
			if (lastBallPos.dist(pVision->Ball().Pos()) > 3)
			{
				lastBallPos = pVision->Ball().Pos();
			}
			lastCycle = pVision->Cycle();
		}
		//���м���
		if (lastBallPos.x() > Param::Field::PITCH_LENGTH / 6)
		{
			CGeoPoint oppPos = pVision->TheirPlayer(num).Pos();
			CGeoPoint ballPos = pVision->Ball().Pos();
			CVector ball2opp = oppPos - ballPos;
			double angleDiff = fabs(Utils::Normalize(ball2opp.dir() - CVector(OUR_GOAL_CENTER - ballPos).dir()));
			CGeoLine line_ball2goal = CGeoLine(ballPos,OUR_GOAL_CENTER);
			double opp2lineDist = line_ball2goal.projection(oppPos).dist(oppPos);
			if (oppPos.dist(ballPos) < 180  && angleDiff < Param::Math::PI * 0.5)
			{
				tempValue = 10000.0 / ( opp2lineDist + 0.1);
				tempValue = tempValue > 400 ? 400 : tempValue;
			}			
		}
	}
	//cout<< " AImmortalFactor    :" << tempValue << endl;
	setValue(tempValue);
}

EVALUATE_ATTRIBUTE(ATouchAbility)
{
	double tempValue = 600;//�����Ե�ֵԽС������������Խ��
	const MobileVisionT& ball = pVision->Ball();
	double ballVelMod = pVision->Ball().Vel().mod();
	if (ballVelMod > OPP_PASS_VEL)
	{
		CGeoPoint ballPos = ball.Pos();
		double ballSpeedDir = ball.Vel().dir();
		const PlayerVisionT& opp = pVision->TheirPlayer(num);
		CGeoPoint oppPos = opp.Pos() + Utils::Polar2Vector(opp.Vel().mod()*0.7,opp.Vel().dir());
		CVector ball2opp = oppPos - ballPos;
		double angleDiff = fabs(Utils::Normalize(ball2opp.dir() - ballSpeedDir));
		if (angleDiff < Param::Math::PI * 0.6 && oppPos.x() < Param::Field::PITCH_LENGTH / 4)//0.6Ϊ����touch�ĽǶȼ���
		{
			CGeoLine ballSpeedLine = CGeoLine(ballPos,ballSpeedDir);
			CGeoPoint proPos = ballSpeedLine.projection(oppPos);
			tempValue = proPos.dist(oppPos) / (proPos.dist(ballPos) / (ballVelMod + 10) + 0.1) + oppPos.dist(ballPos) * 0.35;
		}
	}
	setValue(tempValue);
}

EVALUATE_ATTRIBUTE(AChaseAbility)
{
	//�Լ�������������
	static CGeoPoint logBallPos = pVision->Ball().RawPos();
	static int logCycle = pVision->Cycle();
	if (pVision->Cycle() > logCycle)
	{
		logCycle = pVision->Cycle();
		logBallPos = pVision->Ball().RawPos();
	}
	//�ر����immortal��chaseKicker
	double tempValue = 0;
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& opp = pVision->TheirPlayer(num);
	CVector ballVel = pVision->Ball().Pos() - logBallPos;
	double angle_opp2ball_opp2goal = fabs(Utils::Normalize(CVector(ball.Pos() - opp.Pos()).dir()-CVector(OUR_GOAL_CENTER - opp.Pos()).dir()));
	//double angle_oppVel_opp2goal = fabs(Utils::Normalize(opp.Vel().dir()-CVector(OUR_GOAL_CENTER - opp.Pos()).dir()));
	//double angle_oppVel_opp2ball = fabs(Utils::Normalize(opp.Vel().dir()-CVector(ball.Pos() - opp.Pos()).dir()));
//	double angle_ballVel_ball2goal = fabs(Utils::Normalize(ball.Vel().dir()-CVector(OUR_GOAL_CENTER - ball.Pos()).dir()));
	//double angle_ballVel_oppVel = fabs(Utils::Normalize(opp.Vel().dir() - ball.Vel().dir()));
	//double angle_opp2goal_ball2goal = fabs(Utils::Normalize(CVector(OUR_GOAL_CENTER - ball.Pos()).dir()-CVector(OUR_GOAL_CENTER - opp.Pos()).dir()));
	
	if (ballVel.mod() > OPP_PASS_VEL && ballVel.mod() < 600)//�����ڴ����ٶȺ���С�����ٶ�֮��
	{
		CGeoLine ballVelLine = CGeoLine(ball.Pos(),ball.Vel().dir());
		CGeoLineLineIntersection interPoint = CGeoLineLineIntersection(ballVelLine,OUR_GOAL_LINE); 
		bool ballVelToOurGoal = false;
		if (interPoint.Intersectant())
		{
			if (interPoint.IntersectPoint().dist(OUR_GOAL_CENTER) < Param::Field::PITCH_WIDTH/2.0)//���޸ģ���
			{
				ballVelToOurGoal = true;
			}
		}
		if (/*angle_ballVel_ball2goal < Param::Math::PI * 20 / 180 ||*/ ballVelToOurGoal)//���ǳ����������߹�ȥ��,Ҳ���Կ��������������ҵ��ߵĽ��� �� goal��ľ����ж�
		{
			double angle_oppDir_ball2goal = fabs(Utils::Normalize(opp.Dir()-CVector(OUR_GOAL_CENTER - ball.Pos()).dir()));
			double angle_oppVel_opp2goal = fabs(Utils::Normalize(opp.Vel().dir()-CVector(OUR_GOAL_CENTER - opp.Pos()).dir()));
			if (angle_opp2ball_opp2goal < Param::Math::PI * 60 / 180)
			{
				double angle_oppVel_opp2ball = fabs(Utils::Normalize(opp.Vel().dir()-CVector(ball.Pos() - opp.Pos()).dir()));
				//���š��򡢶��־�����immortalKick��λ��
				if (opp.Vel().mod() >= 15 
					&& angle_oppDir_ball2goal < Param::Math::PI * 30 / 180
					&& (angle_oppVel_opp2ball < Param::Math::PI * 50 / 180 || angle_oppVel_opp2goal < Param::Math::PI * 60 / 180)
					&& opp.Pos().dist(ball.Pos()) < 150)
				{
					//ok
					tempValue = 800 - opp.Pos().dist(ball.Pos());
				}
			}
			else if (angle_opp2ball_opp2goal < Param::Math::PI * 150 / 180)//�������������һ��
			{				
				if (ball.Pos().dist(opp.Pos()) < 35 && angle_oppDir_ball2goal < Param::Math::PI * 35 / 180)
				{
					//ok
					tempValue = 750 - opp.Pos().dist(ball.Pos());
				}
			}
			else
			{//�൱�ڸմ�����������,�˷����մ�������һ˲��receiver��Ʊ
				//double angle_ballVel_ball2opp = fabs(Utils::Normalize(ball.Vel().dir()-CVector(opp.Pos() - ball.Pos()).dir()));
				if (ball.Pos().dist(opp.Pos()) < 150 /*&& angle_ballVel_ball2opp < Param::Math::PI * 60 / 180*/
					&& angle_oppDir_ball2goal < Param::Math::PI * 30 / 180)
				{
					//ok
					tempValue = 700 - opp.Pos().dist(ball.Pos());
				}
			}
		}
	}else if(ballVel.mod() <= OPP_PASS_VEL){
		//�ձ��ӵĻ���immortalKick: TODO
	}
	//tempValue = 1000 - opp.dist.ball
	setValue(tempValue);
}
