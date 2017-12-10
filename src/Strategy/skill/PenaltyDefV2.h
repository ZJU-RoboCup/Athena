#ifndef __PENALTY_DEF_V2_H__
#define __PENALTY_DEF_V2_H__
#include <skill/PlayerTask.h>
/**********************************************************
* Skill: PenaltyDefV2
* Description: �������
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
	bool isOppReady(const CVisionModule *pVision);   // �����Ƿ��Ѿ��ú���
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
	bool captureFlag;     // �Ƿ��¼ת��ǰʱ��ı�ʶ
	int _stillCycle[100];
	int _averageStillCycle;
	double _variance;     // _stillCycle[] �ķ���
	int _stillCycleCnt;		// ���򲻶�ʱ�䣨֡����
	double _initOppDir;   // readyʱ���ֵĳ���
	bool _isNormalStartLastCycle;
	int _readyCnt;
	bool _readyFlag;
	int _category;    // 0��ʾ��֪��, 1��ʾ������ֱ����ǰ��(CMU), 2��ʾ�����̶�ʱ���ת(MRL), 3��ʾ�����ת��ʱ���������.
	int switchCnt;
};

#endif //__PENALTY_DEF_V2_H__