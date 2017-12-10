#ifndef _SHOOT_RANGE_LIST_H_
#define _SHOOT_RANGE_LIST_H_
#include <ValueRange.h>
#include "VisionModule.h"
#include <vector>
/**
* CShootRangeList.
* 产生一个可射到球门的角度域 列表
* 
* player 指队员号(只是为了取得robotCap,若小车性能一样,可任取1~5值)
* pos 用于直接计算某点处(而不是球的位置)的射门空档
* added by jwl
*/

#define SR_DEFENCE true  //专用于defence时的防守标志

class CShootRangeList{
public:
	CShootRangeList(const CVisionModule* pVision, const int player, 
					const CGeoPoint pos = CGeoPoint(5000,5000));
	//专为防守而计算的射门角度,为了明确重载，必须传入第二个参数
	//player为需要排除的一个己方队员，player = 0则不需要排除己方队员 wq
	//！！！！！！！！！注意得到的结果需要反向
	CShootRangeList(const CVisionModule* pVision,const bool defence,
					const int player = 0,const CGeoPoint pos = CGeoPoint(5000,5000));//结果要反向

	const CValueRangeList& get();
	const CValueRangeList& getShootRange();
	const CValueRangeList& getChipKickRange();

private:
	// 保留上次的，防止重复计算
	CValueRangeList lastBothKickList; // 上次射门的范围，包括击球挑球
	CValueRangeList lastShootList; // 上次击球射门
	CValueRangeList lastChipKickList; // 上次挑球射门
	int lastCycle; // 上次的时间
	int lastPlayer; // 上次的队员号码
	CGeoPoint lastPoint;

};
#endif // _SHOOT_RANGE_LIST_H_