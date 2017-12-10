#include "OppRoleFactory.h"
#include "OppPlayer.h"
#include "AttributeSet.h"
#include "PlayInterface.h"
#include "TaskMediator.h"
#include "defence/DefenceInfo.h"//这里最好不用include这个
#include "WorldModel.h"//球速等数据，求助于worldModel
#include <math.h>
#include <iostream>
using namespace std;
namespace{
	bool output_value = true;
	//定义这里边所需要的静态变量和常量
};

///>角色定义
//特定角色 NULL_ROLE
DEFINE_CLASS_EX_CPP(RNull,PRIORITY_LAST,0.9,0.6,0.4,0.1)
//测试的oppRole
DEFINE_CLASS_EX_CPP(RTest,PRIORITY_ATTACK,0.9,0.6,0.4,0.1)
DEFINE_CLASS_EX_CPP(RTest2,PRIORITY_ATTACK,0.9,0.6,0.4,0.1)
DEFINE_CLASS_EX_CPP(RTest3,PRIORITY_ATTACK,0.9,0.6,0.4,0.1)
DEFINE_CLASS_EX_CPP(RLeader,PRIORITY_ATTACK,0.9,0.6,0.4,0.1)
DEFINE_CLASS_EX_CPP(RReceiver,PRIORITY_ATTACK,0.9,0.6,0.4,0.1)
DEFINE_CLASS_EX_CPP(RSoldier,PRIORITY_ATTACK,0.9,0.6,0.4,0.1)
DEFINE_CLASS_EX_CPP(RAttacker,PRIORITY_ATTACK,0.9,0.6,0.4,0.1)
DEFINE_CLASS_EX_CPP(RGoalie,PRIORITY_DEFENCE,0.9,0.6,0.4,0.1)
DEFINE_CLASS_EX_CPP(RDefender,PRIORITY_DEFENCE,0.9,0.6,0.4,0.1)
DEFINE_CLASS_EX_CPP(RRusherFront,PRIORITY_ATTACK,0.9,0.6,0.4,0.1)
DEFINE_CLASS_EX_CPP(RRusherBack,PRIORITY_DEFENCE,0.9,0.6,0.4,0.1)

COppRoleFactory::COppRoleFactory()
{
	_oppRoleBackMap = new OppRoleMap();
	_oppRoleBackMap->clear();
	configuration();
}

COppRoleFactory::~COppRoleFactory()
{
	for (OppRoleMap::iterator it = _oppRoleBackMap->begin(); it != _oppRoleBackMap->end() ; ++it)
	{
		delete it->second;
	}
	delete _oppRoleBackMap;
}

void COppRoleFactory::configuration()
{
	//将所有角色预添加入
	COppRole *role = new RTest();
	_oppRoleBackMap->insert(OppRoleMap::value_type(role->getName(),role));
	role = new RTest2();
	_oppRoleBackMap->insert(OppRoleMap::value_type(role->getName(),role));
	role = new RTest3();
	_oppRoleBackMap->insert(OppRoleMap::value_type(role->getName(),role));
	role = new RLeader();
	_oppRoleBackMap->insert(OppRoleMap::value_type(role->getName(),role));
	role = new RReceiver();
	_oppRoleBackMap->insert(OppRoleMap::value_type(role->getName(),role));
	role = new RSoldier();
	_oppRoleBackMap->insert(OppRoleMap::value_type(role->getName(),role));
	role = new RAttacker();
	_oppRoleBackMap->insert(OppRoleMap::value_type(role->getName(),role));
	role = new RGoalie();
	_oppRoleBackMap->insert(OppRoleMap::value_type(role->getName(),role));
	role = new RDefender();
	_oppRoleBackMap->insert(OppRoleMap::value_type(role->getName(),role));
}

//NULL
CALC_ROLE(RNull)
{
	setSubValue(0.0);
}

//for test
CALC_ROLE(RTest)
{
	double t = attrSet->getValue("ATest");
	setSubValue(t);
}

CALC_ROLE(RTest2)
{
	double t = attrSet->getValue("ATest2");
	setSubValue(t);
}

CALC_ROLE(RTest3)
{
	double t = attrSet->getValue("ADist2Ball");
	setSubValue(t);
}

//CALC_ROLE(aaaaaaaaaaaaaaaa)
//{
//	double t = attrSet->getValue("ADist2Ball");
//	setSubValue(t);
//}

CALC_ROLE(RLeader)
{
	double t = 0;
	t = attrSet->getValue("ACanBeLeader");
	//
	//t=attrSet->getValue("ABestPlayer");
	//cout << t << endl;
	setSubValue(t);//0.9...
}

CALC_ROLE(RReceiver)
{
	double t = 0;
	double touch = attrSet->getValue("ATouchAbility");
	double chase = attrSet->getValue("AChaseAbility");
	double reflect = attrSet->getValue("AReceiveReflectAngle");
	reflect = reflect < 0.7 ? 0.7 : reflect * 1.5;
	touch = 600 - touch;
	if (chase >590)
	{
		t = chase;
	}else t = touch / reflect;
	setSubValue(t);
}

CALC_ROLE(RAttacker)
{
	double t = 0;
	double posX = attrSet->getValue("APosX");
	double shootRange = attrSet->getValue("AShootRangeWithoutMarker");
	double fillingInDist = attrSet->getValue("AFillingInDist");
	double keepFaceAngle = attrSet->getValue("AKeeperDirFactor");
	double reflectAngle = attrSet->getValue("AShootReflectAngle");
	double passBlock = attrSet->getValue("APassBlock");
	double velX = attrSet->getValue("AVelX");
	double dist2ball = attrSet->getValue("ADist2Ball");
	double immortalFactor = attrSet->getValue("AImmortalFactor");
	reflectAngle = reflectAngle < 0.7 ? 0.7 : reflectAngle;
	reflectAngle = reflectAngle > 2.3 ? reflectAngle * 5.0 : reflectAngle;//折射角整定，小于45度时按照45度整定
	double posX2 = posX + velX * 0.5;
	double t4 = (-posX2 + 450) / 900.0;//t4 : 0~1,进攻方向最大,t4参数首次计算 原300 600 gty 2017.7
	reflectAngle = t4 < 0.25 ? reflectAngle + 5 : reflectAngle;
	double t11 = (t4 < 0.35) ? 1.5 : 1.0;	
	double t10 = -velX * t11;//这两行代码令defender和attacker之间的切换变得敏感 add
	//t4变换：
	double t5 = t4 < 0.6 ? (t4 / 0.6)*(t4 / 0.6) : 1;//原0.35 gty 2017.7
	t5 = t4 > 0.9 ? 1.9 - t4 : t5;
	//使用t5
	double t1 = shootRange * 3.5 * t4;
	double t2 = fillingInDist < 35 ? fillingInDist * 0.7 : fillingInDist;
	
	double t5new = -0.8 / (pow(250, 2))*(pow(posX, 2)) - 0.8 / 125 * posX + 0.2;
	if (posX > 29.5) t5new = 0;
	t2 = t2 * t5new * 0.9;//原 t2 = t2 * t5 * 0.9;
	double t3 = 90 / (keepFaceAngle + 1.0);
	t3 = t5 * t3;
	double t6 = 210 / (reflectAngle + 0.35);
	t6 = t6 * t5;
	double t7 = passBlock * t5;
	double t8 = (500 - dist2ball) * t5 * 0.3; //*0.1

	t = t1 + t2 + t3 + t6 + t7 + t8 + t10 + immortalFactor;
	if (Debug::isWrite&&output_value) {
		if (!Debug::File.is_open()) {
			Debug::File.open("MarkInfo.txt",std::ios::out);
		}
		Debug::File <<t<<"="<<t1<<"+" << t2 << "+" << t3 << "+" << t6 << "+" << t7 << "+" << t8 << "+" << t10 << "+" << immortalFactor <<std::endl;
	}
	setSubValue(t);
}

CALC_ROLE(RSoldier)
{
	double t = 0;
	double t1 = attrSet->getValue("APosX");
	double t2 = attrSet->getValue("AVelX");
	double t3 = attrSet->getValue("AKeeperDirFactor");
	double t4 = attrSet->getValue("ADist2Ball");
	t1 = t1 + t2 * 0.7;
	t1 = 300.0 - t1;//t1 range : 0~600 
	t3 = 300.0 / (t4*t3+1);
	t = t1+t3;
	setSubValue(t);//290 210 160 110
}

CALC_ROLE(RGoalie)//暂时这样判断，以后要改掉！！
{
	double t = attrSet->getValue("AGoalie");
	setSubValue(t);//0.9...
}

CALC_ROLE(RDefender)//优先级在goalie之后！！
{
	double t = 0;
	double dist2theirGoal = attrSet->getValue("ADist2TheirGoal");
	t = 200/(dist2theirGoal+0.01);
	setSubValue(t);//1.5 1.2 1.0 0.8
}

CALC_ROLE(RRusherFront)
{

}

CALC_ROLE(RRusherBack)
{

}
