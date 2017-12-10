#include "AttributeSet.h"
#include <malloc.h>
#include "ClassFactory.h"
#include "VisionModule.h"

CAttributeSet::CAttributeSet(const CAttributeSet& attr)
{
	for (AttributeMap::const_iterator it = attr._attributeMap.begin() ; it != attr._attributeMap.end(); ++it)
	{
		_attributeMap.insert(AttributeMap::value_type(it->first, (CAttribute*)(ClassFactory::GetClassByName(it->first))));
	}
	_num = attr._num;
}

CAttributeSet::CAttributeSet()
{
	_attributeMap.clear();
	_num = 0;
}

CAttributeSet::~CAttributeSet()
{
	for (AttributeMap::iterator it = _attributeMap.begin() ; it != _attributeMap.end(); ++it)
	{
		delete (it->second);
	}
}

bool CAttributeSet::add(CAttribute* attr)
{
	string name = attr->getName();
	if (_attributeMap.end() != _attributeMap.find(name))
	{
		cout<<"ERROR: attributes have the same name!!!"<<endl;
		return false;
	}
	_attributeMap.insert(AttributeMap::value_type(name,(CAttribute*)(ClassFactory::GetClassByName(name))));

	if (_attributeMap.end() != _attributeMap.find(name))
	{
		return true;
	}
	return false;
}

bool CAttributeSet::remove(const string name)
{
	AttributeMap::iterator it =  _attributeMap.find(name);
	if (it != _attributeMap.end())
	{
		delete it->second;
		_attributeMap.erase(name);
		return true;
	}
	return false;
}

double CAttributeSet::getValue(const string name)
{
	AttributeMap::iterator it = _attributeMap.find(name);
	if (it == _attributeMap.end())
	{
		cout << "ERROR: wrong name of the attribute :  the name is  " << name <<"  !!!" <<endl;
		return 0.0;
	}
	return it->second->getValue();
}

void CAttributeSet::evaluate(const CVisionModule *pVision)
{
	//static int cnt = 0;
	for (AttributeMap::iterator it  = _attributeMap.begin(); it != _attributeMap.end(); it++)
	{
	//	std::cout<<pVision->Cycle()<<" cnt: "<<cnt<<std::endl;
		it->second->evaluate(pVision,_num);
	//	cnt++;
	}
}