#include "TandemPos.h"
#include "Global.h"
#include "utils.h"
#include "TaskMediator.h"
#include "BestPlayer.h"

namespace{
	CGeoPoint ourgoal;
	CGeoPoint theirgoal;
	CGeoPoint ourLeftPost;
	CGeoPoint ourRightPost;
}

CTandemPos::CTandemPos(){
	_tandemPos =CGeoPoint(0,0);
	ourgoal = CGeoPoint(-Param::Field::PITCH_LENGTH/2,0);
	theirgoal = CGeoPoint(Param::Field::PITCH_LENGTH/2,0);
	ourLeftPost = CGeoPoint(-Param::Field::PITCH_LENGTH/2,-Param::Field::GOAL_WIDTH/2);
	ourRightPost = CGeoPoint(-Param::Field::PITCH_LENGTH/2,Param::Field::GOAL_WIDTH/2);
}

CTandemPos::~CTandemPos(){
}

void CTandemPos::generatePos(){
	CVisionModule* pVision = vision;
	const MobileVisionT& ball = pVision->Ball();
	const CVector ball2ourgoal = ourgoal - ball.Pos();
	CGeoPoint defaultTandemPos = ball.Pos()  + Utils::Polar2Vector(70.0*Param::Field::RATIO,ball2ourgoal.dir());
	int teamnum = TaskMediator::Instance()->advancer();
	int oppnum = BestPlayer::Instance()->getTheirBestPlayer();
	if (teamnum == 0){
		cout<<"error: no advancer!!!"<<endl;
	}
	if (oppnum!=0){
		analyzeSituation(teamnum,oppnum);
		TandemState tandemstate = getState();
		switch(tandemstate){
			case BallMovingtoTheirHalf:
			{
				planAssistAttack(teamnum,oppnum);
				break;
			}
			case BallMovingtoOurHalf:
			case OurChance:
			{
				planAssistGetBall(teamnum,oppnum);
				break;
		     }
			case TheirChance:
			case Special:
			{
				planBlock(teamnum,oppnum);
				break;
			}
			default:
			{
				planBlock(teamnum,oppnum);
				break;
			}
		}
	}else{
		_tandemPos = defaultTandemPos;
	}
}

void CTandemPos::analyzeSituation(int teamnum,int oppnum){
	// 在球后方60度区域内搜索一个好的接应位置
	CVisionModule* pVision = vision;
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& ourAggressor = pVision->OurPlayer(teamnum);
	const PlayerVisionT& theirAggressor = pVision->TheirPlayer(oppnum);

	const CVector ball2teammate = ourAggressor.Pos() - ball.Pos();
	const CVector ball2teammateUnit = ball2teammate/ball2teammate.mod();
	const CVector ball2opp = theirAggressor.Pos() - ball.Pos();
	const CVector ball2oppUnit = ball2opp/ball2opp.mod();

	bool find_situation = false;
	if ( ball.Vel().mod() > 50*Param::Field::RATIO ){
		double diff_ballMoving2opp = Utils::Normalize(ball2opp.dir() - ball.Vel().dir());
		CGeoLine ballMovingLine(ball.Pos(), ball.Vel().dir());
		CGeoPoint oppProj = ballMovingLine.projection(theirAggressor.Pos());
		double oppProjDist = oppProj.dist(theirAggressor.Pos());
		bool intercpet_by_opp = abs(diff_ballMoving2opp) < Param::Math::PI/2 && oppProjDist < 40;
		if ( abs(ball.Vel().dir()) < Param::Math::PI/4 && !intercpet_by_opp ){
			// 球向对方半场滚去
			setState(BallMovingtoTheirHalf);
			cout<<"ballmovingtotheirhalf"<<endl;
			find_situation = true;
		}
		else if ( abs(ball.Vel().dir()) > 3*Param::Math::PI/4 && !intercpet_by_opp ){
			setState(BallMovingtoOurHalf);
			cout<<"ballmovingtoouthalf"<<endl;
			find_situation = true;
		}
	}

	double ballMovingEffectUs = Utils::VectorDot(ball2teammateUnit, ball.Vel())/4;
	double ballMovingEffetThem = Utils::VectorDot(ball2oppUnit, ball.Vel())/4;
	double ballDistUs = ball2teammate.mod() - ballMovingEffectUs;
	double ballDistThem = ball2opp.mod() - ballMovingEffetThem ;

	if ( !find_situation ){
		if ( ballDistUs < ballDistThem - Param::Vehicle::V2::PLAYER_SIZE*1.1 && ball.X() > -Param::Field::PITCH_LENGTH/3 ){
			setState(OurChance);
			cout<<"our chance"<<endl;
		}else if ( ballDistThem < ballDistUs - 9 || ballDistUs > 40 ){
			setState(TheirChance);
			cout<<"their chance"<<endl;
		}else{
			setState(Special);
			cout<<"special"<<endl;
		}
	}
}

void CTandemPos::planAssistAttack(int teamnum,int oppnum){
	CVisionModule* pVision = vision;
	CGeoPoint teamPos = pVision->OurPlayer(teamnum).Pos();
	double tandemDir = (theirgoal - teamPos).dir();
	if (teamPos.y()>0){
		tandemDir = tandemDir + DefendUtils::calcBlockAngle(ourgoal,teamPos);
	}else{
		tandemDir = tandemDir - DefendUtils::calcBlockAngle(ourgoal,teamPos);
	}
	_tandemPos = teamPos +Utils::Polar2Vector(-50*Param::Field::RATIO,tandemDir);
}

void CTandemPos::planAssistGetBall(int teamnum,int oppnum){
	CVisionModule* pVision = vision;
	const PlayerVisionT& opp = pVision->TheirPlayer(oppnum);
	const PlayerVisionT& teammate = pVision->OurPlayer(teamnum);
	const MobileVisionT& ball =pVision->Ball();
	CVector teammate2ball = ball.Pos() - teammate.Pos();
	CVector ball2opp = opp.Pos() - ball.Pos();
	bool block_opp = false;
	CGeoPoint candidatePos = opp.Pos() + Utils::Polar2Vector(-Param::Field::MAX_PLAYER_SIZE-5, ball2opp.dir());
	CVector ball2candidatePos = candidatePos - ball.Pos();
	double diff_candidate2kickball = Utils::Normalize(ball2candidatePos.dir() - teammate2ball.dir());
	if (candidatePos.dist(ball.Pos())>18*Param::Field::RATIO && abs(diff_candidate2kickball)>Param::Math::PI/3 && abs(teammate.X() - opp.X())<100*Param::Field::RATIO){
		block_opp = true;
		_tandemPos = candidatePos;
	}
	//cout<<"_block_opp "<<block_opp<<endl;
	if (!block_opp){
		double backDist = ball.X() > Param::Field::PITCH_LENGTH/4 ? 80*Param::Field::RATIO : 50*Param::Field::RATIO;
		int sidefactor = 0;
		if (teammate.Y()>0){
			sidefactor = -1;
		}else{
			sidefactor = 1;
		}
		_tandemPos = teammate.Pos()+Utils::Polar2Vector(backDist,Utils::Normalize((theirgoal - teammate.Pos()).dir()+sidefactor*Param::Math::PI/2));
		if (teammate.Y()<20*Param::Field::RATIO && teammate.Y()>-20*Param::Field::RATIO){
			_tandemPos = teammate.Pos() + Utils::Polar2Vector(-50*Param::Field::RATIO,(theirgoal - teammate.Pos()).dir());
		}
		GDebugEngine::Instance()->gui_debug_line(teammate.Pos(),_tandemPos);
	}
}

void CTandemPos::planBlock(int teamnum,int oppnum){
	CVisionModule* pVision = vision;
	const PlayerVisionT& opp = pVision->TheirPlayer(oppnum);
	const PlayerVisionT& teammate = pVision->OurPlayer(teamnum);
	const MobileVisionT& ball =pVision->Ball();
	const CVector opp2ball = ball.Pos() - teammate.Pos();
	const CVector ball2ourgoal = ourgoal - ball.Pos();
	const CVector ball2LeftPost = ourLeftPost - ball.Pos();
	const CVector ball2RightPost = ourRightPost - ball.Pos();
	bool oppdir_in_ourgoal = false;
	if ( ball.Y() > Param::Field::GOAL_WIDTH/2 ){
		oppdir_in_ourgoal = opp2ball.dir() > ball2RightPost.dir() && opp2ball.dir() < ball2LeftPost.dir();
	}
	else if ( ball.Y() < -Param::Field::GOAL_WIDTH/2 ){
		oppdir_in_ourgoal = opp2ball.dir() > ball2RightPost.dir() && opp2ball.dir() < ball2LeftPost.dir();
	}
	else{
		oppdir_in_ourgoal = opp2ball.dir() > 0 && opp2ball.dir() > ball2RightPost.dir()
			|| opp2ball.dir() < 0 && opp2ball.dir() < ball2LeftPost.dir();
	}
	CGeoPoint blockPos;
	if ( oppdir_in_ourgoal )
		blockPos = ball.Pos() + Utils::Polar2Vector(50.0*Param::Field::RATIO, opp2ball.dir());
	else
		blockPos = ball.Pos() + Utils::Polar2Vector(50.0*Param::Field::RATIO, ball2ourgoal.dir());

	if ( Utils::InOurPenaltyArea(blockPos, 5) )
		blockPos = Utils::MakeOutOfOurPenaltyArea(blockPos, 5);

	const PlayerVisionT& leader = pVision->OurPlayer(teamnum);
	const CVector ball2leader = leader.Pos() - ball.Pos();
	double teammate2ballDir = Utils::Normalize(ball2leader.dir() + Param::Math::PI);
	double blockAngle;
	bool block_success = false;
	if ( oppdir_in_ourgoal )
		blockAngle = Utils::Normalize(opp2ball.dir() - ball2leader.dir());
	else
		blockAngle = Utils::Normalize(ball2ourgoal.dir() - ball2leader.dir());
	if ( abs(blockAngle) < Param::Math::PI/12 && ball2leader.mod() < 12*Param::Field::RATIO )
		block_success = true;

	//cout<<"block success:"<<block_success<<", teammate ok:"<<teammate_ok<<endl;

	if ( block_success ){
		double baseDir = oppdir_in_ourgoal ? opp2ball.dir() : ball2ourgoal.dir();
		double pick_dir = baseDir + Utils::Sign(blockAngle)*Param::Math::PI/3;
		blockPos = ball.Pos() + Utils::Polar2Vector(70.0*Param::Field::RATIO, pick_dir);
	}

	// 球在较前场,我可不用跟在拿球者后面
	if ( ball.X() > Param::Field::PITCH_LENGTH/2 - 80*Param::Field::RATIO ){
		const CVector theirgoal2ball = ball.Pos() - theirgoal;
		double ratio = 13/(Param::Field::PITCH_LENGTH/2 - ball.X()) + 5.0/6.0;
		double diff_tandem = min(Param::Math::PI/2, ratio*Param::Math::PI/3);
		double tandem_dir = theirgoal2ball.dir() + Utils::Sign(theirgoal2ball.dir())*diff_tandem;
		blockPos = theirgoal + Utils::Polar2Vector(max(50.0*Param::Field::RATIO, theirgoal2ball.mod()), tandem_dir);
	}
	_tandemPos = blockPos;
}

CGeoPoint CTandemPos::getTandemPos(){
	generatePos();
	return _tandemPos;
}