#ifndef _VISION_MODULE_H_
#define _VISION_MODULE_H_
#include <param.h>
#include <WorldDefine.h>
#include <PlayerCommand.h>
#include "BallPredictor.h"
#include "RobotPredictor.h"
#include <game_state.h>
#include <ServerInterface.h>
#include <OptionModule.h>
#include "utils.h"
#include <vector>

using namespace std;

/// @file   VisionModule.h
/// @author Yonghai Wu <liunian@zju.edu.cn>
/// @date   Tue Oct 13 14:26:36 2009
/// 
/// @brief  视觉预处理，包括滤波，预测等等。
///         
///     本文件主要是维护整个世界模型，最主要的是视觉信息，其他还有裁判
///  盒信息，和下位机通过双向通讯得到的机器人电机转速及其自身状态信息。
///  其中：
/// 
///   -  视觉的原始信息包括 10 个机器人和球的位置信息。视觉信息的处理主
/// 要包括:
///       -# 容错和滤波处理：由于原始数据包含测量噪声，有时还有信息不全
///    和错误的情况发生，所以需要先进行滤波和容错处理。这里采用的
///    kalman 滤波算法。
///       -# 预测：由于摄像机硬件和图像处理费时的原因，视觉信息存在 100
///    ms 左右的延时。所以必须进行预测才能得到机器人和球的真实位置。这
///    里总共尝试过基于神经网络的预测模型和线性预测模型。
///       -# 碰撞模型：在球被机器人挡住时，采用基于碰撞模型的算法对球的
///    位置进行估计。
/// 
///  - 对裁判盒信息的处理，这里主要是做了一层比较友好的包装，重新定义了
/// 读取的接口。
/// 
///  - 从下位机读取的信息具体有：
///      - 红外对管是否检测到球
///      - 射门指令是否被执行\n
///   红外的状态可以用来校正在球被机器人挡住时的碰撞模型，射门的执行情
///   况用来判断射门动作是否被真正执行，这对机器人状态的跳转非常重要。
/************************************************************************/
/*                       VisionModule                                   */
/************************************************************************/
class CVisionModule{
public:
	CVisionModule();
	~CVisionModule(void);
	void registerOption(const COptionModule* pOption);
	const int& getValidNum() const { return _validNum; };
	// 对方场上可用球员数 [6/28/2011 zhanfei]
	const int& getTheirValidNum() const { return _TheirValidNum; };
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	void SetNewVision(const CServerInterface::VisualInfo& vInfo)
	///
	/// @brief 视觉处理主循环，包括滤波，预测等. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	vInfo	Information describing the v. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetRefRecvMsg(const RefRecvMsg);
	void SetNewVision(const CServerInterface::VisualInfo& vInfo);
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	const PlayerVisionT& AllPlayer(int num) const
	///
	/// @brief 得到所有机器人的视觉信息. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	num - 机器人号码，1-5 为我方， 6-10 为对手. 
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const PlayerVisionT& AllPlayer(int num) const { return (num <= Param::Field::MAX_PLAYER) ? OurPlayer(num) : TheirPlayer(num - Param::Field::MAX_PLAYER); }
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	const PlayerVisionT& OurPlayer(int num) const
	///
	/// @brief	取我方机器人视觉信息. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	num	- 机器人号码， 1-5
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const PlayerVisionT& OurPlayer(int num) const {
		if (Utils::PlayerNumValid(num)) {
			return _ourPlayerPredictor[num-1].getResult(_timeCycle);
		} else {
		//	std::cout<<"Player num:" << num << " [ ####### ] Get our player info Invalid !!!"<<std::endl;
			return _ourPlayerPredictor[0].getResult(_timeCycle);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	const PlayerVisionT& TheirPlayer(int num) const
	///
	/// @brief	取对手机器人信息. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	num	- 机器人号码， 1-5 
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const PlayerVisionT& TheirPlayer(int num) const {
		if (Utils::PlayerNumValid(num)) {
			return _theirPlayerPredictor[num-1].getResult(_timeCycle);
		} else {
		//	std::cout<<"Player num:" << num << " [ ####### ] Get their player info Invalid !!!"<<std::endl;
			return _theirPlayerPredictor[0].getResult(_timeCycle);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	const MobileVisionT& Ball() const
	///
	/// @brief	得到球的视觉信息. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const MobileVisionT& Ball() const { return _ballPredictor.getResult(_timeCycle); }	
	// 带有周期数的队员和球的位置信息

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	const PlayerVisionT& OurPlayer(int cycle, int num) const
	///
	/// @brief	取特定周期的机器人视觉信息. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	cycle	周期数. 
	/// @param	num		机器人号码 1-5. 
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const PlayerVisionT& OurPlayer(int cycle, int num) const { return _ourPlayerPredictor[num-1].getResult(cycle); }

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	const PlayerVisionT& TheirPlayer(int cycle, int num) const
	///
	/// @brief	取特定周期对方机器人视觉信息. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	cycle	The cycle. 
	/// @param	num		机器人号码 1-5. 
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const PlayerVisionT& TheirPlayer(int cycle, int num) const { return _theirPlayerPredictor[num-1].getResult(cycle); }

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	const MobileVisionT& Ball(int cycle) const
	///
	/// @brief	取特定周期数球的信息. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	cycle	The cycle. 
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const MobileVisionT& Ball(int cycle) const { return _ballPredictor.getResult(cycle);	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	const ObjectPoseT& RawBall() const
	///
	/// @brief	取未经过处理的原始信息的球. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const ObjectPoseT& RawBall() const {return _rawBallPos; }
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	const RobotRawVisionData& OurRawPlayer(int num) const
	///
	/// @brief	取我方机器人的原始视觉信息. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	num	- 机器人号码， 1-5 为我方， 6-10 为对手. 
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const RobotRawVisionData& OurRawPlayer(int num) const {return _ourPlayerPredictor[num-1].getRawData(_timeCycle); }
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	const RobotRawVisionData& TheirRawPlayer(int num) const
	///
	/// @brief	取对方机器人的原始视觉信息. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	num	机器人号码， 1-5. 
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const RobotRawVisionData& TheirRawPlayer(int num) const {return _theirPlayerPredictor[num-1].getRawData(_timeCycle); }
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	void SetPlayerCommand(int num, const CPlayerCommand* pCmd)
	///
	/// @brief	记录本周期我方机器人执行的指令，保存到历史堆栈里。 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	num		Number of. 
	/// @param	pCmd	If non-null, the command. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetPlayerCommand(int num, const CPlayerCommand* pCmd);
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	const GameState& gameState() const
	///
	/// @brief	得到裁判盒信息. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const GameState& gameState() const { return _gameState; } ///< 比赛状态
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	int PlayerLostTime(const int number) const
	///
	/// @brief	得到我方机器人连续丢失的时间. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	number	Number of. 
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	int PlayerLostTime(const int number) const { return _ourPlayerPredictor[number-1].lostTime(); }
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	int Cycle() const
	///
	/// @brief	得到当前的周期数. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	int Cycle() const { return _timeCycle; }
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	int LastCycle() const
	///
	/// @brief	得到上一次周期的周期数. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	int LastCycle() const { return _lastTimeCycle; }

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	int Side() const
	///
	/// @brief	得到我方选中的边. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	int Side() const { return _pOption->MySide(); }

	const string GetCurrentRefereeMsg()const;
	const string GetLastRefereeMsg()const;
	int  OurGoal(){ return _ourGoal; }
	int  TheirGoal(){ return _theirGoal; }
	int  TimeRemain(){ return _refRecvMsg.timeRemain; }

	const CVector OurRawPlayerSpeed(int num) const {return _ourPlayerPredictor[num-1].getRawSpeed(_timeCycle); }
	const CVector TheirRawPlayerSpeed(int num) const {return _theirPlayerPredictor[num-1].getRawSpeed(_timeCycle); }

	const int GetTheirPenaltyNum() {return _theirPenaltyNum;}
	void ResetTheirPenaltyNum() { _theirPenaltyNum = 0;}
	int GetTheirGoalieStrategyNum() {return _theirGoalieStrategyNum; }

	bool getBallVelStable(){return !_ballVelDirChanged;}

	bool ballVelValid();
protected:
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	void SetCycle(int cycle)
	///
	/// @brief	设置当前周期数. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	cycle	The cycle. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetCycle(int cycle){ _lastTimeCycle = _timeCycle; _timeCycle = cycle; } 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	void CheckBothSidePlayerNum()
	///
	/// @brief	检测双方球员在场数量. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void CheckBothSidePlayerNum();
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	void CheckKickoffStatus(const CServerInterface::VisualInfo& info)
	///
	/// @brief	检测带球状态. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	info	The information. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void CheckKickoffStatus(const CServerInterface::VisualInfo& info); // 检查开球状态

	void UpdateRefereeMsg();

	void judgeBallVelStable();
private:
	/// 比赛模式参数以及比赛当前状态
	const COptionModule* _pOption;   			///< 比赛相关参数，是左半场还是右半场. 
	static const int MAX_SAVE_CYCLES = 64;		///< 保存的最多周期数. 
	GameState _gameState;						///< 比赛状态，裁判盒信息.

	/// 比赛相关的周期（时间）
	int _timeCycle;								///< 当前周期数.
	int _lastTimeCycle;							///< 上一周期数.
	
	///	数据滤波器
	CBallPredictor _ballPredictor;										///< 球预测. 
	CRobotPredictor _ourPlayerPredictor[Param::Field::MAX_PLAYER];		///< 我方机器人预测. 
	CRobotPredictor _theirPlayerPredictor[Param::Field::MAX_PLAYER];	///< 对方机器人预测. 

	/// 双方场上队员统计
	int _validNum;							///< 得到除守门员外我方的球员数
	int _TheirValidNum;						///< 得到对方的球员数

	bool _ballKicked;						///< 判断球是否被踢,用来决定比赛状态.
	CGeoPoint _ballPosSinceNotKicked;		///< 球在没有被踢之前的位置.
	ObjectPoseT _rawBallPos;				///< 这个周期球的原始数据，用来draw比较.
	
	ObjectPoseT _newBallPos; // the most useless varible I have ever seen.
	CGeoPoint _ballPos[10];  // and this more than the last line.
	int ballPosCollectorCounter;

	ObjectPoseT _lastRawBallPos;			///< 上帧球的的原始数据
	// 保存数据
	bool _hasCollision;						///< 发生过碰撞,此时的处理特殊些(给下一个周期用).
	string _refereeMsg;
	string _lastRefereeMsg;
	int _lastContactNum;
	RefRecvMsg _refRecvMsg;
	int _ourGoal;
	int _theirGoal;
	int _ourGoalie;
	int _theirGoalie;
	int _theirGoalieStrategyNum;
	int _last_xbox_pressed;
	int _theirPenaltyNum; // 对方第几个点球

	int _ballVelChangeCouter;
	bool _ballVelDirChanged;

private:
	static int RECEIVE_X_MIN;
	static int RECEIVE_X_MAX;
	static int RECEIVE_Y_MIN;
	static int RECEIVE_Y_MAX;
};

typedef NormalSingleton<CVisionModule> VisionModule;
#define VISION_MODULE VisionModule::Instance()

#endif // _VISION_MODULE_H_
