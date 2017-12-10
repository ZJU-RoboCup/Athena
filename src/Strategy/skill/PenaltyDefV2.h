#ifndef __PENALTY_DEF_V2_H__
#define __PENALTY_DEF_V2_H__
#include <skill/PlayerTask.h>
/**********************************************************
* Skill: PenaltyDefV2
* Description: 点球防守
* Author: Ye Yangsheng
* E-mail: piaoxu1993@gmail.com
* Created Date: 2014-05-29
***********************************************************/

class CPenaltyDefV2 :public CStatedTask {
public:
	CPenaltyDefV2();
	~CPenaltyDefV2();
	virtual void plan(const CVisionModule* pVision);
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
	virtual bool isEmpty() const { return false; }
	int GetNearestEnemy(const CVisionModule *pVision);
	bool isOppTurn(const CVisionModule *pVision);
	bool isOppReady(const CVisionModule *pVision);   // 对手是否已经拿好球
	bool isOppDribble(const CVisionModule *pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "CPenaltyDefV2" << std::endl; }
	void printStillCycle();
	void calAverageStillCycle();
	void calVariance();
	void resetData();
private:
	enum {
		CATEGORY_1 = 1,
		CATEGORY_2,
		CATEGORY_3,
		CATEGORY_0,
		WAIT0,
		POS_1,
		POS_2,
		WAIT,
		RUSH,
		RANDOM1,
		RANDOM2,
		POS_3,
		POS_4,
		WAIT2,
		RUSH2,
		STOP
	};
	CPlayerCommand* _directCommand;
	int _state;
	int _lastCycle;
	int _theirPenaltyNum;
	bool _isFirst;
	bool captureFlag;     // 是否记录转动前时间的标识
	int _stillCycle[100];
	int _averageStillCycle;
	double _variance;     // _stillCycle[] 的方差
	int _stillCycleCnt;		// 持球不动时间（帧数）
	double _initOppDir;   // ready时对手的朝向
	bool _isNormalStartLastCycle;
	int _readyCnt;
	bool _readyFlag;
	int _category;    // 0表示不知道, 1表示不拿球直接上前踢(CMU), 2表示拿球后固定时间会转(MRL), 3表示拿球后转动时间是随机的.
	int switchCnt;
};

#endif //__PENALTY_DEF_V2_H__