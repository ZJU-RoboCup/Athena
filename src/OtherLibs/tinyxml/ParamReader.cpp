/**
* @file ParamReader.cpp
* ���ļ�Ϊ��ȡ���������ʵ��.
* @date $Date: 2004/09/01 04:04:15 $
* @version $Revision: 1.11 $
* @author peter@mail.ustc.edu.cn
*/

#include "ParamReader.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include "tinyxml.h"
#include <param.h>

namespace {
	const bool VERBOSE_MODE = false;
}

CParamReader::CParamReader() : verboseMode(VERBOSE_MODE), _pParamReaderUI(0)
{

}

CParamReader::~CParamReader()
{
	
}

void CParamReader::saveParams(const char* strFileName)
{
	// ָ�������ļ���
	if (strFileName) { 
				// ��������������ļ��У�����ֻ����Ķ�����
		saveParams(_headerPairList, strFileName, true); 	
	} else {	// ���������ļ��ͱ�ѡ�ļ�
		saveParams(_defaultHeaderPairList, std::string(Param::File::ParamDir) + _paramFileName + "_dist.xml", false); // ������������������ļ���
		saveParams(_headerPairList, std::string(Param::File::ParamDir) + _paramFileName + ".xml", false); // ��������������ļ���	
	}

	return ;
}

void CParamReader::readParams(const char* strFileName)
{
	// 1.ȷ����Ҫ������ⲿ�����ļ�
	std::string file_name;
	ParamStatus status;
	if (strFileName) {	// ����ȱʡ�����ļ�
		file_name = std::string(strFileName);
		status = eParamChanged;
	} else {			// ����ȱʡ�����ļ���Ŀǰ�����ַ�ʽ����� "zeus_main.cpp"
		file_name = std::string(Param::File::ParamDir) + _paramFileName + ".xml";
		status = eParamDefault;
	}
	// 2.�����Ӧ���ⲿ�����ļ�
	// ������XML�ĸ�ʽ����� params\PROJECT_NAME.xml ����, PROJECT_NAME ��Ԥ�ȶ���õĺ�
	TiXmlDocument paramFile(file_name);
	if (! paramFile.LoadFile()) {
		std::cout << "Can not open parammeter file " << file_name << " or file isn't an xml format. " << Param::Output::NewLineCharacter;
		exit(-1);
		return;
	}
	TiXmlNode* projectHeaderNode = paramFile.FirstChild();
	bool hah = false;
	if (projectHeaderNode == NULL) hah = true;
	while (projectHeaderNode) {		// ������
		if (TiXmlNode::ELEMENT == projectHeaderNode->Type()) {
			TiXmlNode* paramHeaderNode = projectHeaderNode->FirstChild();
			while (paramHeaderNode) {	// ������
				if (TiXmlNode::ELEMENT == paramHeaderNode->Type()) {
					std::string paramHeader = paramHeaderNode->Value();
					
					TiXmlNode* paramNameNode = paramHeaderNode->FirstChild();
					while (paramNameNode) {	// ����
						if (TiXmlNode::ELEMENT == paramNameNode->Type()) {
							std::string paramName = paramNameNode->Value();
							TiXmlNode* paramValueNode = paramNameNode->FirstChild();
							// �趨��ǰ���������ض�������ֵ
							if (paramValueNode) {
								std::string paramValue = paramValueNode->Value();
								setParam(paramHeader, paramName, paramValue, status);
							}
						}
						// �ƶ�����ǰ���������һ������
						paramNameNode = paramNameNode->NextSibling();
					}
				}

				// �ƶ�����ǰ����������һ�����
				paramHeaderNode = paramHeaderNode->NextSibling();
			}
		}

		// �ƶ�����һ�鹤����
		projectHeaderNode = projectHeaderNode->NextSibling();
	}

	return	;
}

void CParamReader::saveParams(const HeaderPairList& headerPairList, const std::string& file_name, bool onlyChanged)
{
	// �����в������浽 params\PROJECT_NAME_dist.xml ����, �����޸�
	std::ofstream outFile(file_name.c_str());
	outFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << Param::Output::NewLineCharacter;
	outFile << "<" << _paramFileName << ">" << Param::Output::NewLineCharacter;
	
	for (HeaderPairList::const_iterator header=headerPairList.begin(); header != headerPairList.end(); ++header) {
		// �������������û���޸Ĺ��Ĳ���
		if (onlyChanged) {
			bool hasChangedParam = false;
			for (ParamPairList::const_iterator param=header->params.begin(); param != header->params.end(); ++ param) {
				if (eParamChanged == param->status) {
					hasChangedParam = true;
					break;
				}
			}
			
			// ������һ��
			if (! hasChangedParam) {
				continue; 
			}
		}

		outFile << "\t<" << header->key << ">" << Param::Output::NewLineCharacter;
		for (ParamPairList::const_iterator param=header->params.begin(); param != header->params.end(); ++ param) {
			if (!(onlyChanged && param->status != eParamChanged)) {
				outFile << "\t\t<" << param->key << ">" << param->value << "</" << param->key << ">" << Param::Output::NewLineCharacter;
			}
		}
		outFile << "\t</" << header->key << ">" << Param::Output::NewLineCharacter;
	}
	outFile << "</" << _paramFileName << ">" << Param::Output::NewLineCharacter;

	return ;
}

void CParamReader::setParam(const std::string& paramHeader,const std::string& paramName,const std::string& paramValue, ParamStatus status)
{
	HeaderPairList& headerList = _headerPairList;
	HeaderPairList::iterator header = std::find(headerList.begin(),headerList.end(), paramHeader);
	if (header != headerList.end()) { 
		// �ҵ��Ĳ������ڵ���
		// ���Ҳ�����λ��,���û�о����һ��
		ParamPairList::iterator param = std::find(header->params.begin(), header->params.end(), paramName);
		if (param != header->params.end()) {
			param->value = paramValue;
			param->status = status;
			// ���������ǰ�Ѿ������ˣ��������޸�״̬����Ҫͬʱ�ı���֮��صı���
			for (std::vector< AddressPair >::iterator v=param->vars.begin(); v!= param->vars.end(); ++v) {
				v->converter(paramValue, v->address);				
			}
		} else {
			header->params.push_back(ParamPair(paramName, paramValue, status));
		}
	} else {
		// �¼�һ��������
		HeaderPair header_pair(paramHeader);
		header_pair.params.push_back(ParamPair(paramName, paramValue, status));
		headerList.push_back(header_pair);
	}

	if (verboseMode) {
		std::cout << "Set Param " << paramHeader << "." << paramName << " value " << paramValue << Param::Output::NewLineCharacter;
	}

	if (_pParamReaderUI) {
		_pParamReaderUI->showParam(paramHeader, paramName, paramValue, status);
	}

	return ;
}