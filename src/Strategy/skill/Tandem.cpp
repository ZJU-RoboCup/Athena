#include "Tandem.h"
#include <utils.h>
#include <VisionModule.h>
#include "skill/Factory.h"
#include <BestPlayer.h>
#include <GDebugEngine.h>
#include <PlayInterface.h>

namespace{
	CGeoPoint ourGoal;
	CGeoPoint ourLeftPost;
	CGeoPoint ourRightPost;
	CGeoPoint theirGoal;

	bool VERBOSE_MODE = false;
}

CTandem::CTandem()
{
	ourGoal=CGeoPoint(-Param::Field::PITCH_LENGTH/2, 0);
	ourLeftPost=CGeoPoint(-Param::Field::PITCH_LENGTH/2, -Param::Field::GOAL_WIDTH/2);
	ourRightPost=CGeoPoint(-Param::Field::PITCH_LENGTH/2, Param::Field::GOAL_WIDTH/2);
	theirGoal=CGeoPoint(Param::Field::PITCH_LENGTH/2, 0);
	_tandem_role = 0;
	_init = false;
}
CTandem::~CTandem(){}
void CTandem::plan(const CVisionModule* pVision)
{
	static bool is_first = true;
	if (is_first == true)
	{
		cout<<"get into CTandem !"<<endl;
		is_first = false;
	}
	//内部状态进行重置
	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1 )
	{
		setState(BEGINNING);
	}
	const MobileVisionT& ball = pVision->Ball();
	const int _executor = task().executor;
	int flag = task().player.flag;
	const PlayerVisionT& self = pVision->OurPlayer(_executor);
	const CVector self2ball = ball.Pos() - self.Pos();
	const CVector ball2ourgoal = ourGoal - ball.Pos();
	const CGeoPoint defaultTandemPos = ball.Pos() + Utils::Polar2Vector(70.0, ball2ourgoal.dir());
	
	int robberNum = task().ball.receiver;
	if (Utils::PlayerNumValid(robberNum) == false)
	{
		robberNum = 1;
	}

	CGeoPoint drawPos = self.Pos() + Utils::Polar2Vector(18, 3*Param::Math::PI/4);
	if (VERBOSE_MODE)
	{
		GDebugEngine::Instance()->gui_debug_msg(drawPos, "Tandem", COLOR_ORANGE);
	}

	_init = true;
	_called_for_attack = false;
	int opp_num = checkThreatonOpp(pVision);//获取离球最近的对方车
	if ( opp_num > 0 ){
		analyzSituation(pVision, robberNum, opp_num);
		if (VERBOSE_MODE)
		{
			cout<<"opp: "<<opp_num<<endl;
		}

		switch (state()) 
		{
		case BallMoving2TheirHalf: // 选个好的射门位置
			{
				//cout<<"Situation: Assist Attack\n";
				planAssistAttack(pVision, robberNum);
				break; 
			}
		case BallMoving2OurHalf:
		case OurChance:
			{
				//cout<<"Situation: Assist GetBall\n";
				planAssistGetBall(pVision, robberNum, opp_num);
				break;
			}
		case TheirChance:
		case Special:
			{
				//cout<<"Situation: Assist Block\n";
				planBlock(pVision, robberNum, opp_num);
				break;
			}
		default:
			planBlock(pVision, robberNum, opp_num);
			break;
		}
		const PlayerVisionT& leader = pVision->OurPlayer(robberNum);
		const CVector ball2leader = leader.Pos() - ball.Pos();

		// 避让队友的拿球路线
		CGeoSegment shootLine(leader.Pos(), leader.Pos() + Utils::Polar2Vector(-100, ball2leader.dir()));

		// 不能把点算在禁区里
		if ( Utils::InOurPenaltyArea(_myPosition, Param::Vehicle::V2::PLAYER_SIZE) ){
			_myPosition = Utils::MakeOutOfOurPenaltyArea(_myPosition, Param::Vehicle::V2::PLAYER_SIZE);
		}

		TandemStatus::Instance()->setStandByPosition(_myPosition);

		if (VERBOSE_MODE)
		{
			GDebugEngine::Instance()->gui_debug_line(self.Pos(), _myPosition, COLOR_RED);
		}
		double mydir = self2ball.dir();
		if ( state() == OurChance )
			mydir = (theirGoal - self.Pos()).dir();
		setSubTask(PlayerRole::makeItGoto(_executor, _myPosition, mydir, flag | PlayerStatus::DODGE_BALL));
	}
	else{
		setSubTask(PlayerRole::makeItGoto(_executor, defaultTandemPos, self2ball.dir(), flag));
		_myPosition = defaultTandemPos;
	}
	GDebugEngine::Instance()->gui_debug_x(_myPosition,COLOR_WHITE);
	_lastCycle = pVision->Cycle();
	CStatedTask::plan(pVision);
}
int CTandem::checkThreatonOpp(const CVisionModule* pVision)
{
	const MobileVisionT& ball = pVision->Ball();
	int bestOpp = 0;
	if ( ball.Vel().mod() < 40 ){
		double minDist2ball = 30;
		for (int i=1; i<=Param::Field::MAX_PLAYER; i++){
			if ( !pVision->TheirPlayer(i).Valid() )
				continue;

			double tempDist = pVision->TheirPlayer(i).Pos().dist(ball.Pos());
			if ( tempDist < minDist2ball ){
				minDist2ball = tempDist;
				bestOpp = i;
			}
		}
	}
	
	const CBestPlayer::PlayerList& opp_List = BestPlayer::Instance()->theirFastestPlayerToBallList();
	if ( opp_List.size() > 0 ){
		if ( opp_List[0].num != bestOpp && bestOpp > 0 )
			return bestOpp;
		else
			return opp_List[0].num;
	}
	else
		return 0;
}

void CTandem::analyzSituation(const CVisionModule* pVision, int teammate_id, int opp_id)
{
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& ourAggressor = pVision->OurPlayer(teammate_id);
	const PlayerVisionT& theirAggressor = pVision->TheirPlayer(opp_id);

	const CVector ball2teammate = ourAggressor.Pos() - ball.Pos();
	const CVector ball2teammateUnit = ball2teammate/ball2teammate.mod();
	const CVector ball2opp = theirAggressor.Pos() - ball.Pos();
	const CVector ball2oppUnit = ball2opp/ball2opp.mod();

	bool find_situation = false;
	if ( ball.Vel().mod() > 50 ){
		double diff_ballMoving2opp = Utils::Normalize(ball2opp.dir() - ball.Vel().dir());
		CGeoLine ballMovingLine(ball.Pos(), ball.Vel().dir());
		CGeoPoint oppProj = ballMovingLine.projection(theirAggressor.Pos());
		double oppProjDist = oppProj.dist(theirAggressor.Pos());
		bool intercpet_by_opp = abs(diff_ballMoving2opp) < Param::Math::PI/2 && oppProjDist < 40;
		if ( abs(ball.Vel().dir()) < Param::Math::PI/4 && !intercpet_by_opp ){
			// 球向对方半场滚去
			setState(BallMoving2TheirHalf);
			find_situation = true;
		}
		else if ( abs(ball.Vel().dir()) > 3*Param::Math::PI/4 && !intercpet_by_opp ){
			setState(BallMoving2OurHalf);
			find_situation = true;
		} 
	}
	
	double ballMovingEffectUs = Utils::VectorDot(ball2teammateUnit, ball.Vel())/4;
	double ballMovingEffetThem = Utils::VectorDot(ball2oppUnit, ball.Vel())/4;
	double ballDistUs = ball2teammate.mod() - ballMovingEffectUs;
	double ballDistThem = ball2opp.mod() - ballMovingEffetThem ;

	if ( !find_situation ){
		if ( ballDistUs < ballDistThem - Param::Vehicle::V2::PLAYER_SIZE*1.1 && ball.X() > -Param::Field::PITCH_LENGTH/3 )
			setState(OurChance);
		else if ( ballDistThem < ballDistUs - 9 || ballDistUs > 40 )
			setState(TheirChance);
		else
			setState(Special);
	}
}
void CTandem::planAssistAttack(const CVisionModule* pVision, int teammate_id)
{
	// 在球后方60度区域内搜索一个好的接应位置
	const MobileVisionT& ball = pVision->Ball();
	const int _executor = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(_executor);
	const CVector ball2me = me.Pos() - ball.Pos();
	double SEARCH_DIST = 80;
	const int max_search_counts = 5;
	
	const PlayerVisionT& teammate = pVision->OurPlayer(teammate_id);
	const CVector teammate2ball = ball.Pos() - teammate.Pos();
	int side = Utils::Sign(Utils::Normalize(ball2me.dir() - teammate2ball.dir()));
	// 我尽量跑在原来的同一侧
	double dir;
	if ( abs(ball.Y()) > Param::Field::PITCH_WIDTH/3 )
		dir = Utils::Sign(ball.Y())*2*Param::Math::PI/3;
	else
		dir = side*2*Param::Math::PI/3;

	double span_left = Utils::Normalize(dir-Param::Math::PI/4);
	double span_right = Utils::Normalize(dir+Param::Math::PI/4);

	CVector ball2lastPosition = _myPosition - ball.Pos();
	if ( ball2lastPosition.dir() < span_left || ball2lastPosition.dir() > span_right ){
		_myPosition = ball.Pos() + Utils::Polar2Vector(SEARCH_DIST, dir);
		TandemStatus::Instance()->setTandemAgentStatus(TandemStatus::ReadyForPickup);
	}
	else
		TandemStatus::Instance()->setTandemAgentStatus(TandemStatus::StandByOk);
}
void CTandem::planAssistGetBall(const CVisionModule* pVision, int teammate_id, int opp_id)
{
	// 当差
	const MobileVisionT& ball = pVision->Ball();
	CVector ball2theirgoal = theirGoal - ball.Pos();
	const int _executor = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(_executor);
	CVector ball2me = me.Pos() - ball.Pos();
	const PlayerVisionT& opp = pVision->TheirPlayer(opp_id);
	CVector ball2opp = opp.Pos() - ball.Pos();

	const PlayerVisionT& teammate = pVision->OurPlayer(teammate_id);
	const CVector teammate2ball = ball.Pos() -  teammate.Pos();
	double diff_ball2me_teammate2ball = Utils::Normalize(ball2me.dir() - teammate2ball.dir());
	double diff_ball2opp_teammate2ball = Utils::Normalize(ball2opp.dir() - teammate2ball.dir());

	int side = Utils::Sign(Utils::Normalize(ball2me.dir() - ball2theirgoal.dir()));

	CGeoPoint helpPosition;
	if ( TandemStatus::Instance()->getHelpPos(helpPosition) ){
		cout<<"Call for Receive\n";
		_myPosition = helpPosition;
		_called_for_attack = true;
		return;
	}

	// 我只能去档和我在同一边的对手(不能档自己的射门路线)
	bool block_opp = false;
	if ( Utils::Sign(diff_ball2me_teammate2ball) == Utils::Sign(diff_ball2opp_teammate2ball) 
		&& abs(abs(diff_ball2me_teammate2ball) - abs(diff_ball2opp_teammate2ball)) < Param::Math::PI/2 ){
		if ( me.Pos().dist(opp.Pos()) < 100 ){
			// 对手不能离我太远
			CGeoPoint candidatePos = opp.Pos() + Utils::Polar2Vector(-Param::Field::MAX_PLAYER_SIZE-5, ball2opp.dir());
			CVector ball2candidatePos = candidatePos - ball.Pos();
			// 对己方出球路线的阻挡
			double diff_candidate2kickball = Utils::Normalize(ball2candidatePos.dir() - teammate2ball.dir());
			// 第二个条件有待考虑 fix me!
			if ( candidatePos.dist(ball.Pos()) > 1.5*Param::Field::MAX_PLAYER_SIZE && abs(diff_candidate2kickball) > Param::Math::PI/3 ){
				block_opp = true;
				_myPosition = candidatePos;
				TandemStatus::Instance()->setTandemAgentStatus(TandemStatus::TandemGetBall);
			}
		}
	}

	if ( !block_opp ){
		double aheadDist = ball.Vel().mod() > 50 ? ball.Vel().mod()/5 : 0;
		CGeoPoint nextBallPos = ball.Pos() + Utils::Polar2Vector(aheadDist, ball.Vel().dir());

		// 我尽量还是跑在原来那一边(保持一个阵型)
		CVector me2theirGoal = theirGoal - me.Pos();
		CVector teammate2theirGoal = theirGoal - teammate.Pos();
		CVector teammate2me = me.Pos() - teammate.Pos();
		// 选一个最靠近中线的方位
		double bakcDist = ball.X() > Param::Field::PITCH_LENGTH/4 ? 120 : 80;
		if ( abs(me2theirGoal.dir()) < abs(teammate2theirGoal.dir()) ){
			double angleA = abs(Utils::Normalize(me2theirGoal.dir()-teammate2theirGoal.dir()));
			double angleC = Param::Math::PI - Param::Math::PI/3 - angleA;
			double angleSign = Utils::Normalize(teammate2me.dir() - teammate2theirGoal.dir());
			double tanDir = teammate2theirGoal.dir() + Utils::Sign(angleSign)*angleC;
			_myPosition = teammate.Pos() + Utils::Polar2Vector(bakcDist, tanDir);
		} 
		else{
			double angleSign = Utils::Normalize(teammate2me.dir() - teammate2theirGoal.dir());
			double tanDir = teammate2theirGoal.dir() + Utils::Sign(angleSign)*Param::Math::PI/3;
			_myPosition = teammate.Pos() + Utils::Polar2Vector(bakcDist, tanDir);
		}

		if ( me.Pos().dist(_myPosition) < 30 )
			TandemStatus::Instance()->setTandemAgentStatus(TandemStatus::StandByOk);
		else
			TandemStatus::Instance()->setTandemAgentStatus(TandemStatus::TandemGetBall);
	}
}
void CTandem::planBlock(const CVisionModule* pVision, int teammate_id, int opp_id)
{
	const int _executor = task().executor;
	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& threaton_opp = pVision->TheirPlayer(opp_id);
	const CVector opp2ball = ball.Pos() - threaton_opp.Pos();
	const CVector ball2ourgoal = ourGoal - ball.Pos();
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
		blockPos = ball.Pos() + Utils::Polar2Vector(50.0, opp2ball.dir());
	else
		blockPos = ball.Pos() + Utils::Polar2Vector(50.0, ball2ourgoal.dir());

	if ( Utils::InOurPenaltyArea(blockPos, 5) )
		blockPos = Utils::MakeOutOfOurPenaltyArea(blockPos, 5);

	// 检查leader是否已把空档挡住，我就不用档了, 到旁边简陋
	const PlayerVisionT& leader = pVision->OurPlayer(teammate_id);
	const CVector ball2leader = leader.Pos() - ball.Pos();
	double teammate2ballDir = Utils::Normalize(ball2leader.dir() + Param::Math::PI);
	double blockAngle;
	bool block_success = false;
	if ( oppdir_in_ourgoal )
		blockAngle = Utils::Normalize(opp2ball.dir() - ball2leader.dir());
	else
		blockAngle = Utils::Normalize(ball2ourgoal.dir() - ball2leader.dir());
	if ( abs(blockAngle) < Param::Math::PI/12 && ball2leader.mod() < 12 )
		block_success = true;

	//cout<<"block success:"<<block_success<<", teammate ok:"<<teammate_ok<<endl;

	if ( block_success ){
		double baseDir = oppdir_in_ourgoal ? opp2ball.dir() : ball2ourgoal.dir();
		double pick_dir = baseDir + Utils::Sign(blockAngle)*Param::Math::PI/3;
		blockPos = ball.Pos() + Utils::Polar2Vector(70.0, pick_dir);
	}

	/*if ( teammate_ok ){
		double baseDir = oppdir_in_ourgoal ? opp2ball.dir() : ball2ourgoal.dir();
		double recieve_dir = baseDir + 0.5*Utils::Normalize(teammate2ballDir - baseDir);
		blockPos = ball.Pos() + Utils::Polar2Vector(60.0, recieve_dir);
	}*/

	// 球在较前场,我可不用跟在拿球者后面
	if ( ball.X() > Param::Field::PITCH_LENGTH/2 - 80 ){
		const CVector theirgoal2ball = ball.Pos() - theirGoal;
		double ratio = 13/(Param::Field::PITCH_LENGTH/2 - ball.X()) + 5.0/6.0;
		double diff_tandem = min(Param::Math::PI/2, ratio*Param::Math::PI/3);
		double tandem_dir = theirgoal2ball.dir() + Utils::Sign(theirgoal2ball.dir())*diff_tandem;
		blockPos = theirGoal + Utils::Polar2Vector(max(50.0, theirgoal2ball.mod()), tandem_dir);
	}
	const PlayerVisionT& me = pVision->OurPlayer(_executor);

	if ( me.Pos().dist(blockPos) < 30 )
		TandemStatus::Instance()->setTandemAgentStatus(TandemStatus::StandByOk);
	else
		TandemStatus::Instance()->setTandemAgentStatus(TandemStatus::TandemBlocking);

	_myPosition = blockPos;
}
CPlayerCommand* CTandem::execute(const CVisionModule* pVision)
{
	if( subTask() ){
		return subTask()->execute(pVision);
	}
	if( _directCommand ){
		return _directCommand;
	}
	std::cout<<"no command!!!"<<endl;
	return 0;
}

TandemStatus* TandemStatus::Instance()
{
	static TandemStatus* instance = NULL;
	if ( instance == NULL ){
		instance = new TandemStatus;
	}
	return instance;
}
void TandemStatus::callForHelp(const CGeoPoint& helpPos)
{
	_helpPos = helpPos;
	is_helping = true;
}
bool TandemStatus::getHelpPos(CGeoPoint& helpPos)
{
	if ( is_helping ){
		helpPos = _helpPos;
		is_helping = false;
		return true;
	}
	else
		return false;
}