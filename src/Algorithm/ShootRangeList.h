#ifndef _SHOOT_RANGE_LIST_H_
#define _SHOOT_RANGE_LIST_H_
#include <ValueRange.h>
#include "VisionModule.h"
#include <vector>
/**
* CShootRangeList.
* ����һ�����䵽���ŵĽǶ��� �б�
* 
* player ָ��Ա��(ֻ��Ϊ��ȡ��robotCap,��С������һ��,����ȡ1~5ֵ)
* pos ����ֱ�Ӽ���ĳ�㴦(���������λ��)�����ſյ�
* added by jwl
*/

#define SR_DEFENCE true  //ר����defenceʱ�ķ��ر�־

class CShootRangeList{
public:
	CShootRangeList(const CVisionModule* pVision, const int player, 
					const CGeoPoint pos = CGeoPoint(5000,5000));
	//רΪ���ض���������ŽǶ�,Ϊ����ȷ���أ����봫��ڶ�������
	//playerΪ��Ҫ�ų���һ��������Ա��player = 0����Ҫ�ų�������Ա wq
	//������������������ע��õ��Ľ����Ҫ����
	CShootRangeList(const CVisionModule* pVision,const bool defence,
					const int player = 0,const CGeoPoint pos = CGeoPoint(5000,5000));//���Ҫ����

	const CValueRangeList& get();
	const CValueRangeList& getShootRange();
	const CValueRangeList& getChipKickRange();

private:
	// �����ϴεģ���ֹ�ظ�����
	CValueRangeList lastBothKickList; // �ϴ����ŵķ�Χ��������������
	CValueRangeList lastShootList; // �ϴλ�������
	CValueRangeList lastChipKickList; // �ϴ���������
	int lastCycle; // �ϴε�ʱ��
	int lastPlayer; // �ϴεĶ�Ա����
	CGeoPoint lastPoint;

};
#endif // _SHOOT_RANGE_LIST_H_