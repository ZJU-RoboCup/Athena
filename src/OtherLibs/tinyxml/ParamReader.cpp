/**
* @file ParamReader.cpp
* 此文件为读取参数的类的实现.
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
	// 指定保存文件名
	if (strFileName) { 
				// 保存参数到配置文件中，而且只保存改动过的
		saveParams(_headerPairList, strFileName, true); 	
	} else {	// 保存配置文件和备选文件
		saveParams(_defaultHeaderPairList, std::string(Param::File::ParamDir) + _paramFileName + "_dist.xml", false); // 保存参数到备份配置文件中
		saveParams(_headerPairList, std::string(Param::File::ParamDir) + _paramFileName + ".xml", false); // 保存参数到配置文件中	
	}

	return ;
}

void CParamReader::readParams(const char* strFileName)
{
	// 1.确定需要载入的外部配置文件
	std::string file_name;
	ParamStatus status;
	if (strFileName) {	// 不是缺省参数文件
		file_name = std::string(strFileName);
		status = eParamChanged;
	} else {			// 若是缺省参数文件，目前是这种方式，详见 "zeus_main.cpp"
		file_name = std::string(Param::File::ParamDir) + _paramFileName + ".xml";
		status = eParamDefault;
	}
	// 2.载入对应的外部配置文件
	// 参数以XML的格式存放在 params\PROJECT_NAME.xml 里面, PROJECT_NAME 是预先定义好的宏
	TiXmlDocument paramFile(file_name);
	if (! paramFile.LoadFile()) {
		std::cout << "Can not open parammeter file " << file_name << " or file isn't an xml format. " << Param::Output::NewLineCharacter;
		exit(-1);
		return;
	}
	TiXmlNode* projectHeaderNode = paramFile.FirstChild();
	bool hah = false;
	if (projectHeaderNode == NULL) hah = true;
	while (projectHeaderNode) {		// 工程名
		if (TiXmlNode::ELEMENT == projectHeaderNode->Type()) {
			TiXmlNode* paramHeaderNode = projectHeaderNode->FirstChild();
			while (paramHeaderNode) {	// 参数组
				if (TiXmlNode::ELEMENT == paramHeaderNode->Type()) {
					std::string paramHeader = paramHeaderNode->Value();
					
					TiXmlNode* paramNameNode = paramHeaderNode->FirstChild();
					while (paramNameNode) {	// 参数
						if (TiXmlNode::ELEMENT == paramNameNode->Type()) {
							std::string paramName = paramNameNode->Value();
							TiXmlNode* paramValueNode = paramNameNode->FirstChild();
							// 设定当前参数组中特定参数的值
							if (paramValueNode) {
								std::string paramValue = paramValueNode->Value();
								setParam(paramHeader, paramName, paramValue, status);
							}
						}
						// 移动到当前参数组的下一个参数
						paramNameNode = paramNameNode->NextSibling();
					}
				}

				// 移动到当前工程名的下一组参数
				paramHeaderNode = paramHeaderNode->NextSibling();
			}
		}

		// 移动到下一组工程名
		projectHeaderNode = projectHeaderNode->NextSibling();
	}

	return	;
}

void CParamReader::saveParams(const HeaderPairList& headerPairList, const std::string& file_name, bool onlyChanged)
{
	// 把所有参数保存到 params\PROJECT_NAME_dist.xml 里面, 方便修改
	std::ofstream outFile(file_name.c_str());
	outFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << Param::Output::NewLineCharacter;
	outFile << "<" << _paramFileName << ">" << Param::Output::NewLineCharacter;
	
	for (HeaderPairList::const_iterator header=headerPairList.begin(); header != headerPairList.end(); ++header) {
		// 看这个组里面有没有修改过的参数
		if (onlyChanged) {
			bool hasChangedParam = false;
			for (ParamPairList::const_iterator param=header->params.begin(); param != header->params.end(); ++ param) {
				if (eParamChanged == param->status) {
					hasChangedParam = true;
					break;
				}
			}
			
			// 跳过这一组
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
		// 找到的参数所在的类
		// 查找参数的位置,如果没有就添加一个
		ParamPairList::iterator param = std::find(header->params.begin(), header->params.end(), paramName);
		if (param != header->params.end()) {
			param->value = paramValue;
			param->status = status;
			// 这个参数以前已经存在了，可能是修改状态，需要同时改变与之相关的变量
			for (std::vector< AddressPair >::iterator v=param->vars.begin(); v!= param->vars.end(); ++v) {
				v->converter(paramValue, v->address);				
			}
		} else {
			header->params.push_back(ParamPair(paramName, paramValue, status));
		}
	} else {
		// 新加一个参数类
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