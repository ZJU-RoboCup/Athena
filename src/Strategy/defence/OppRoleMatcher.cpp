#include "OppRoleMatcher.h"
#include "OppPlayer.h"
#include "ClassFactory.h"
#include "OppRoleFactory.h"
#include "BestPlayer.h"
#include <fstream>
#include <sstream>

namespace{
	const string CONFIG_PATH = "defence_config\\";
	bool output_num = true;
}
COppRoleMatcher::COppRoleMatcher()
{
	_oppRolePriList.clear();
	for (int i = 0 ; i <= Param::Field::MAX_PLAYER ; ++i)_role[i] = new RNull();//实际上只用到编号的1~6
	OppRoleFactory::Instance();
	readInConfig("TestOppRoleMatch.txt");
}

COppRoleMatcher::~COppRoleMatcher()
{
	for (OppRoleVector::iterator it = _oppRolePriList.begin(); it != _oppRolePriList.end(); ++it)
	{
		delete *it;
	}
	delete []_role;
}

void COppRoleMatcher::readInConfig(std::string fileName)
{
	string fullFileName = CONFIG_PATH + fileName;
	ifstream infile(fullFileName.c_str());
	if (!infile)
	{
		cerr << "ERROR: there is no config file named " << fullFileName << endl;
		exit(-1);
	}
	for (OppRoleVector::iterator it = _oppRolePriList.begin(); it != _oppRolePriList.end(); ++it)
	{
		delete *it;
	}
	_oppRolePriList.clear();
	double mustIn,canIn,canOut,mustOut;
	int attack_threaten = PRIORITY_ATTACK;
	int defend_threaten = PRIORITY_DEFENCE;
	string line;
	string word;
	string name;
	while (getline(infile,line)){
		istringstream lineStream(line);
		lineStream;
		lineStream >> word >> name >> mustIn >> canIn >> canOut >> mustOut;
		OppRoleMap::iterator it = OppRoleFactory::Instance()->get()->find(name);
		if (OppRoleFactory::Instance()->get()->end() != it)
		{
			COppRole* role = (COppRole*)(ClassFactory::GetClassByName(name));
			bool roleRepeated = false;
			//查看role是否重复
			for (OppRoleVector::iterator it2 = _oppRolePriList.begin(); it2 != _oppRolePriList.end() ; it2++)
			{
				if (role->getName() == (*it2)->getName())
				{
					roleRepeated = true;
					break;
				}
			}
			if (false == roleRepeated)
			{
				if ("ATTACKER" == word)
				{
					role->SetProperty(attack_threaten,mustIn,canIn,canOut,mustOut);
					++attack_threaten;
				} else if ("DEFENDER" == word)
				{
					role->SetProperty(defend_threaten,mustIn,canIn,canOut,mustOut);
					++defend_threaten;
				}	
				_oppRolePriList.push_back(role);
			}else{
				cout << "ERROR: the role named " << name <<" has already been in role list " << endl;
			}
		} else {
			cout << "ERROR: there is not a role named " << name << "!!!" << endl;
		}
	}
}

//这个切换规则可以再讨论
COppRole* COppRoleMatcher::matchRole(COppPlayer& oplayer)
{
	int num = oplayer.getNum();
	//wrong number
	if (num < 1 || num > Param::Field::MAX_PLAYER)
	{
		cout << "ERROR: wrong num with opp role match!!!" << endl;
		return NULL;
	}
	//first calc
	if (Debug::isWrite&&output_num) {
		if (!Debug::File.is_open()) {
			Debug::File.open("MarkInfo.txt",ios::out);
		}
		Debug::File << num<<" ";
	}
	//cout << num <<" "<<BestPlayer::Instance()->getTheirBestPlayer() << endl;
	_role[num]->calcSubValue(oplayer.getAttr());//因为subValue不允许直接赋值，这里就计算一次
	//如果高于当前role中有一个mustChangeIn的，则change
	for (OppRoleVector::iterator it = _oppRolePriList.begin() ; it != _oppRolePriList.end() ; ++it)
	{
		(*it)->calcSubValue(oplayer.getAttr());//对一辆车只计算一次即可
		if ((*it)->mustChangeIn() && (*it)->getName() != _role[num]->getName())
		{
			if ((*it)->getThreatenPri() <= _role[num]->getThreatenPri() || !_role[num]->canChangeIn())
			{
				return change(it,num);
			}			
		}
	}
	//如果优先级首轮抢占没有结果
	if (false == _role[num]->mustChangeIn())
	{
		if (_role[num]->mustChangeOut())
		{
			//如果有canChangeIn的，则change
			for (OppRoleVector::iterator it = _oppRolePriList.begin() ; it != _oppRolePriList.end() ; ++it)
			{
				if ((*it)->canChangeIn())
				{
					return change(it,num);
				}
			}
			//如果连一个canChangeIn的都没有，则挑个canChangeOut以上的就change
			for (OppRoleVector::iterator it = _oppRolePriList.begin() ; it != _oppRolePriList.end() ; ++it)
			{
				if (!(*it)->canChangeOut())
				{
					return change(it,num);
				}
			}
			//如果连一个canChangeOut以上的都没有，则挑个不是mustChangeOut的就change
			for (OppRoleVector::iterator it = _oppRolePriList.begin() ; it != _oppRolePriList.end() ; ++it)
			{
				if (!(*it)->mustChangeOut())
				{
					return change(it,num);
				}
			}
		} 
		else if (_role[num]->canChangeOut())
		{
			//如果有canChangeIn的，则change
			for (OppRoleVector::iterator it = _oppRolePriList.begin() ; it != _oppRolePriList.end() ; ++it)
			{
				if ((*it)->canChangeIn())
				{
					return change(it,num);
				}
			}
		}
	}
	return _role[num];
}

COppRole* COppRoleMatcher::change(OppRoleVector::iterator it, int num)
{
	delete _role[num];
	_role[num] = (COppRole*)(ClassFactory::GetClassByName((*it)->getName()));
	_role[num]->SetProperty((*it)->_threatenPri,(*it)->_mustChangeIn,(*it)->_canChangeIn,(*it)->_canChangeOut,(*it)->_mustChangeOut);
	_role[num]->_subValue = (*it)->_subValue;
	return _role[num];
}





