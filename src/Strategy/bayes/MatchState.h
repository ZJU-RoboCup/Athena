/************************************************************************/
/* Description															*/
/* File:			MatchState.h										*/
/* About:			Header file for match situation estimate			*/
/* Author:			cliffyin											*/
/* Email:			cliffyin@zju.edu.cn		cliffyin007@gmail.com		*/
/* Create Date:		[2012-04-23]										*/
/* Modify Date:		[2012-04-23]										*/
/************************************************************************/

#ifndef _MATCH_STATE_H_
#define _MATCH_STATE_H_

//////////////////////////////////////////////////////////////////////////
// include files
#include "VisionModule.h"						// for vision module
#include "OptionModule.h"						// for option module
#include "singleton.h"							// for singleton
#include "bayes/BayesFilter.h"	// for bayes filter

//////////////////////////////////////////////////////////////////////////
// define the CMathcState class used to evaluate the match situation
class CMatchState {
public:
	/// constructor
	CMatchState();

	/// destructor
	~CMatchState();

	/// initialize the module and do some register
	void initialize(const COptionModule* pOption, const CVisionModule* pVision);

	/// if initialed
	bool isInitialed();

	/// update the match state
	void update();

	/// get the current match state
	MatchStateT getMatchState();

	/// get the current match state in form of string
	string getMatchStateInString();

private:
	/// obtain control from stated play attribute
	ControlT obtainControlU();

	/// generate measurement from sensor
	MeasureT generateMeasurementZ();	

	/// judge the final match state
	void judgeToGetMatchState();

	/// debug print & plot
	void debugHandle();

private:
	bool _referee_z_on;
	MeasureT _referee_z;
	int _referee_z_cnt;
	/// generate measurement by referee
	MeasureT handleRefereeMeasurementZ();

	/// generate measurement by vision
	MeasureT handleVisionMeasurementZ();

	/// generate measurement by strategy
	MeasureT handleStrategyMeasurementZ();

private:
	/// initialize flag
	bool _initialed;
	/// option module
	const COptionModule* _pOption;

	/// vision module
	const CVisionModule* _pVision;

	/// bayes filter
	CBayesFilter _bayesfilter;

	/// current match state
	MatchStateT _cur_state;

	/// current cycle
	int _cycle;
};

typedef NormalSingleton< CMatchState > MatchState;

#define MATCH_STATE MatchState::Instance() 

#endif // ~_MATCH_STATE_H_