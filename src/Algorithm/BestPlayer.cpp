/// FileName : 		BestPlayer.cpp
/// 				implementation file
/// Description :	Calculate vehicle to ball potential list, important
///	Keywords :		best, potential
/// Organization : 	ZJUNlict@Small Size League
/// Author : 		cliffyin
/// E-mail : 		cliffyin@zju.edu.cn
///					cliffyin007@gmail.com
/// Create Date : 	2011-07-25
/// Modified Date :	2011-07-25 
/// History :
#include <BestPlayer.h>
#include <math.h>
#include <utils.h>
#include <TaskMediator.h>
#include <PlayInterface.h>
#include <RobotSensor.h>
#include <GDebugEngine.h>

//% BALLPLAYER - �����˹�ʹ�ܼ����ĸ���Աȥ�������
//%  ����: 
//%       ourPalyer   - �ҷ���Աλ��(5*2)
//%       otherPlayer - �Է���Աλ��(5*2)
//%       ball        - ���λ��
//%       biases      - ����Ӱ�����صĲ���
//%  ���:
//%       result      - ��ѵ������Ա
//%       Potentials  - �ҷ�5������potential
//%  ��Ҫ���ǵ�����:
//%  biases(0)  -   DistBallToPlayer   ����ľ���,Խ��Խ��
//%  biases(1)  -   shootAngleDiff     ���򵽶Է����ŵĽǶ�
//%                                    ���ԽСԽ��
//%  biases(2)  -   ClearPah           ��ͳ�ֱ���Ƿ��жԷ�
//%                                    ��
//%  biases(3)  -   anglePlayerToBall  ���ķ���ͳ�����ķ����
//%                                    �Ƕ����ԽСԽ��
//%  biases(4)  -   LastBallPlayer     ��һ���ڵ������Ա

//�����������������������

// �ļ������¼
#include <fstream>

namespace {
	/// Running Mode
	bool IS_SIMULATION = true;

	/// Threshold
	bool VERBOSE_MODE = false;									// ���������Ϣ
	const double RobotSize = Param::Vehicle::V2::PLAYER_SIZE;	// С����С����λ������
	const double BallSize = Param::Field::BALL_SIZE;			// �����С����λ������
	const int playNum = Param::Field::MAX_PLAYER;				// 
	const double BALL_CONTROL_BUFFER = 0.1;						// ����Ƶ���л�
	const double Robot_Ability_Diff = 1;						// ������ʾ����˫������ֵ

	/// Both Goalie
	CGeoPoint BallPos(0.0,0.0);
	double MaxToBallDist;
	int goalieNumber = 0;
	int theirGoalie = 0;
	bool theirGoalieChanged = false;
	double theirGoalieRadius = 65;
	const double extralRadiusDiff = 1*Param::Vehicle::V2::PLAYER_SIZE;
	const double theirGoalieRadiusMin = 65;
	const double theirGoalieRadiusMax = theirGoalieRadiusMin + extralRadiusDiff;
	const double theirGoalieRadiusStep = 2;
	CGeoPoint theirGoal;
	/// Potential 
	int biasesNum = 5;
	double generalBiases[] = { 0.7, 0.0, 0.15, 0.0, 0.15 };			// �ۺ����۲����� �޸�֮��
	double attackBiases[] = { 0.7, 0.0, 0.15, 0.0, 0.15 };
	double defenseBiases[]= { 0.7, 0.0, 0.15, 0.0, 0.15 };
	double ballSpeedBias = 0.3;									// ����Ӱ��ϵ��
	const double BaseToBallDist = 50;							// ��С��������򳵾���

	// Evaluate
	const double OurEvaluateAlpha = 0.75;						// �ҷ�ǰ��֡����Ȩ��ֵ
	const double TheirEvaluateAlpha = 0.75;						// �Է�ǰ��֡����Ȩ��ֵ

	// Default
	double OurBallControlDefaultPotential = 0.0;				// �ҷ�Ĭ����Сʹ��ֵ
	double OurPlayerLoseDefualtPotential = 1.0;					// �ҷ�Ĭ�����ʹ��ֵ
	double TheirBallControlDefaultPotential = 0.0;				// �Է�Ĭ����Сʹ��ֵ
	double TheirPlayerLoseDefualtPotential = 1.0;				// �Է�Ĭ�����ʹ��ֵ
		
	/// Print
	const bool RecordOurPotential = false;
	std::string OurPotentialName = "OurPotential.txt";			// �ҷ�ʹ�ܼ�¼�ļ�
	ofstream OurPotentialFile;
	const bool RecordTheirPotential = false;
	std::string TheirPotentialName = "TheirPotential.txt";		// �Է�ʹ�ܼ�¼�ļ�
	ofstream TheirPotentialFile;

	/// GUI���������ʾ
	typedef enum {
		OUR_SIDE = 0,
		THEIR_SIDE = 1,
	} SIDE;

	CVector offside(10,10);
	void drawRobotLabel(const CVisionModule* pVision, int robotNum, string label, SIDE side, char color = COLOR_PURPLE) {
		bool playerNumValid = Utils::PlayerNumValid(robotNum);
		bool playerValid = playerNumValid;
		CGeoPoint playerPos = CGeoPoint(0,0);

		if (playerValid) {
			if (OUR_SIDE == side) {
				playerPos = pVision->OurPlayer(robotNum).Pos();
				playerValid = pVision->OurPlayer(robotNum).Valid();
			} else {
				playerPos = pVision->TheirPlayer(robotNum).Pos();
				playerValid = pVision->TheirPlayer(robotNum).Valid();
			}
		}

		if (playerValid) {
			//GDebugEngine::Instance()->gui_debug_msg(playerPos+offside, label.c_str(), color);
		}

		return ;
	}

	/// ... ...
	
}

int CBestPlayer::getTheirGoalie()
{
	return theirGoalie;
}

CBestPlayer::CBestPlayer()
{
	{
		DECLARE_PARAM_READER_BEGIN(General)
			READ_PARAM(IS_SIMULATION)
		DECLARE_PARAM_READER_END
	}

	_withBall = false;
	_oppWithBall = false;

	
	_ourChangeFlag = false;
	_ourLastBestPlayer = -1;
	_ourChangeCycle = 0;

	_theirChangeFlag = false;
	_theirLastBestPlayer = -1;
	_theirChangeCycle = 0;

	if (RecordOurPotential) {
		OurPotentialFile.open(OurPotentialName.c_str(),ios::out);
	}

	if (RecordTheirPotential) {
		TheirPotentialFile.open(TheirPotentialName.c_str(),ios::out);
	}

	theirGoal = CGeoPoint(Param::Field::PITCH_LENGTH/2.0,0.0);
	memset(_Potentials,0,Param::Field::MAX_PLAYER*7*sizeof(double));
}

void CBestPlayer::update(const CVisionModule *pVision)
{
	/// ��һ����ʹ�ܸ��£�Ŀǰ�򵥵�ֵ���д���
	double *biases;
	biases = generalBiases;				// ͨ�÷�ʽ
	biases = attackBiases;				// ����ģʽ
	biases = defenseBiases;				// ����ģʽ

	/// �ڶ���������˫����ص���Ϣ��ע��˳��
	updateBothGoalie(pVision);				// �����Ž�
	updateBall(pVision);					// ������
	updateOurPlayerList(pVision,biases);	// ���¶�Ա
	updateOurBestPlayer(pVision);			// �����ҷ������Ա

	updateTheirPlayerList(pVision,biases);	// ���¶���
	updateTheirBestPlayer(pVision);			// ���¶Է������Ա
	
	return;
}

void CBestPlayer::updateBothGoalie(const CVisionModule *pVision)
{
	/************************************************************************/
	/* ��һ���������ҷ��Ž���ֱ��ȡxml�����õļ���						*/
	/************************************************************************/
	goalieNumber = PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie());
	/************************************************************************/
	/* �ڶ��������¶Է��Ž����Ƿ��ڽ������������ж�						*/
	/************************************************************************/
	// ���¶Է��Ž������������������뾶����ֹ�ж��Ž���
	int lastTheirGoalie = theirGoalie;
	if (0 == lastTheirGoalie) {
		theirGoalieRadius = theirGoalieRadiusMin;		
	} else {
		theirGoalieRadius += theirGoalieRadiusStep;
		if (theirGoalieRadius >= theirGoalieRadiusMax) {
			theirGoalieRadius = theirGoalieRadiusMax;
		}
	}
	// �ж��Ƿ���Ҫ���¼��㣺�Ž���ʧ�˻��߲�������뾶�����Ľ�����
	bool needReCal = false;
	theirGoalieChanged = false;
	if (0 != lastTheirGoalie) {
		if (! pVision->TheirPlayer(lastTheirGoalie).Valid()) {
			needReCal = true;
		}

		if (theirGoal.dist(pVision->TheirPlayer(lastTheirGoalie).Pos()) > theirGoalieRadius) {
			needReCal = true;
		}
	} else {
		needReCal = true;
	}
	// �������¼��㣺�Ƿ�������뾶�����Ľ�����
	if (needReCal) {
		for (int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
			if (! pVision->TheirPlayer(i).Valid()) {
				continue;
			}
			// ֻ���ǺͶԷ��������ĵľ��룬��Ҫ�Ľ�
			if (theirGoal.dist(pVision->TheirPlayer(i).Pos()) <= theirGoalieRadius) {
				if (i != theirGoalie) {
					theirGoalieChanged = true;
				}
				theirGoalie = i;
				break;
			}
		}
	}
	// ������ʾ
	if (Utils::PlayerNumValid(theirGoalie)) {
		if (theirGoalieChanged) {
			drawRobotLabel(pVision, theirGoalie, "TheirGoalie, Changed!!!", THEIR_SIDE, COLOR_RED);
		} else {
			drawRobotLabel(pVision, theirGoalie, "TheirGoalie", THEIR_SIDE);
		}
	}	

	return ;	
}

void CBestPlayer::updateBall(const CVisionModule* pVision)
{
	/************************************************************************/
	/* ��һ����������ǰ��λ��								           */
	/************************************************************************/
	BallPos = pVision->RawBall().Pos();
	for (int i = 1; i <= Param::Field::MAX_PLAYER; i ++) {
		_ourPlayerBallLooseControl[i-1] = false;
		_theirPlayerBallLooseControl[i-1] = false;
		_ourPlayerBallStrictControl[i-1] = false;
		_theirPlayerBallStrictControl[i-1] = false;
	}

	/************************************************************************/
	/* �ڶ�������ȡ������ڵ� MAXDIST ��Ϊ��һ���ľ��������Ž�����    */
	/************************************************************************/
	MaxToBallDist = BaseToBallDist;
	for (int i = 1; i <= Param::Field::MAX_PLAYER; i ++) {
		if (pVision->OurPlayer(i).Valid() && i != goalieNumber) {
			CVector playerToBall = BallPos- pVision->OurPlayer(i).RawPos();
			double distPlayerToBall = playerToBall.mod();
			if (distPlayerToBall > MaxToBallDist) {
				MaxToBallDist = distPlayerToBall;
			}
		}
		if (pVision->TheirPlayer(i).Valid() && i != theirGoalie) {
			CVector oppToBall = BallPos- pVision->TheirPlayer(i).Pos();
			double distPlayerToBall = oppToBall.mod();
			if (distPlayerToBall > MaxToBallDist) {
				MaxToBallDist = distPlayerToBall;
			}
		}
	}

	/************************************************************************/
	/* �����������µ�ǰ��Ա�Ŀ������������ͼ���Լ����⣨ʵ�	    */
	/************************************************************************/
	// �˲�����Ҫ��һ������
	bool isOurLooseCtrlBall = false;
	bool isOurStrictCtrlBall = false;
	double OurLooseBallCtrlDist = Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE + 3.5;
	double OurLooseBallCtrlAngle = Param::Math::PI*15/180;
	double OurStrictBallCtrlDist = Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE + 1.5;
	double OurStrictBallCtrlAngle = Param::Math::PI*10/180;
	for (int i = 1; i <= Param::Field::MAX_PLAYER; i ++) {
		bool ourPlayerValid = pVision->OurPlayer(i).Valid();
		if (! ourPlayerValid) {
			_ourPlayerBallLooseControl[i-1] = false;
			_ourPlayerBallStrictControl[i-1] = false;
			continue ;
		}

		CVector player2Ball = BallPos - pVision->OurPlayer(i).Pos();
		double distPlayer2Ball = player2Ball.mod();
		double dAnglePlayer2Ball = fabs(Utils::Normalize(player2Ball.dir() - pVision->OurPlayer(i).Dir()));

		bool ballLooseCtrl = (distPlayer2Ball < OurLooseBallCtrlDist && dAnglePlayer2Ball < OurLooseBallCtrlAngle);
		if (! IS_SIMULATION) {
			ballLooseCtrl = ballLooseCtrl || RobotSensor::Instance()->IsInfraredOn(i);
		}

		bool ballStrictCtrl = (distPlayer2Ball < OurStrictBallCtrlDist && dAnglePlayer2Ball < OurStrictBallCtrlAngle);
		if (! IS_SIMULATION) {
			ballStrictCtrl = ballStrictCtrl || RobotSensor::Instance()->IsInfraredOn(i);
		}
		
		if (ballLooseCtrl) {
			_ourPlayerBallLooseControl[i-1] = true;
			isOurLooseCtrlBall = true;
		} else {
			_ourPlayerBallLooseControl[i-1] = false;
		}

		if (ballStrictCtrl) {
			_ourPlayerBallStrictControl[i-1] = true;
			isOurStrictCtrlBall = true;
		} else {
			_ourPlayerBallStrictControl[i-1] = false;
		}
	}

	bool isTheirLooseCtrlBall = false;
	bool isTheirStrictCtrlBall = false;
	double TheirLooseBallCtrlDist = Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE + 15.5;
	double TheirLooseBallCtrlAngle = Param::Math::PI*45/180;
	double TheirStrictBallCtrlDist = Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE + 8.5;
	double TheirStrictBallCtrlAngle = Param::Math::PI*30/180;
	for (int i = 1; i <= Param::Field::MAX_PLAYER; i ++) {
		bool theirPlayerValid = pVision->TheirPlayer(i).Valid();
		if (! theirPlayerValid) {
			_theirPlayerBallLooseControl[i-1] = false;
			_theirPlayerBallStrictControl[i-1] = false;
			continue ;
		}

		CVector player2Ball = BallPos - pVision->TheirPlayer(i).Pos();
		double distPlayer2Ball = player2Ball.mod();
		double dAnglePlayer2Ball = fabs(Utils::Normalize(player2Ball.dir() - pVision->TheirPlayer(i).Dir()));
		//double dAnglePlayer2Ball = 0.0;

		bool ballLooseCtrl = (distPlayer2Ball < TheirLooseBallCtrlDist && dAnglePlayer2Ball < TheirLooseBallCtrlAngle);

		bool ballStrictCtrl = (distPlayer2Ball < TheirStrictBallCtrlDist && dAnglePlayer2Ball < TheirStrictBallCtrlAngle);

		if (ballLooseCtrl) {
			_theirPlayerBallLooseControl[i-1] = true;
			isTheirLooseCtrlBall = true;
		} else {
			_theirPlayerBallLooseControl[i-1] = false;
		}

		if (ballStrictCtrl) {
			_theirPlayerBallStrictControl[i-1] = true;
			isTheirStrictCtrlBall = true;
		} else {
			_theirPlayerBallStrictControl[i-1] = false;
		}
	}

	/************************************************************************/
	/* ���Ĳ������¶���Ŀ������										    */
	/************************************************************************/
	if (isOurStrictCtrlBall) {
		_withBall = true;
		_oppWithBall = false;
	} else {
		if (isTheirStrictCtrlBall) {
			_withBall = false;
			_oppWithBall = true;
		} else if (isOurLooseCtrlBall && !isTheirLooseCtrlBall) {
			_withBall = true;
			_oppWithBall = false;
		} else {
			_withBall = false;
			_oppWithBall = false;
		}
	}

	bool ourListEmpty = _ourFastestPlayerToBallList.empty();
	bool theirListEmpty = _theirFastestPlayerToBallList.empty();
	if (!ourListEmpty && theirListEmpty) {
		_withBall = true;
	} 
	if (ourListEmpty && !theirListEmpty) {
		_oppWithBall = true;
	}

	return ;
}

void CBestPlayer::updateOurPlayerList(const CVisionModule *pVision,const double* biases)
{	
	/************************************************************************/
	/* ��һ�������ݣ���ȡ�ϴν�����ҷ��Ž���������С����Ϊ�����		*/
	/************************************************************************/
	// a. ���ܿ���
	double _Last_Potentials[Param::Field::MAX_PLAYER][7];
	memset(_Last_Potentials,0,Param::Field::MAX_PLAYER*7*sizeof(double));
	memcpy(_Last_Potentials,_Potentials,Param::Field::MAX_PLAYER*7*sizeof(double));
	// b. ���п���
	PlayerList _ourLastFastestPlayerToBallList;
	_ourLastFastestPlayerToBallList.clear();
	_ourLastFastestPlayerToBallList.assign(_ourFastestPlayerToBallList.begin(),_ourFastestPlayerToBallList.end());
	// c. �жϵ�ǰͼ������ʷ���Ŷ�Ա
	int old_fastest = 0;
	int old_fastest_locate = 0;
	int currentBallhandler = TaskMediator::Instance()->ballHandler();
	for (PlayerList::iterator it = _ourLastFastestPlayerToBallList.begin(); it != _ourLastFastestPlayerToBallList.end(); it ++) {
		int tempFastNum = it->num;
		old_fastest_locate ++;
		// �������һ �� ��Ա���Ϸ� ���� �Ž�
		if (! Utils::PlayerNumValid(tempFastNum) || tempFastNum == goalieNumber) {
			continue;
		}
		// ��������� �� ��Ա�����ڿ�����
		if (! pVision->OurPlayer(tempFastNum).Valid()) {
			continue;
		}

		// ����Ϊ��Ѵ�����Ķ�Ա
		old_fastest = tempFastNum;
		break;
	}

	/************************************************************************/
	/* �ڶ������������ܼ��㣬�ۺϿ�����ʷ��¼���						*/
	/************************************************************************/
	// ��ճ�ʼ��
	memset(_Potentials,0,Param::Field::MAX_PLAYER*7*sizeof(double));
	_ourFastestPlayerToBallList.clear();

	// ���и���
	double antiShootDir=Utils::Normalize((theirGoal-pVision->Ball().RawPos()).dir()+Param::Math::PI);
	CGeoPoint ball=pVision->Ball().RawPos();
	for (int i = 1; i <= Param::Field::MAX_PLAYER; ++ i) {
		if (pVision->OurPlayer(i).Valid()) {
			// ����Potentials
			//CGeoPoint predictBallPos=BallSpeedModel::Instance()->posForTime(15,pVision);
			CGeoPoint _ourPlayer = pVision->OurPlayer(i).RawPos();
			CVector playerToBall = ball - _ourPlayer;
			//if (pVision->Ball().Vel().mod()>80){
			//	playerToBall=predictBallPos-_ourPlayer;
			//}
			double DistPlayerToBall = playerToBall.mod(); //c  0~1
			CGeoPoint theirGoal=CGeoPoint(Param::Field::PITCH_LENGTH/2,0);
			double kickDir=(ball-theirGoal).dir();
			CGeoPoint kickPos=ball+Utils::Polar2Vector(8,kickDir);
			CVector playerToKickPos=kickPos-_ourPlayer;
			// �������������������أ�����������˵��
			if (biases[0] != 0) {
				_Potentials[i-1][1] = distBallToPlayer(playerToKickPos,biases,MaxToBallDist);
				_Potentials[i-1][5] = velBallToPlayer(pVision->OurPlayer(i),pVision->Ball(),biases,MaxToBallDist);
				if(VERBOSE_MODE) cout << "Player: " << i  <<  "	"<<" "<<playerToBall.mod()<<" "<<_Potentials[i-1][1] <<"  "<< 	_Potentials[i-1][5]  << endl;
				//if (_Potentials[i-1][5]+_Potentials[i-1][1] < 0) {
				//	_Potentials[i-1][5] = -_Potentials[i-1][1];//��֤���ﲻ���ָ�ֵ
				//}
			}
			if (biases[1] != 0) {
				_Potentials[i-1][2] = 0;//shootAngle(_ourPlayer,BallPos,biases,true);
			}
			if (biases[2] != 0) {
				_Potentials[i-1][3] = 0;//clearPath(pVision,_ourPlayer,BallPos,biases);
			}
			if (biases[3] != 0) {
				_Potentials[i-1][4] = 0;//anglePlayerToBall(pVision->OurPlayer(i),BallPos,biases);
			}
			//[1]distBallToPlayer
			//[2]shootAngle
			//[3]clearPath
			//[4]anglePlayerToBall
			//[5]velBallToPlayer
			// �ۺϼ������
			double distFactor=1,velFactor=5;
			//if (pVision->Ball().Vel().mod()>250&&pVision->Ball().Valid())
			//{
			//	distFactor=0.9;velFactor=1.1;
			//}
			_Potentials[i-1][6] = distFactor*_Potentials[i-1][1]+velFactor*_Potentials[i-1][5];
			_Potentials[i-1][0] = _Potentials[i-1][6];

			// ����ͬ�ϴ�һ�£�������н���ϵ������
			if (i == _ourLastBestPlayer) {
				//double multiAlpha = 0.8 + 0.05 * old_fastest_locate;
				_Potentials[i-1][0] -= 0.12;
			}

			if (i != goalieNumber) {
				CGeoPoint me=_ourPlayer+Utils::Polar2Vector(8,playerToBall.dir());
				CGeoSegment metoBallSeg=CGeoSegment(me,ball);
				for (int j=1;j<=Param::Field::MAX_PLAYER;j++)
				{
					if (pVision->OurPlayer(j).Valid()&&j!=i){
						CGeoPoint proj=metoBallSeg.projection(pVision->OurPlayer(j).RawPos());
						double dist = proj.dist(pVision->OurPlayer(j).RawPos());
						double distMe = proj.dist(me);
						double metoBlockerDist=proj.dist(me);
						if (metoBallSeg.IsPointOnLineOnSegment(proj)&&dist<25&&metoBlockerDist<80&&distMe>Param::Vehicle::V2::PLAYER_SIZE*2){
 							_Potentials[i-1][0] +=0.3;
						}
					}
				}

				// TODO ���Ǻ�׼ȷ��
/*				if (_ourPlayerBallStrictControl[i-1]) {
					_Potentials[i-1][0] = OurBallControlDefaultPotential;
				} else if (_ourPlayerBallLooseControl[i-1]) {
					_Potentials[i-1][0] = OurBallControlDefaultPotential + 0.1;
				}	*/			
			} else { // �Ž��Ŀ�����Ҫ����
				_Potentials[i-1][0] = 1.5*_Potentials[i-1][0] + 1;
			}			
		} else {
			_Potentials[i-1][0] = OurPlayerLoseDefualtPotential;
		}

		// �ۺ�ǰ�����εļ������ͳһ
		double current_potential = OurEvaluateAlpha * _Potentials[i-1][0] + (1.0-OurEvaluateAlpha) * _Last_Potentials[i-1][0];
		
		//if (current_potential >= OurPlayerLoseDefualtPotential) {
		//	current_potential = OurPlayerLoseDefualtPotential;
		//} else if (current_potential <= OurBallControlDefaultPotential) {
		//	current_potential = OurBallControlDefaultPotential;
		//}

		if (!pVision->OurPlayer(i).Valid()){
			current_potential=current_potential+1;
		}

		// ���Ե�ֱ�Ӵ�����Ĵ������Ľ� cliffyin
		//if (currentBallhandler == i) {
		//	current_potential *= 0.5;
		//}
		if(VERBOSE_MODE) cout << "CurPotential: " << i  <<  "	"<<current_potential<< endl;
		_Potentials[i-1][0] = current_potential;
	}

	//patch
	//if (pVision->Ball().Vel().mod()<120)
	//{
	//	int tempBestPlayer=NormalPlayUtils::getOurMostClosetoPos(pVision,BallPos);
	//	int patcherOne=NormalPlayUtils::patchForBestPlayer(pVision,_ourLastBestPlayer);
	//	if (patcherOne!=0&&tempBestPlayer!=patcherOne){
	//		CGeoPoint theirGoal=CGeoPoint(Param::Field::PITCH_LENGTH/2,0);
	//		double lasttoGoalDir=(theirGoal-pVision->OurPlayer(tempBestPlayer).RawPos()).dir();
	//		double lasttoBallDir=(BallPos-pVision->OurPlayer(tempBestPlayer).RawPos()).dir();
	//		double lastDiff=fabs(Utils::Normalize(lasttoBallDir-0));

	//		double patchertoGoalDir=(theirGoal-pVision->OurPlayer(patcherOne).RawPos()).dir();
	//		double patchertoBallDir=(BallPos-pVision->OurPlayer(patcherOne).RawPos()).dir();
	//		double patcherDiff=fabs(Utils::Normalize(patchertoBallDir-0));
	//		if (lastDiff-patcherDiff>Param::Math::PI/3){
	//			tempBestPlayer=patcherOne;
	//		}		
	//	}
	//	_Potentials[tempBestPlayer-1][0]=0;
	//}


	for (int i = 1; i <= Param::Field::MAX_PLAYER; ++ i) {
		_ourFastestPlayerToBallList.push_back(ToBallPotentialPair(i, getOurPotential(i)));
	}

	/************************************************************************/
	/* ʹ�ܼ���������													*/
	/************************************************************************/
	
	std::sort(_ourFastestPlayerToBallList.begin(), _ourFastestPlayerToBallList.end());

	if (RecordOurPotential) {
		if (OurPotentialFile.is_open()) {
			for (PlayerList::iterator it = _ourLastFastestPlayerToBallList.begin(); it != _ourLastFastestPlayerToBallList.end(); it ++) {
				OurPotentialFile<<setprecision(8)<<it->potential<<'\t';
			}
			OurPotentialFile<<endl;
		}		
	}

	if (! _ourFastestPlayerToBallList.empty()) {
		int curOurFastestPlayer = _ourFastestPlayerToBallList[0].num;
		drawRobotLabel(pVision, curOurFastestPlayer, "OurFast", OUR_SIDE);
	}

	return ;
}


void CBestPlayer::updateOurBestPlayer(const CVisionModule* pVision)
{
	// �����жϵ�ǰ�������Ա ���� �Ƿ��б仯
	//const PlayerVisionT me=pVision->OurPlayer(_ourLastBestPlayer);
	//const MobileVisionT ball=pVision->Ball();
	int cur_our_best_player = _ourFastestPlayerToBallList[0].num;
	bool cur_our_best_player_changed = false;
	//cout<<cur_our_best_player<<" "<<TaskMediator::Instance()->leftBack()<<" "<<TaskMediator::Instance()->rightBack()<<" "<<TaskMediator::Instance()->defendMiddle()<<endl;
	if (cur_our_best_player==TaskMediator::Instance()->leftBack() || cur_our_best_player==TaskMediator::Instance()->rightBack()){
		if (TaskMediator::Instance()->defendMiddle()!=0){
			double defendMiddleValue=getOurPotential(TaskMediator::Instance()->defendMiddle());
			if (defendMiddleValue-_ourFastestPlayerToBallList[0].potential<0.1){
				cur_our_best_player=TaskMediator::Instance()->defendMiddle();
			}
		}
		//cout<<"bestPlayer"<<cur_our_best_player<<endl;
	}

	if (Utils::PlayerNumValid(cur_our_best_player) && cur_our_best_player != _ourLastBestPlayer) {
		cur_our_best_player_changed = true;
		_ourLastBestPlayer = cur_our_best_player;
	}

	if (cur_our_best_player_changed) {
		_ourChangeFlag = true;
		_ourChangeCycle = pVision->Cycle();
	} else {
		if (_ourChangeFlag) {
			if (pVision->Cycle() - _ourChangeCycle > 0.1*Param::Vision::FRAME_RATE) {
				_ourChangeFlag = false;
			}
		}
	}

	return ;
}

void CBestPlayer::updateTheirPlayerList(const CVisionModule *pVision,const double *biases)
{
	/************************************************************************/
	/* ��һ�� ���ݣ� ��ȡ���ϴθ��½���г��ҷ��Ž�����������С����Ϊ����� */
	/************************************************************************/
	// a. ���ܿ���
	//const MobileVisionT& ball = pVision->Ball();
	double _Last_theirPotentials[Param::Field::MAX_PLAYER][6];
	memset(_Last_theirPotentials,0,Param::Field::MAX_PLAYER*6*sizeof(double));
	memcpy(_Last_theirPotentials,_theirPotentials,Param::Field::MAX_PLAYER*6*sizeof(double));
	// b. �жϵ�ǰͼ������ʷ���Ŷ�Ա
	PlayerList _theirLastFastestPlayerToBallList;
	_theirLastFastestPlayerToBallList.clear();
	_theirLastFastestPlayerToBallList.assign(_theirFastestPlayerToBallList.begin(),_theirFastestPlayerToBallList.end());
	int old_their_fastest = 0;
	int old_their_fastest_locate = 0;
	for (PlayerList::iterator it = _theirLastFastestPlayerToBallList.begin(); it != _theirLastFastestPlayerToBallList.end(); it ++) {
		int tempFastNum = it->num;
		old_their_fastest_locate ++;
		// �������һ �� ��Ա���Ϸ� ���� �Ž�
		if (! Utils::PlayerNumValid(tempFastNum) || tempFastNum == theirGoalie) {
			continue;
		}
		// ��������� �� ��Ա�����ڿ�����
		if (! pVision->TheirPlayer(tempFastNum).Valid()) {
			continue;
		}

		old_their_fastest = tempFastNum;
		break;
	}

	double tempBiases[5];
	for (int i = 0; i < 5; i ++) {
		tempBiases[i] = biases[i];
	}
	
	/************************************************************************/
	/* �ڶ������������ܼ��㣬�ۺϿ�����ʷ��¼���                     */
	/************************************************************************/
	// ��ճ�ʼ��
	memset(_theirPotentials,0,Param::Field::MAX_PLAYER*6*sizeof(double));
	_theirFastestPlayerToBallList.clear(); // ����б�
	// ���и���
	for (int i = 1; i <= Param::Field::MAX_PLAYER; ++ i) {
		if (pVision->TheirPlayer(i).Valid()) {
			//����Potentials
			CGeoPoint _theirPlayer = pVision->TheirPlayer(i).Pos();
			CVector playerToBall = BallPos- _theirPlayer;
			double diff_dribble_ball = Utils::Normalize(playerToBall.dir() - pVision->TheirPlayer(i).Dir());
			double DistPlayerToBall = playerToBall.mod();//c  0~1

			if (tempBiases[0] != 0) {
				_theirPotentials[i-1][1] = Robot_Ability_Diff*distBallToPlayer(playerToBall,tempBiases,MaxToBallDist);
				_theirPotentials[i-1][5] = Robot_Ability_Diff*velBallToPlayer(pVision->TheirPlayer(i),pVision->Ball(),tempBiases,MaxToBallDist);

				//cout << "Player: " << i  <<  " "<<"ballMovingCost: " << 	_theirPotentials[i-1][5] << " "<<"distcost: "<<_theirPotentials[i-1][1] <<endl;
				/*if (_theirPotentials[i-1][1]+_theirPotentials[i-1][5] < 0) {
					_theirPotentials[i-1][5] = - _theirPotentials[i-1][1];
				}*/
			}
			if (tempBiases[1] != 0) {
				_theirPotentials[i-1][2] = 0;//shootAngle(_theirPlayer,BallPos,tempBiases,false); //�Է������ź����ǵ��෴����û����
			}
			if (tempBiases[2] != 0) {
				_theirPotentials[i-1][3] = 0;//theirClearPath(pVision,_theirPlayer,BallPos,tempBiases);
			}
			if (tempBiases[3] != 0) {
				_theirPotentials[i-1][4] = 0;//anglePlayerToBall(pVision->TheirPlayer(i),BallPos,tempBiases);

			}
			_theirPotentials[i-1][0] = _theirPotentials[i-1][1]+_theirPotentials[i-1][2]+_theirPotentials[i-1][3]+_theirPotentials[i-1][4]+_theirPotentials[i-1][5];
			
			// ����ͬ�ϴ�һ�£�������н���ϵ������
			if (i == _theirLastBestPlayer) {
				double multiAlpha =  0.8 + 0.05 * old_their_fastest_locate;
				_theirPotentials[i-1][0] *= multiAlpha;
			}

			// TODO ���Ǻ�׼ȷ��
			if (i != theirGoalie) {
				if (_theirPlayerBallStrictControl[i-1]) {
					_theirPotentials[i-1][0] = TheirBallControlDefaultPotential;
				} else if (_theirPlayerBallLooseControl[i-1]) {
					_theirPotentials[i-1][0] = TheirBallControlDefaultPotential + 0.1;
				}				
			} else { // �Ž��Ŀ�����Ҫ����
				_theirPotentials[i-1][0] = 1.5*_theirPotentials[i-1][0] + 0.2;
			}		
		} else {
			_theirPotentials[i-1][0] = TheirPlayerLoseDefualtPotential;
		}

		double current_their_potential = TheirEvaluateAlpha * _theirPotentials[i-1][0] + (1.0-TheirEvaluateAlpha) * _Last_theirPotentials[i-1][0];
		//if (current_their_potential >= TheirPlayerLoseDefualtPotential) {
		//	current_their_potential = TheirPlayerLoseDefualtPotential;
		//} else if (current_their_potential <= TheirBallControlDefaultPotential) {
		//	current_their_potential = TheirBallControlDefaultPotential;
		//}

		//add by twj
		if (!pVision->TheirPlayer(i).Valid()){
			current_their_potential=current_their_potential+1;
		}

		_theirPotentials[i-1][0] = current_their_potential;
		_theirFastestPlayerToBallList.push_back(ToBallPotentialPair(i, getTheirPotential(i)));
	}

	/************************************************************************/
	/* ʹ�ܼ���������                                                     */
	/************************************************************************/
	std::sort(_theirFastestPlayerToBallList.begin(),_theirFastestPlayerToBallList.end());

	if (RecordTheirPotential) {
		if (TheirPotentialFile.is_open()) {
			for (PlayerList::iterator it = _theirFastestPlayerToBallList.begin(); it != _theirFastestPlayerToBallList.end(); it ++) {
				TheirPotentialFile<<setprecision(8)<<it->potential<<'\t';
			}
			TheirPotentialFile<<endl;
		}		
	}

	if (! _theirFastestPlayerToBallList.empty()) {
		int curTheirFastestPlayer = _theirFastestPlayerToBallList[0].num;
		drawRobotLabel(pVision, curTheirFastestPlayer, "TheirFast", THEIR_SIDE);
	}

	return;
}

void CBestPlayer::updateTheirBestPlayer(const CVisionModule* pVision)
{
	// �����жϵ�ǰ�������Ա ���� �Ƿ��б仯
	int cur_their_best_player = _theirFastestPlayerToBallList[0].num;
	bool cur_their_best_player_changed = false;
	if (Utils::PlayerNumValid(cur_their_best_player) && cur_their_best_player != _theirLastBestPlayer) {
		cur_their_best_player_changed = true;
		_theirLastBestPlayer = cur_their_best_player;
	}

	if (cur_their_best_player_changed) {
		_theirChangeFlag = true;
		_theirChangeCycle = pVision->Cycle();
	} else {
		if (_theirChangeFlag) {
			if (pVision->Cycle() - _theirChangeCycle > 15) {
				_theirChangeFlag = false;
			}
		}
	}

	return ;
}

// // // // // 
double CBestPlayer::distBallToPlayer(CVector playerToBall,const double *biases,double maxDist)
{
	//��������Ӱ��
	double DistPlayerToBall = playerToBall.mod();			//c  0~1
	return (biases[0] * DistPlayerToBall / (maxDist+1.0));	//--Potential
}

double CBestPlayer::shootAngle(CGeoPoint& _ourPlayer,CGeoPoint& ball,const double *biases,bool flag)
{
	//����Ƕȵ�Ӱ��*�����Ҷ����2���ǹ�ʽ
	int side = flag?1:-1;
	const CGeoPoint GoalCenter(flag*Param::Field::PITCH_LENGTH/2,0); //�����Ų�����Ҫ����
	CVector playerToBall = ball- _ourPlayer;
	double DistPlayerToBall = playerToBall.mod();//c  
	double DistBallToGoal = (GoalCenter-ball).mod();//a
	double DistPlayerToGoal = (GoalCenter-_ourPlayer).mod();//b
	double CosA =  (DistPlayerToGoal*DistPlayerToGoal+DistPlayerToBall*DistPlayerToBall-DistBallToGoal*DistBallToGoal)/(2*DistPlayerToGoal*DistPlayerToBall+2);
	double angleEffect = sqrt((1-CosA)/2);		//  0~1
	return (biases[1] * angleEffect);			//--Potential
}

double CBestPlayer::clearPath(const CVisionModule *pVision,CGeoPoint& _ourPlayer, CGeoPoint& ball,const double *biases)
{
	//�Է���Ա��Ӱ��
	CVector playerToBall = ball- _ourPlayer;
	double DistPlayerToBall = playerToBall.mod();//c  0~1
	double tempPotential=0;
	for (int j=1;j<=playNum;j++)
	if (pVision->TheirPlayer(j).Valid())
	{
		CVector playerToOpp = pVision->TheirPlayer(j).Pos() - _ourPlayer;
		double DistPlayerToOpp = playerToOpp.mod();
		if (DistPlayerToOpp < DistPlayerToBall && std::abs(std::sin(Utils::Normalize(playerToBall.dir()-playerToOpp.dir())))*DistPlayerToOpp<RobotSize)
		{
			double DistParam = 0;
			double AngleParam = 0;
			//Ӱ���赲���صļ�������
			if (DistPlayerToBall < 1.5*RobotSize + 0.5*BallSize)//������ȥ��
			{
				continue;
			}
			else{
				DistParam = RobotSize/(fabs(DistPlayerToBall-RobotSize)+1);//r/(Dist-r),������Ǹ������Եľ�������
			}
			//�г�r��thirt�ĺ���,�ü������ʾ,r=DistPlayerToOpp,������Ǽ��㿪ʼӰ�쳵�˶������ƫ���,������Ϊ��Ϊ������
			double Th = std::asin(RobotSize/DistPlayerToOpp);
			//-Th~ThΪ0��1���캯�����Ա���
			double AngleDiff = Utils::Normalize(playerToOpp.dir()-playerToBall.dir());
			if (AngleDiff > Th || AngleDiff < -Th)
			{
				continue;
			}
			else{
				if (fabs(Th) > 0.000001) {
					AngleParam = std::cos((Param::Math::PI/2/Th)*AngleDiff);//1��0
				} else {
					AngleParam = 0.0;
				}				
			}
			DistParam *= (DistPlayerToBall - DistPlayerToOpp)/(DistPlayerToOpp+1);//1��0
			tempPotential += DistParam*AngleParam;
		}

	}

	if (tempPotential < 0) {
		tempPotential = 0;
	} else if (tempPotential > 1.0) {
		tempPotential = 1.0;
	}

	return biases[2] * tempPotential;
}

double CBestPlayer::theirClearPath(const CVisionModule *pVision,CGeoPoint& _theirPlayer,CGeoPoint& ball,const double *biases)
{
	//�����ҷ���Ա�ԶԷ���Ա��Ӱ��
	CVector playerToBall = ball- _theirPlayer;
	double DistPlayerToBall = playerToBall.mod();//c  0~1
	double tempPotential=0;
	for (int j=1;j<=playNum;j++)
	if(pVision->OurPlayer(j).Valid())
	{
		CVector playerToOpp = pVision->OurPlayer(j).Pos()-_theirPlayer;
		double DistPlayerToOpp = playerToOpp.mod();
		if (DistPlayerToOpp < DistPlayerToBall && std::sin(std::abs(playerToBall.dir()-playerToOpp.dir()))<RobotSize)
		{
			double DistParam = 0;
			double AngleParam = 0;
			//Ӱ���赲���صļ�������
			if (DistPlayerToBall < 1.5*RobotSize + 0.5*BallSize)//������ȥ��
			{
				continue;
			}
			else{
				DistParam = RobotSize/(fabs((DistPlayerToBall-RobotSize))+1);//r/(Dist-r),������Ǹ������Եľ�������
			}
			//�г�r��thirt�ĺ���,�ü������ʾ,r=DistPlayerToOpp,������Ǽ��㿪ʼӰ�쳵�˶������ƫ���,������Ϊ��Ϊ������
			double Th = std::asin(RobotSize/(DistPlayerToOpp+1));
			//-Th~ThΪ0��1���캯�����Ա���
			double AngleDiff = playerToOpp.dir()-playerToBall.dir();
			if (AngleDiff > Th || AngleDiff < -Th)
			{
				continue;
			}
			else{
				if (fabs(Th) > 0.000001) {
					AngleParam = std::cos((Param::Math::PI/2/Th)*AngleDiff);//1��0
				} else {
					AngleParam = 0.0;
				}				
			}
			DistParam *= (DistPlayerToBall - DistPlayerToOpp)/(DistPlayerToOpp+1);//1��0
			tempPotential += DistParam*AngleParam;
		}
	}

	if (tempPotential < 0) {
		tempPotential = 0;
	} else if (tempPotential > 1.0) {
		tempPotential = 1.0;
	}

	return biases[2] * tempPotential;
}

double CBestPlayer::anglePlayerToBall(const PlayerVisionT& player,const CGeoPoint& ball,const double *biases)
{
	CVector playerToBall = ball - player.Pos();
	double angleDiff = fabs(Utils::Normalize(player.Dir()-playerToBall.dir()));
	return biases[3] * sqrt(1.0 - cos(angleDiff/2.0));
}

double CBestPlayer::velBallToPlayer(const PlayerVisionT& player,const MobileVisionT& ball,const double *biases,double maxDist)
{
	/// �������ٴ�СΪ8m/s
	double limited_ball_speed = 800;
	if (! ball.Valid()) {
		limited_ball_speed = 50;
	}
	CVector applicable_ball_vel = ball.Vel();
	if (ball.Vel().mod() > limited_ball_speed) {
		applicable_ball_vel = Utils::Polar2Vector(limited_ball_speed,ball.Vel().dir());
	}

	double maxProjDist=80;
	CGeoLine ballVelLine=CGeoLine(ball.Pos(),ball.Pos()+applicable_ball_vel);
	CGeoPoint proj=ballVelLine.projection(player.Pos());
	double dist=player.Pos().dist(proj);
	if (ball.Vel().mod()<30||!ball.Valid()){
		dist = player.Pos().dist(ball.RawPos());
	}
	dist=min(dist,maxProjDist);
	dist=max(dist,10);

	/// �����򿿽���Ա����Զ����Ա
	const double own_vel_alpha = 0;
	CVector playerToBall = ball.RawPos() - player.RawPos();	
	CVector diffVel = applicable_ball_vel - player.Vel()*own_vel_alpha;
	double angleDiff = Utils::Normalize(diffVel.dir() - playerToBall.dir());
	// ����ֵ
	double ballMovingCost = diffVel.mod() * cos(angleDiff);
	//ballMovingCost += diffVel.mod() * fabs(sin(Param::Math::PI-fabs(angleDiff)));

	
	if (ballMovingCost >= maxDist) {
		ballMovingCost = maxDist;
	} else if (ballMovingCost <= -maxDist) {
		ballMovingCost = -maxDist;
	}

	double ballFactor = (ballMovingCost) / (maxDist+1.0);
	//double finalBallFactor=0.5* ballFactor+0.5;

	double yDistFactor=0.5;
	if (applicable_ball_vel.mod()>600){
		yDistFactor=1.3;
	}else if (applicable_ball_vel.mod()>400){
		yDistFactor=1.1;
	}else if (applicable_ball_vel.mod()>350){
		yDistFactor=1;
	}else if (applicable_ball_vel.mod()>280){
		yDistFactor=0.8;
	}else if (applicable_ball_vel.mod()>220){
		yDistFactor=0.65;
	}else if (applicable_ball_vel.mod()>160){
		yDistFactor=0.45;
	}else if (applicable_ball_vel.mod()>100){
		yDistFactor=0.3;
	}else{
		yDistFactor=0.1;
	}

  double finalBallFactor;
  if (fabs(ball.Vel().dir()) > Param::Math::PI * 100 / 180) {
    double yTime;
    double yVel = (player.Vel().mod() * cos((proj - player.Pos()).dir() - player.Vel().dir()));
    if (yVel > 0)
      yTime = -log(yVel + 1);
    else
      yTime = log(-yVel + 1);
    yTime = yTime / 10.0;
    finalBallFactor = 0.5 * ballFactor + yDistFactor*yTime / maxProjDist;
  } else {
    finalBallFactor = 0.5 * ballFactor + yDistFactor*dist / maxProjDist;
  }
	//if (VERBOSE_MODE) cout << "ballMovingCost: " << ballMovingCost <<" "<<"ballFactor: "<<ballFactor <<endl;
	if (ball.Vel().mod()<100){
		return 0;
	}else{
		return biases[0] * finalBallFactor;
	}
}

int CBestPlayer::getCurMesurementZ()
{
	if (_withBall) {
		return 0;
	}

	if (_oppWithBall) {
		return 2;
	}

	double ourBestPotential = _ourFastestPlayerToBallList[0].potential;
	double theirBestPotential = _theirFastestPlayerToBallList[0].potential;

	char buf[100];
	double diff_potential = ourBestPotential - theirBestPotential;
	sprintf(buf, "%f", diff_potential);
	//GDebugEngine::Instance()->gui_debug_msg(BallPos+CVector(-25.0,10.0),buf,COLOR_CYAN);

	if (diff_potential < -0.0) {
		return 0;
	} else if (diff_potential > 0.1) {
		return 2;
	} else {
		return 1;
	}

	return 0;
}
CBestPlayer::~CBestPlayer()
{

}