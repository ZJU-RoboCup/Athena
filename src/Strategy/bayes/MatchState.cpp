/************************************************************************/
/* Description															*/
/* File:			MatchState.h										*/
/* About:			Header file for match situation estimate			*/
/* Author:			cliffyin											*/
/* Email:			cliffyin@zju.edu.cn		cliffyin007@gmail.com		*/
/* Create Date:		[2012-04-23]										*/
/* Modify Date:		[2012-04-23]										*/
/************************************************************************/

//////////////////////////////////////////////////////////////////////////
// include files
#include "MatchState.h"
#include <param.h>
#include <WorldModel.h>
#include <BestPlayer.h>
#include "GDebugEngine.h"
#include "BestPlayer.h"
#include <fstream>
using namespace std;

#include <assert.h>

//////////////////////////////////////////////////////////////////////////
// define default namespace for common use
namespace {
	/// xml configure tags
	bool VERBOSE_MODE = true;
	string BAYESLIST;
	string BAYESPARAM;
	const bool RecordFlag = false;
	std::string RecordName = "../running_folder/params/GameFilterParam/output.txt";		// 对方使能记录文件
	ofstream RecordFile;
	bool USE_LUA_SCRIPTS = false;
};

/// constructor
CMatchState::CMatchState()
{
	// read in some strings
	{
		DECLARE_PARAM_READER_BEGIN(CMatchState)
			READ_PARAM(BAYESLIST)
			READ_PARAM(BAYESPARAM)
		DECLARE_PARAM_READER_END
	}
	{
		DECLARE_PARAM_READER_BEGIN(General)
		READ_PARAM(USE_LUA_SCRIPTS)
		DECLARE_PARAM_READER_END
	}
	if (RecordFlag) {
		RecordFile.open(RecordName.c_str(),ios::out);
		if (RecordFile.is_open()) {
			RecordFile<<"# Name.bayes"<<endl;
			RecordFile<<BAYESPARAM<<".bayes"<<endl;
			RecordFile<<"#Cycle"<<'\t'<<"U"<<'\t'<<"Z"<<'\t'<<"Bel(Attack)"<<'\t'<<"Bel(Deadlock)"<<'\t'<<"Bel(Defend)"<<endl;
		}
	}

	{
		_pOption = NULL;
		_pVision = NULL;
		_initialed = false;
	}		
}

/// destructor
CMatchState::~CMatchState()
{

}

/// initialize the module and do some register
void CMatchState::initialize(const COptionModule* pOption, const CVisionModule* pVision)
{
	// point to some modules
	_pOption = pOption;
	_pVision = pVision;
	_initialed = false;

	// read in parameters and initial the bayes filter
	BayesReader::Instance()->readBayesParam(BAYESLIST);
	_bayesfilter.init(BayesReader::Instance()->getBayesParam(BAYESPARAM));
	if (VERBOSE_MODE) {
		BayesReader::Instance()->printBayesParam(BAYESPARAM);
	}
	_initialed = true;
	
	// initialize some members
	_cycle = 0;
	_cur_state = MS_DEADLOCK;

	_referee_z_on = false;
	_referee_z = MS_DEADLOCK;
	_referee_z_cnt = 0;
}

/// if initialed
bool CMatchState::isInitialed()
{
	return _initialed;
}

/// update the match state
void CMatchState::update()
{
	// error handle
	if (! _initialed) {
		return ;
	}

	// make sure 
	assert(_pVision);
	_cycle = _pVision->Cycle();

	// 进行控制，得到控制量 u
	ControlT u = obtainControlU();

	// 进行观测，得到观测值 z
	MeasureT z = generateMeasurementZ();

	//GDebugEngine::Instance()->gui_debug_curve(u, 3,-1);
	// 进行滤波，预测与更新
	_bayesfilter.filter(u, z);

	// 判断获得当前的比赛状态
	judgeToGetMatchState();

	// 重新设置置信度 TODO
	//_bayesfilter.resetBeleifs(...)
	//debugHandle();

	if (RecordFlag) {
		double curBel[3];
		_bayesfilter.getBeliefsAfterFiltering(curBel);
		if (RecordFile.is_open()) {
			RecordFile<<_cycle<<'\t'<<u<<'\t'<<z<<'\t'<<curBel[MS_ATTACK]<<'\t'<<curBel[MS_DEADLOCK]<<'\t'<<curBel[MS_DEFEND]<<endl;
			RecordFile<<endl;
		}		
	}

	return ;
}

/// get the current match state
MatchStateT CMatchState::getMatchState()
{
	return _cur_state;
}

/// judge the final match state
void CMatchState::judgeToGetMatchState()
{
	// the max value for the temp
	double curBel[3];
	_bayesfilter.getBeliefsAfterFiltering(curBel);

	bool chooseCost = true;
	if (chooseCost) {
		static double last_cost = 1.0;
		double cost = 0.0;
		for (int i = 0 ; i < 3; i ++) {
			cost += curBel[i] * i;
		}
		cost = cost * 0.75 + last_cost * 0.25;
		last_cost = cost;
		if (cost < 0.75) {
			_cur_state = MS_ATTACK;
		} else if (cost > 1.25) {
			_cur_state = MS_DEFEND;
		} else {
			_cur_state = MS_DEADLOCK;
		}

	} else {
		if (curBel[MS_ATTACK] >= curBel[MS_DEADLOCK] && curBel[MS_ATTACK] >= curBel[MS_DEFEND]) {
			_cur_state = MS_ATTACK;
		}

		if (curBel[MS_DEADLOCK] >= curBel[MS_ATTACK] && curBel[MS_DEADLOCK] >= curBel[MS_DEFEND]) {
			_cur_state = MS_DEADLOCK;
		}

		if (curBel[MS_DEFEND] >= curBel[MS_ATTACK] && curBel[MS_DEFEND] >= curBel[MS_DEADLOCK]) {
			_cur_state = MS_DEFEND;
		}
	}	

//	GDebugEngine::Instance()->gui_debug_curve(_cur_state, 3,-1);


	return ;
}

/// debug print & plot
void CMatchState::debugHandle()
{
	GDebugEngine::Instance()->gui_debug_msg(_pVision->Ball().Pos(), getMatchStateInString().c_str(), COLOR_RED);

	return ;
}

/// get the current match state in form of string
string CMatchState::getMatchStateInString()
{
	switch(_cur_state){
		case MS_ATTACK:
			return "attack";
		case MS_DEADLOCK:
			return "stale";
		case MS_DEFEND:
			return "defense";
		default:
			return "unknown";
	}
}

/// obtain control from stated play attribute
ControlT CMatchState::obtainControlU()
{
	//if(USE_LUA_SCRIPTS){
		return (ControlT)0;
	//} 
	/*else{
		CStatedScriptPlay* curSPlay = SPLAY_BOOK->getcurrentPlay();
		if (NULL == curSPlay) {
			return MS_DEADLOCK;
		}
		return (ControlT)(curSPlay->getPlaySituationAttri());
	}*/
}

/// generate measurement from sensor
MeasureT CMatchState::generateMeasurementZ()
{
	MeasureT cur_z = MS_DEADLOCK;

	// 优先考虑裁判盒的观测：		定时处理
	cur_z = handleRefereeMeasurementZ();
	if (_referee_z_on) {
		_referee_z_cnt --;
		if (_referee_z_cnt <= 0) {
			_referee_z_on = false;			// 打开裁判盒观测启动开关
			_referee_z = MS_DEADLOCK;		// 重置裁判盒观测获取结果
			_referee_z_cnt = 0;				// 重置裁判盒观测定时周期
		}

		return cur_z;
	}

	// 其次考虑策略强制的观测：	定时处理


	// 最后考虑图像判断的观测：	实时处理
	cur_z = handleVisionMeasurementZ();	

	return cur_z;
}

/// generate measurement by referee
MeasureT CMatchState::handleRefereeMeasurementZ()
{
	// 临时变量，表示当前的裁判观测
	MeasureT tmp_referee_z = MS_DEADLOCK;
	bool tmp_referee_z_on = false;

	// 更新当前的裁判观测
	if ((! _pVision->gameState().canMove())
		|| _pVision->gameState().gameOver() 
		|| _pVision->gameState().isOurTimeout()) {
		tmp_referee_z = MS_DEADLOCK;
		tmp_referee_z_on = true;
	} else if(!_pVision->gameState().allowedNearBall()){
		// 对方处理球
		tmp_referee_z_on = true;
		if(_pVision->gameState().theirIndirectKick()){			
			tmp_referee_z = MS_DEFEND;
		} else if (_pVision->gameState().theirDirectKick() ){			
			tmp_referee_z = MS_DEFEND;
		} else if (_pVision->gameState().theirKickoff()){			
			tmp_referee_z = MS_DEFEND;
		} else if (_pVision->gameState().theirPenaltyKick()){			
			tmp_referee_z = MS_DEFEND;
		} else {			
			tmp_referee_z = MS_DEADLOCK;
		}
	} else if( _pVision->gameState().ourRestart()){
		// 我发处理球
		tmp_referee_z_on = true;
		if( _pVision->gameState().ourKickoff() ) {
			tmp_referee_z = MS_ATTACK;
		} else if(_pVision->gameState().penaltyKick()){
			tmp_referee_z = MS_ATTACK;
		} else if(_pVision->gameState().ourIndirectKick()){
			tmp_referee_z = MS_ATTACK;
		} else if(_pVision->gameState().ourDirectKick()){
			tmp_referee_z = MS_ATTACK;
		} 
	}

	// 合理的实时更新，并进行定时操作
	if (tmp_referee_z_on) {
		_referee_z_on = true;			// 打开裁判盒观测启动开关
		_referee_z = tmp_referee_z;		// 设置裁判盒观测获取结果
		_referee_z_cnt = 180;			// 设定裁判盒观测定时周期
	}

	return _referee_z;
}

/// generate measurement by vision
MeasureT CMatchState::handleVisionMeasurementZ()
{
	MatchStateT curMeasurement = (MatchStateT)BestPlayer::Instance()->getCurMesurementZ();

	string output = "";
	if (MS_ATTACK == curMeasurement) {
		output = "Z_Attack";
	} else if (MS_DEADLOCK == curMeasurement) {
		output = "Z_Stale";
	} else if (MS_DEFEND == curMeasurement) {
		output = "Z_Defend";
	}

	//GDebugEngine::Instance()->gui_debug_msg(_pVision->Ball().Pos(), output.c_str(), COLOR_CYAN);
	return curMeasurement;
}

/// generate measurement by strategy
MeasureT CMatchState::handleStrategyMeasurementZ()
{
	return MS_DEFEND;
}


//void CMatchState::drawOurRobotLabel(int robotNum, const char* label, char color) {
//	if (Utils::PlayerNumValid(robotNum) && _pVision->OurPlayer(robotNum).Valid()) {
//		GDebugEngine::Instance()->gui_debug_msg(_pVision->OurPlayer(robotNum).Pos(), label, color);
//	}
//
//	return ;
//}