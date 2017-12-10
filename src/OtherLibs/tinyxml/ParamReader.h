/**
* @file ParamReader.h
* ���ļ�Ϊ��ȡ�������������.
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
// ������״̬���ֱ��ʾ������ȱʡ�����ļ���Դ�������õ��������Ѿ��ı���
enum ParamStatus{ eParamNone, eParamDefault, eParamBuiltin, eParamChanged };

//////////////////////////////////////////////////////////////////////////
// define some classes/functions for handling params reading

/**
* ParamReader���û�����,UI��ʽ����
*/
class CParamReaderUI {
public:
	virtual void showParam(const std::string& paramHeader, const std::string& paramName, const std::string& paramValue, ParamStatus status) = 0;
};

/**
* ģ�庯�����ַ�������������ֵת��(��ֵ��/������/�ַ����ͣ�
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
* ����ָ�룺ָ�� �ַ����������� ��ֵת������
*/
typedef void (*Converter)(const std::string&, void*);

/**
* CParamReader.
* ��ȡ�������
*/
class CParamReader {
	// �ṹ��������ַ + ת������
	struct AddressPair {
		AddressPair(void* a, Converter c) : address(a),converter(c){ }
		void* address;						// ������ַ
		Converter converter;				// ת������
	};

	// �ṹ�������� + ����ֵ
	struct ParamPair {
		ParamPair(const std::string& k,const std::string& v, const ParamStatus& s) : key(k),value(v),status(s){ }
		bool operator == (const std::string& k) const{ return key == k; }
		std::string key;					// ������
		std::string value;					// ����ֵ
		ParamStatus status;					// ����״̬
		std::vector< AddressPair > vars;	// ������ر�����
	};
	typedef std::vector< ParamPair > ParamPairList;

	// �ṹ���������� + �����б�
	struct HeaderPair {
		HeaderPair(const std::string& k) : key(k){ }
		bool operator == (const std::string& k) const{ return key == k; }
		std::string key;					// ������
		ParamPairList params;				// �����б�
	};
	typedef std::vector< HeaderPair > HeaderPairList;

public:
	// ���캯��
	CParamReader();

	// ��������
	~CParamReader();

	// ��ȡ�ⲿ���������ļ���XML���еĲ������ڴ�
	void readParams(const char* strFileName = 0);

	// �����ڴ��еĲ������ⲿ���������ļ���XML��
	void saveParams(const char* strFileName = 0);

	// ע���û�����,��UI�ķ�ʽ���֣����Ʋ�����UI��ȡ�뱣��
	void registerUI(CParamReaderUI* pPRUI) { _pParamReaderUI = pPRUI; }

	// ģ�庯�������в����Ļ�ȡ
	template < typename ParamType >
	ParamType getParam(const std::string& paramHeader, const std::string& paramName, ParamType& defaultValue)
	{
		// 1.���������ȱʡ�б���ΪĬ�ϱ���
		addDefault(paramHeader, paramName, valueToString(defaultValue));

		// 2.�ҵ�����ͷ��ʶ���ڵĲ�����
		HeaderPairList& headerList = _headerPairList;
		HeaderPairList::iterator header = std::find(headerList.begin(),headerList.end(), paramHeader);
		// ����Ѿ��ҵ��Ķ�Ӧ�Ĳ�����
		if (header != headerList.end()) { 
			// 3.�ҵ���������ʾ���ض�����
			ParamPairList& paramList = header->params;
			ParamPairList::iterator param = std::find(paramList.begin(),paramList.end(), paramName);
			// ����Ѿ��ҵ��ò���
			if (param != paramList.end()) {
				// �Ѳ����ַ���ת������Ҫ������
				if (verboseMode) {
					std::cout << "Got Param " << paramHeader << "." << paramName << " value " << param->value << Param::Output::NewLineCharacter;
				}
				param->vars.push_back(AddressPair(&defaultValue, convertStringToValue< ParamType >));
				
				return stringToValue< ParamType >(param->value);
			}
		}

		// û���ҵ��������
		if (verboseMode) {
			std::cout << "No such Param " << paramHeader << "." << paramName << Param::Output::NewLineCharacter;
		}
		setParam(paramHeader, paramName, valueToString(defaultValue), eParamBuiltin);
		
		return getParam(paramHeader, paramName, defaultValue);
	}

	// ģ�庯�������в����ı���
	template < typename ParamType >
	void saveParam(const std::string& paramHeader, const std::string& paramName, const ParamType& value)
	{
		setParam(paramHeader, paramName, valueToString(value), eParamChanged);
	}

	// �����ض��Ĳ�������ض��Ĳ������ڴ�
	void setParam(const std::string& paramHeader,const std::string& paramName,const std::string& paramValue, ParamStatus status);

protected:
	// ���������ָ���ļ���
	void saveParams(const HeaderPairList& headerPairList, const std::string& fileName, bool onlyChanged); 
	
	// ģ�庯�����ַ���ת����������ֵ
	template < typename ParamType >
	ParamType stringToValue(const std::string& str)
	{
		ParamType returnValue;
		std::stringstream valueStream;
		valueStream << str;
		valueStream >> returnValue;
		return returnValue;
	}
	
	// ģ�庯����������ֵ���ַ���
	template < typename ParamType >
	std::string valueToString(const ParamType& value)
	{
		// �Ѳ���ת�����ַ���
		std::stringstream valueStream;
		valueStream << value;
		std::string strValue;
		valueStream >> strValue;
		return strValue;
	}

	// ����Ĭ�ϲ���ֵ
	void addDefault(const std::string& paramHeader, const std::string& paramName, const std::string& paramValue)
	{
		// ���浽�б���
		HeaderPairList& headerList = _defaultHeaderPairList;
		HeaderPairList::iterator header = std::find(headerList.begin(),headerList.end(), paramHeader);
		if( header != headerList.end() ){ 
			// �ҵ��Ĳ������ڵ���
			header->params.push_back(ParamPair(paramName, paramValue, eParamBuiltin));
		}else{
			// �¼�һ��������
			HeaderPair header_pair(paramHeader);
			header_pair.params.push_back(ParamPair(paramName, paramValue, eParamBuiltin));
			headerList.push_back(header_pair);
		}
	}

private:
	HeaderPairList _headerPairList;			///< ������в���
	HeaderPairList _defaultHeaderPairList;	///< ȱʡ�Ĳ���
	static std::string _paramFileName;		///< �����ļ���
	bool verboseMode;						///< �����������
	CParamReaderUI* _pParamReaderUI;		///< �û������ָ��
};

typedef NormalSingleton< CParamReader > ParamReader;

#define PARAM_READER ParamReader::Instance()

#endif	// ~_PARAM_READER_H_
