/// FileName : 		BestPlayer.h
/// 				declaration file
/// Description :	Calculate vehicle to ball potential list, important
///	Keywords :		best, potential
/// Organization : 	ZJUNlict@Small Size League
/// Author : 		cliffyin
/// E-mail : 		cliffyin@zju.edu.cn
///					cliffyin007@gmail.com
/// Create Date : 	2011-07-25
/// Modified Date :	2011-07-25 
/// History :

#ifndef _BEST_PLAYER_H_
#define _BEST_PLAYER_H_

//////////////////////////////////////////////////////////////////////////
// include files
#include <geometry.h>
#include <VisionModule.h>
#include <param.h>

//////////////////////////////////////////////////////////////////////////
// define the CBestPlayer class used to evaluate to ball potential
// important algorithm module !!!
class CBestPlayer
{
public:
	//////////////////////////////////////////////////////////////////////////
	// define the ToBallPotentialPair structure used to store ball potential
	struct ToBallPotentialPair { 
		ToBallPotentialPair(int n, double p) : num(n), potential(p) { }
		bool operator < (const ToBallPotentialPair& n) const { return potential < n.potential; }
		int num;					// player number
		double potential;			// ball potential
	};
	typedef std::vector< ToBallPotentialPair > PlayerList;

public:
	CBestPlayer();

	~CBestPlayer();

	void update(const CVisionModule *pVision);
	int getCurMesurementZ();

	const PlayerList& ourFastestPlayerToBallList() const { return _ourFastestPlayerToBallList; }
	double getOurPotential(int num) { return _Potentials[num-1][0]; }
	const PlayerList& theirFastestPlayerToBallList() const { return _theirFastestPlayerToBallList; }
	double getTheirPotential(int num) const { return _theirPotentials[num-1][0]; }

	bool withBall() const { return _withBall; }
	bool oppWithBall() const { return _oppWithBall; }

	int getTheirGoalie();

	bool isOurBestPlayerChanged() { return _ourChangeFlag; }
	int getOurBestPlayer() { return _ourLastBestPlayer; }
	bool isTheirBestPlayerChanged() { return _theirChangeFlag; }
	int getTheirBestPlayer() { return _theirLastBestPlayer; }	

	bool isOurPlayerStrictControlBall(int num) { return _ourPlayerBallStrictControl[num-1]; }
	bool isOurPlayerLooseControlBall(int num) { return _ourPlayerBallLooseControl[num-1]; }

	bool isTheirPlayerStrictControlBall(int num) { return _theirPlayerBallStrictControl[num-1]; }
	bool isTheirPlayerLooseControlBall(int num) { return _theirPlayerBallLooseControl[num-1]; }

protected:
	void updateBothGoalie(const CVisionModule *pVision);

	void updateBall(const CVisionModule* pVision);

	void updateOurPlayerList(const CVisionModule *pVision,const double *biases);

	void updateOurBestPlayer(const CVisionModule* pVision);

	void updateTheirPlayerList(const CVisionModule *pVision,const double *biases);

	void updateTheirBestPlayer(const CVisionModule* pVision);	

	//����Ӱ�����ܱ仯������,biasesд����������涼���ԣ�ͳһ��׼
	double distBallToPlayer(CVector playerToBall,const double *biases,double maxDist);
	double shootAngle(CGeoPoint& _ourPlayer,CGeoPoint& ball,const double *biases,bool flag);//�ҷ���flagΪtrue���ط���flagΪfalse
	double clearPath(const CVisionModule *pVision,CGeoPoint& _ourPlayer,CGeoPoint& ball,const double *biases);//�з������ҷ������赲
	double theirClearPath(const CVisionModule *pVision,CGeoPoint& _theirPlayer,CGeoPoint& ball,const double *biases);//�ҷ����Եз������赲
	double anglePlayerToBall(const PlayerVisionT& player,const CGeoPoint& ball,const double *biases);//������֮��ĽǶ�����
	double velBallToPlayer(const PlayerVisionT& player,const MobileVisionT& ball,const double *biases,double maxDist);//����ٶ�
private:
	/// �ҷ���Ա������
	double _Potentials[Param::Field::MAX_PLAYER][7];
	/// �Է���Ա������
	double _theirPotentials[Param::Field::MAX_PLAYER][6];

	/// �ҷ���Ա�������ܵ��б���С����
	PlayerList _ourFastestPlayerToBallList;
	/// �Է���Ա�������ܵ��б���С����
	PlayerList _theirFastestPlayerToBallList;

	/// �ҷ���Ա�Ƿ��п��򣺿���
	bool _ourPlayerBallLooseControl[Param::Field::MAX_PLAYER];
	/// �Է���Ա�Ƿ��п��򣺿���
	bool _theirPlayerBallLooseControl[Param::Field::MAX_PLAYER];

	/// �ҷ���Ա�Ƿ��п����Ͻ�
	bool _ourPlayerBallStrictControl[Param::Field::MAX_PLAYER];
	/// �Է���Ա�Ƿ��п����Ͻ�
	bool _theirPlayerBallStrictControl[Param::Field::MAX_PLAYER];

	/// �ҷ�������
	bool _withBall;
	/// �Է�������
	bool _oppWithBall;

	/// ��ʶ�ҷ��Ķ���������Ա�б仯
	bool _ourChangeFlag;
	/// ��¼�ҷ��Ķ���������Ա
	int _ourLastBestPlayer;
	/// ��¼�ҷ��Ķ���������Ա�仯����
	int _ourChangeCycle;

	/// ��ʶ�Է��Ķ���������Ա�б仯
	bool _theirChangeFlag;
	/// ��¼�Է��Ķ���������Ա
	int _theirLastBestPlayer;
	/// ��¼�Է��Ķ���������Ա�仯����
	int _theirChangeCycle;
};

typedef NormalSingleton< CBestPlayer > BestPlayer; // singleton pattern

#define BEST_PLAYER BestPlayer::Instance()

#endif // ~_BEST_PLAYER_H_