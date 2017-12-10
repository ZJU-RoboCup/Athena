/**
* @file ParamReader.h
* 此文件为读取参数的类的声明.
* @date $Date: 2004/09/01 04:04:15 $
* @version $Revision: 1.12 $
* @author peter@mail.ustc.edu.cn
*/

#ifndef _PARAM_READER_H_
#define _PARAM_READER_H_

//////////////////////////////////////////////////////////////////////////
// include files
#include "param_reader_macros.h"		// Macros for handling the params
#include <param.h>

#include <singleton.h>					// Singleton model

#include <sstream>						// Stream for handling the strings
#include <iostream>

#include <string>						// STL
#include <vector>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////
// define the enumerator for the status of the params
// 参数的状态，分别表示是属于缺省参数文件，源代码编译得到，还是已经改变了
enum ParamStatus{ eParamNone, eParamDefault, eParamBuiltin, eParamChanged };

//////////////////////////////////////////////////////////////////////////
// define some classes/functions for handling params reading

/**
* ParamReader的用户界面,UI方式呈现
*/
class CParamReaderUI {
public:
	virtual void showParam(const std::string& paramHeader, const std::string& paramName, const std::string& paramValue, ParamStatus status) = 0;
};

/**
* 模板函数：字符串到变量的数值转换(数值型/布尔型/字符串型）
*/
template < typename ValueType >
void convertStringToValue(const std::string& str, void* a) 
{
	ValueType& v = *((ValueType*)a);
	std::stringstream valueStream;
	valueStream << str;
	valueStream >> v;
}

/**
* 函数指针：指向 字符串到变量的 数值转换函数
*/
typedef void (*Converter)(const std::string&, void*);

/**
* CParamReader.
* 读取程序参数
*/
class CParamReader {
	// 结构：参数地址 + 转换函数
	struct AddressPair {
		AddressPair(void* a, Converter c) : address(a),converter(c){ }
		void* address;						// 参数地址
		Converter converter;				// 转换函数
	};

	// 结构：参数名 + 参数值
	struct ParamPair {
		ParamPair(const std::string& k,const std::string& v, const ParamStatus& s) : key(k),value(v),status(s){ }
		bool operator == (const std::string& k) const{ return key == k; }
		std::string key;					// 参数键
		std::string value;					// 参数值
		ParamStatus status;					// 参数状态
		std::vector< AddressPair > vars;	// 参数相关变量对
	};
	typedef std::vector< ParamPair > ParamPairList;

	// 结构：参数类名 + 参数列表
	struct HeaderPair {
		HeaderPair(const std::string& k) : key(k){ }
		bool operator == (const std::string& k) const{ return key == k; }
		std::string key;					// 参数键
		ParamPairList params;				// 参数列表
	};
	typedef std::vector< HeaderPair > HeaderPairList;

public:
	// 构造函数
	CParamReader();

	// 析构函数
	~CParamReader();

	// 读取外部参数配置文件（XML）中的参数到内存
	void readParams(const char* strFileName = 0);

	// 保存内存中的参数到外部参数配置文件（XML）
	void saveParams(const char* strFileName = 0);

	// 注册用户界面,以UI的方式呈现，控制参数的UI读取与保存
	void registerUI(CParamReaderUI* pPRUI) { _pParamReaderUI = pPRUI; }

	// 模板函数，进行参数的获取
	template < typename ParamType >
	ParamType getParam(const std::string& paramHeader, const std::string& paramName, ParamType& defaultValue)
	{
		// 1.保存参数到缺省列表，作为默认备用
		addDefault(paramHeader, paramName, valueToString(defaultValue));

		// 2.找到参数头标识所在的参数组
		HeaderPairList& headerList = _headerPairList;
		HeaderPairList::iterator header = std::find(headerList.begin(),headerList.end(), paramHeader);
		// 如果已经找到的对应的参数组
		if (header != headerList.end()) { 
			// 3.找到参数名表示的特定参数
			ParamPairList& paramList = header->params;
			ParamPairList::iterator param = std::find(paramList.begin(),paramList.end(), paramName);
			// 如果已经找到该参数
			if (param != paramList.end()) {
				// 把参数字符串转化成所要的类型
				if (verboseMode) {
					std::cout << "Got Param " << paramHeader << "." << paramName << " value " << param->value << Param::Output::NewLineCharacter;
				}
				param->vars.push_back(AddressPair(&defaultValue, convertStringToValue< ParamType >));
				
				return stringToValue< ParamType >(param->value);
			}
		}

		// 没有找到这个参数
		if (verboseMode) {
			std::cout << "No such Param " << paramHeader << "." << paramName << Param::Output::NewLineCharacter;
		}
		setParam(paramHeader, paramName, valueToString(defaultValue), eParamBuiltin);
		
		return getParam(paramHeader, paramName, defaultValue);
	}

	// 模板函数，进行参数的保存
	template < typename ParamType >
	void saveParam(const std::string& paramHeader, const std::string& paramName, const ParamType& value)
	{
		setParam(paramHeader, paramName, valueToString(value), eParamChanged);
	}

	// 保存特定的参数组的特定的参数到内存
	void setParam(const std::string& paramHeader,const std::string& paramName,const std::string& paramValue, ParamStatus status);

protected:
	// 保存参数表到指定文件中
	void saveParams(const HeaderPairList& headerPairList, const std::string& fileName, bool onlyChanged); 
	
	// 模板函数：字符串转换到变量数值
	template < typename ParamType >
	ParamType stringToValue(const std::string& str)
	{
		ParamType returnValue;
		std::stringstream valueStream;
		valueStream << str;
		valueStream >> returnValue;
		return returnValue;
	}
	
	// 模板函数：变量数值到字符串
	template < typename ParamType >
	std::string valueToString(const ParamType& value)
	{
		// 把参数转化成字符串
		std::stringstream valueStream;
		valueStream << value;
		std::string strValue;
		valueStream >> strValue;
		return strValue;
	}

	// 增加默认参数值
	void addDefault(const std::string& paramHeader, const std::string& paramName, const std::string& paramValue)
	{
		// 保存到列表中
		HeaderPairList& headerList = _defaultHeaderPairList;
		HeaderPairList::iterator header = std::find(headerList.begin(),headerList.end(), paramHeader);
		if( header != headerList.end() ){ 
			// 找到的参数所在的类
			header->params.push_back(ParamPair(paramName, paramValue, eParamBuiltin));
		}else{
			// 新加一个参数类
			HeaderPair header_pair(paramHeader);
			header_pair.params.push_back(ParamPair(paramName, paramValue, eParamBuiltin));
			headerList.push_back(header_pair);
		}
	}

private:
	HeaderPairList _headerPairList;			///< 存放所有参数
	HeaderPairList _defaultHeaderPairList;	///< 缺省的参数
	static std::string _paramFileName;		///< 参数文件名
	bool verboseMode;						///< 调试输出开关
	CParamReaderUI* _pParamReaderUI;		///< 用户界面的指针
};

typedef NormalSingleton< CParamReader > ParamReader;

#define PARAM_READER ParamReader::Instance()

#endif	// ~_PARAM_READER_H_
