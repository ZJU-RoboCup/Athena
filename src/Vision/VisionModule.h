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
/// @brief  �Ӿ�Ԥ���������˲���Ԥ��ȵȡ�
///         
///     ���ļ���Ҫ��ά����������ģ�ͣ�����Ҫ�����Ӿ���Ϣ���������в���
///  ����Ϣ������λ��ͨ��˫��ͨѶ�õ��Ļ����˵��ת�ټ�������״̬��Ϣ��
///  ���У�
/// 
///   -  �Ӿ���ԭʼ��Ϣ���� 10 �������˺����λ����Ϣ���Ӿ���Ϣ�Ĵ�����
/// Ҫ����:
///       -# �ݴ���˲���������ԭʼ���ݰ���������������ʱ������Ϣ��ȫ
///    �ʹ�������������������Ҫ�Ƚ����˲����ݴ���������õ�
///    kalman �˲��㷨��
///       -# Ԥ�⣺���������Ӳ����ͼ�����ʱ��ԭ���Ӿ���Ϣ���� 100
///    ms ���ҵ���ʱ�����Ա������Ԥ����ܵõ������˺������ʵλ�á���
///    ���ܹ����Թ������������Ԥ��ģ�ͺ�����Ԥ��ģ�͡�
///       -# ��ײģ�ͣ����򱻻����˵�סʱ�����û�����ײģ�͵��㷨�����
///    λ�ý��й��ơ�
/// 
///  - �Բ��к���Ϣ�Ĵ���������Ҫ������һ��Ƚ��Ѻõİ�װ�����¶�����
/// ��ȡ�Ľӿڡ�
/// 
///  - ����λ����ȡ����Ϣ�����У�
///      - ����Թ��Ƿ��⵽��
///      - ����ָ���Ƿ�ִ��\n
///   �����״̬��������У�����򱻻����˵�סʱ����ײģ�ͣ����ŵ�ִ����
///   �������ж����Ŷ����Ƿ�����ִ�У���Ի�����״̬����ת�ǳ���Ҫ��
/************************************************************************/
/*                       VisionModule                                   */
/************************************************************************/
class CVisionModule{
public:
	CVisionModule();
	~CVisionModule(void);
	void registerOption(const COptionModule* pOption);
	const int& getValidNum() const { return _validNum; };
	// �Է����Ͽ�����Ա�� [6/28/2011 zhanfei]
	const int& getTheirValidNum() const { return _TheirValidNum; };
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	void SetNewVision(const CServerInterface::VisualInfo& vInfo)
	///
	/// @brief �Ӿ�������ѭ���������˲���Ԥ���. 
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
	/// @brief �õ����л����˵��Ӿ���Ϣ. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	num - �����˺��룬1-5 Ϊ�ҷ��� 6-10 Ϊ����. 
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const PlayerVisionT& AllPlayer(int num) const { return (num <= Param::Field::MAX_PLAYER) ? OurPlayer(num) : TheirPlayer(num - Param::Field::MAX_PLAYER); }
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	const PlayerVisionT& OurPlayer(int num) const
	///
	/// @brief	ȡ�ҷ��������Ӿ���Ϣ. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	num	- �����˺��룬 1-5
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
	/// @brief	ȡ���ֻ�������Ϣ. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	num	- �����˺��룬 1-5 
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
	/// @brief	�õ�����Ӿ���Ϣ. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const MobileVisionT& Ball() const { return _ballPredictor.getResult(_timeCycle); }	
	// �����������Ķ�Ա�����λ����Ϣ

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	const PlayerVisionT& OurPlayer(int cycle, int num) const
	///
	/// @brief	ȡ�ض����ڵĻ������Ӿ���Ϣ. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	cycle	������. 
	/// @param	num		�����˺��� 1-5. 
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const PlayerVisionT& OurPlayer(int cycle, int num) const { return _ourPlayerPredictor[num-1].getResult(cycle); }

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	const PlayerVisionT& TheirPlayer(int cycle, int num) const
	///
	/// @brief	ȡ�ض����ڶԷ��������Ӿ���Ϣ. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	cycle	The cycle. 
	/// @param	num		�����˺��� 1-5. 
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const PlayerVisionT& TheirPlayer(int cycle, int num) const { return _theirPlayerPredictor[num-1].getResult(cycle); }

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	const MobileVisionT& Ball(int cycle) const
	///
	/// @brief	ȡ�ض������������Ϣ. 
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
	/// @brief	ȡδ���������ԭʼ��Ϣ����. 
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
	/// @brief	ȡ�ҷ������˵�ԭʼ�Ӿ���Ϣ. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	num	- �����˺��룬 1-5 Ϊ�ҷ��� 6-10 Ϊ����. 
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const RobotRawVisionData& OurRawPlayer(int num) const {return _ourPlayerPredictor[num-1].getRawData(_timeCycle); }
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	const RobotRawVisionData& TheirRawPlayer(int num) const
	///
	/// @brief	ȡ�Է������˵�ԭʼ�Ӿ���Ϣ. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	num	�����˺��룬 1-5. 
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const RobotRawVisionData& TheirRawPlayer(int num) const {return _theirPlayerPredictor[num-1].getRawData(_timeCycle); }
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	void SetPlayerCommand(int num, const CPlayerCommand* pCmd)
	///
	/// @brief	��¼�������ҷ�������ִ�е�ָ����浽��ʷ��ջ� 
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
	/// @brief	�õ����к���Ϣ. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @return	. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	const GameState& gameState() const { return _gameState; } ///< ����״̬
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	int PlayerLostTime(const int number) const
	///
	/// @brief	�õ��ҷ�������������ʧ��ʱ��. 
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
	/// @brief	�õ���ǰ��������. 
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
	/// @brief	�õ���һ�����ڵ�������. 
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
	/// @brief	�õ��ҷ�ѡ�еı�. 
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
	/// @brief	���õ�ǰ������. 
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
	/// @brief	���˫����Ա�ڳ�����. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void CheckBothSidePlayerNum();
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	void CheckKickoffStatus(const CServerInterface::VisualInfo& info)
	///
	/// @brief	������״̬. 
	///
	/// @author	Yonghai Wu
	/// @date	2009-10-13
	///
	/// @param	info	The information. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void CheckKickoffStatus(const CServerInterface::VisualInfo& info); // ��鿪��״̬

	void UpdateRefereeMsg();

	void judgeBallVelStable();
private:
	/// ����ģʽ�����Լ�������ǰ״̬
	const COptionModule* _pOption;   			///< ������ز���������볡�����Ұ볡. 
	static const int MAX_SAVE_CYCLES = 64;		///< ��������������. 
	GameState _gameState;						///< ����״̬�����к���Ϣ.

	/// ������ص����ڣ�ʱ�䣩
	int _timeCycle;								///< ��ǰ������.
	int _lastTimeCycle;							///< ��һ������.
	
	///	�����˲���
	CBallPredictor _ballPredictor;										///< ��Ԥ��. 
	CRobotPredictor _ourPlayerPredictor[Param::Field::MAX_PLAYER];		///< �ҷ�������Ԥ��. 
	CRobotPredictor _theirPlayerPredictor[Param::Field::MAX_PLAYER];	///< �Է�������Ԥ��. 

	/// ˫�����϶�Աͳ��
	int _validNum;							///< �õ�������Ա���ҷ�����Ա��
	int _TheirValidNum;						///< �õ��Է�����Ա��

	bool _ballKicked;						///< �ж����Ƿ���,������������״̬.
	CGeoPoint _ballPosSinceNotKicked;		///< ����û�б���֮ǰ��λ��.
	ObjectPoseT _rawBallPos;				///< ����������ԭʼ���ݣ�����draw�Ƚ�.
	
	ObjectPoseT _newBallPos; // the most useless varible I have ever seen.
	CGeoPoint _ballPos[10];  // and this more than the last line.
	int ballPosCollectorCounter;

	ObjectPoseT _lastRawBallPos;			///< ��֡��ĵ�ԭʼ����
	// ��������
	bool _hasCollision;						///< ��������ײ,��ʱ�Ĵ�������Щ(����һ��������).
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
	int _theirPenaltyNum; // �Է��ڼ�������

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
