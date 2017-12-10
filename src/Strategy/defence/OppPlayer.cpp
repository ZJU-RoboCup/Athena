#include "OppPlayer.h"
#include "OppRoleMatcher.h"
#include "OppRoleFactory.h"
#include "OppRole.h"
#include "AttributeSet.h"

COppPlayer::COppPlayer()
{
	_num = 0;
	_attrSetP = NULL;
	_oppRole = new RNull();
}

COppPlayer::~COppPlayer()
{
	delete _attrSetP;
	delete _oppRole;
}

string COppPlayer::getRoleName()
{
	return _oppRole->getName();
}

int COppPlayer::getThreatenPri()
{
	return _oppRole->getThreatenPri();
}
double COppPlayer::getThreatenValue()
{
	return _oppRole->getSubValue();
}

double COppPlayer::getAttributeValue(string name)
{
	return _attrSetP->getValue(name);
}

void COppPlayer::setProperty(const int num,const CAttributeSet& attrSet)
{
	delete _attrSetP;
	_num = num;
	_attrSetP = new CAttributeSet(attrSet);
	_attrSetP->setNum(_num);
}

void COppPlayer::evaluate(const CVisionModule* pVision)
{
	_attrSetP->evaluate(pVision);
}

void COppPlayer::matchRole()
{
	_oppRole = OppRoleMatcher::Instance()->matchRole(*this);
}

bool COppPlayer::isTheRole(const string name)
{
	if (name == _oppRole->getName())
	{
		return true;
	}
	for (OppRoleMap::iterator it = OppRoleFactory::Instance()->get()->begin();it != OppRoleFactory::Instance()->get()->end();++it)
	{
		if (name == it->first)
		{
			return false;
		}
	}
	cout<<"ERROR: There is no Role called "<< name <<" !!!"<<endl;
	return false;
}