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

	//六个影响势能变化的因素,biases写在里面和外面都可以，统一标准
	double distBallToPlayer(CVector playerToBall,const double *biases,double maxDist);
	double shootAngle(CGeoPoint& _ourPlayer,CGeoPoint& ball,const double *biases,bool flag);//我方的flag为true，地方的flag为false
	double clearPath(const CVisionModule *pVision,CGeoPoint& _ourPlayer,CGeoPoint& ball,const double *biases);//敌方车对我方车的阻挡
	double theirClearPath(const CVisionModule *pVision,CGeoPoint& _theirPlayer,CGeoPoint& ball,const double *biases);//我方车对敌方车的阻挡
	double anglePlayerToBall(const PlayerVisionT& player,const CGeoPoint& ball,const double *biases);//车和球之间的角度因素
	double velBallToPlayer(const PlayerVisionT& player,const MobileVisionT& ball,const double *biases,double maxDist);//相对速度
private:
	/// 我方球员的势能
	double _Potentials[Param::Field::MAX_PLAYER][7];
	/// 对方球员的势能
	double _theirPotentials[Param::Field::MAX_PLAYER][6];

	/// 我方球员对球势能的列表：由小到大
	PlayerList _ourFastestPlayerToBallList;
	/// 对方球员对球势能的列表：由小到大
	PlayerList _theirFastestPlayerToBallList;

	/// 我方球员是否有控球：宽松
	bool _ourPlayerBallLooseControl[Param::Field::MAX_PLAYER];
	/// 对方球员是否有控球：宽松
	bool _theirPlayerBallLooseControl[Param::Field::MAX_PLAYER];

	/// 我方球员是否有控球：严紧
	bool _ourPlayerBallStrictControl[Param::Field::MAX_PLAYER];
	/// 对方球员是否有控球：严紧
	bool _theirPlayerBallStrictControl[Param::Field::MAX_PLAYER];

	/// 我方控制球
	bool _withBall;
	/// 对方控制球
	bool _oppWithBall;

	/// 标识我方的对球最优球员有变化
	bool _ourChangeFlag;
	/// 记录我方的对球最优球员
	int _ourLastBestPlayer;
	/// 记录我方的对球最优球员变化周期
	int _ourChangeCycle;

	/// 标识对方的对球最优球员有变化
	bool _theirChangeFlag;
	/// 记录对方的对球最优球员
	int _theirLastBestPlayer;
	/// 记录对方的对球最优球员变化周期
	int _theirChangeCycle;
};

typedef NormalSingleton< CBestPlayer > BestPlayer; // singleton pattern

#define BEST_PLAYER BestPlayer::Instance()

#endif // ~_BEST_PLAYER_H_