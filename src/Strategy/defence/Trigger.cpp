#include "Trigger.h"
#include "DefenceInfo.h"
#include <math.h>
#include <algorithm>
#include "WorldModel.h"
bool CTrigger::handler(const CVisionModule* pVision)
{
	if (true == this->trigger(pVision))
	{
		return true;
	} else if (NULL != _next)
	{
		return _next->handler(pVision);
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
//make trigger

namespace{};

DESCRIPT_TRIGGER(testTrigger1)
{
	//cout<<" trigger test 1  is false"<<endl;
	return false;
}

DESCRIPT_TRIGGER(testTrigger2)
{
	//cout<<" trigger test 2  is true"<<endl;
	return false;
}

// 第一个防守序列更新触发器
// getAttackOppNumByPri
DESCRIPT_TRIGGER(TKeeperChanged)
{
	bool changed = false;
	static int lastBallKeeper = 0;
	//compare
	if (lastBallKeeper != DefenceInfo::Instance()->getAttackOppNumByPri(0))
	{
		//cout<<"keeper changed is "<<changed<<endl;
		changed = true;
	}
	//update
	lastBallKeeper = DefenceInfo::Instance()->getAttackOppNumByPri(0);
	//return
	//if (changed)
		//cout << "keepper changed"<< endl;
	return changed;
}

DESCRIPT_TRIGGER(TAttackerChanged)
{
	static int lastAttackNum = 0;
	static int lastAttackArray[Param::Field::MAX_PLAYER] = {0,0,0,0,0,0};
	bool changed = false;
	const int attackNum = DefenceInfo::Instance()->getAttackNum();
	//compare
	if (lastAttackNum != attackNum)
	{
		changed = true;
		//cout<<"ssssssssssssss"<<endl;
	} 
	else{
		sort(lastAttackArray,lastAttackArray + attackNum);
		int tempArray[Param::Field::MAX_PLAYER] = {0,0,0,0,0,0};
		for (int i = 0;i <= attackNum - 1;++i)
			tempArray[i] = DefenceInfo::Instance()->getAttackOppNumByPri(i);
		sort(tempArray,tempArray + attackNum);
		int i = 0;
		for (;i < attackNum;++i)
		{
			if (lastAttackArray[i] != tempArray[i])
			{
				changed = true;
				//cout<<"true!!!!!!!!!!!!!!!!!!!"<<endl;
				break;
			}
		}
	} 
	//update
	lastAttackNum = DefenceInfo::Instance()->getAttackNum();
	for (int i = 0;i < Param::Field::MAX_PLAYER;++i)
	{
		lastAttackArray[i] = DefenceInfo::Instance()->getAttackOppNumByPri(i);
	}
	//return
	//if (changed)
		//cout << "attacker changed" << endl;
	return changed;
}

DESCRIPT_TRIGGER(TReceiverNoMarked)
{
	const string refMsg = WorldModel::Instance()->CurrentRefereeMsg();
	if ("theirIndirectKick" == refMsg || "theirDirectKick" == refMsg || "theirKickOff" == refMsg || VisionModule::Instance()->gameState().gameOn() )
	{
		if (DefenceInfo::Instance()->getAttackNum() >= 2){
			for (int i=1;i<=2;i++)
			{
				if (DefenceInfo::Instance()->getOppPlayerByNum(i)->getRoleName()=="RReceiver" && DefenceInfo::Instance()->getAttackNum() >= 2){
					//cout<<"RReceiver "<<i<<endl;
					if(false == DefenceInfo::Instance()->queryMarked(DefenceInfo::Instance()->getAttackOppNumByPri(i))){
						return true;
					}
				}
			}
		}
		//if (DefenceInfo::Instance()->getAttackNum() >= 2 && false == DefenceInfo::Instance()->queryMarked(DefenceInfo::Instance()->getAttackOppNumByPri(1)))
		//{
		//	//cout<<"receiverNoMarked is 1"<<endl;
		//	return true;
		//}
	}
	return false;
}

DESCRIPT_TRIGGER(TReceiverNoMarked2)
{
	const string refMsg = WorldModel::Instance()->CurrentRefereeMsg();
	if ("theirIndirectKick" == refMsg || "theirDirectKick" == refMsg || "theirKickOff" == refMsg)
	{
		if (DefenceInfo::Instance()->getAttackNum() >= 3 && false == DefenceInfo::Instance()->queryMarked(DefenceInfo::Instance()->getAttackOppNumByPri(2)))
		{
			//cout<<"receiverNoMarked2 is 1"<<endl;
			return true;
		}
	}
	return false;
}

DESCRIPT_TRIGGER(MTPnoMarked)
{
	const string refMsg = WorldModel::Instance()->CurrentRefereeMsg();
	if ("theirIndirectKick" == refMsg || "theirDirectKick" == refMsg || "theirKickOff" == refMsg || VisionModule::Instance()->gameState().gameOn())
	{

	}
	double maxValue = 0;
	bool isMarked = true;
	for (int i = 1; i <= Param::Field::MAX_PLAYER; ++i)
	{
		int pri = DefenceInfo::Instance()->getOppPlayerByNum(i)->getThreatenPri();
		int value = DefenceInfo::Instance()->getOppPlayerByNum(i)->getThreatenValue();
		bool Marked = DefenceInfo::Instance()->queryMarked(i);
		//getSteadyAttackOppNumByPri
		//cout << i << " " << pri << " " << value << " " << Marked << endl;
		if (pri > 1.5 && pri<50 && value>maxValue) { //att:2 leader:1 goalie:100
			maxValue = value;
			isMarked = Marked;
		}
	}
	if (isMarked == false)
		return true;
	return false;
}

//MarkingValueDifference
DESCRIPT_TRIGGER(MarkingValueDifference)
{
	static int lastFrame = 0;
	int Markcnt = 0;
	double sum_unsteady = 0;
	double sum_steady = 0;
	for (int i = 1; i <= Param::Field::MAX_PLAYER; ++i)
	{
		DefenceInfo::Instance()->getOppPlayerByNum(i)->getThreatenPri();
		DefenceInfo::Instance()->getOppPlayerByNum(i)->getThreatenValue();
		if (DefenceInfo::Instance()->queryMarked(i))
			Markcnt++;
		//cout <<"in1"<< i<<" "<<Markcnt << endl;
		//getSteadyAttackOppNumByPri
		//getAttackOppNumByPri
	}
	for (int i = 0; i <Markcnt; i++) {
		
		int unsteady_num = DefenceInfo::Instance()->getAttackOppNumByPri(i);
		int steady_num = DefenceInfo::Instance()->getSteadyAttackOppNumByPri(i);
		//cout << "in2" << i << " 1" << endl;
		double un_pri = DefenceInfo::Instance()->getOppPlayerByNum(unsteady_num)->getThreatenPri();
		double un_value = DefenceInfo::Instance()->getOppPlayerByNum(unsteady_num)->getThreatenValue();
		double s_pri = DefenceInfo::Instance()->getOppPlayerByNum(steady_num)->getThreatenPri();
		double s_value = DefenceInfo::Instance()->getOppPlayerByNum(steady_num)->getThreatenValue();
		//cout <<"in3"<< i << " 2" << endl;
		if (un_pri > 2.5 || un_pri < 1.5)//leader?ò?????±
			continue;
		sum_unsteady += un_value;
		sum_steady += s_value;
	}
	//cout << sum_unsteady-sum_steady << endl;
	if (sum_unsteady - sum_steady >= 120) {
		if (lastFrame > 10) {
			lastFrame = 0;
			return true;
		}
		else
			lastFrame++;
	}
	else
		lastFrame = 0;
	return false;
}