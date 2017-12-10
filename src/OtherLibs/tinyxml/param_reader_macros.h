#ifndef _UGLY_BUT_USEFUL_PARAM_READER_MACROS_H_
#define _UGLY_BUT_USEFUL_PARAM_READER_MACROS_H_

// Define the macro-group for param reading
#define DECLARE_PARAM_READER_BEGIN(ClassName) \
	static bool paramRead = false;\
	if (! paramRead) {\
		CParamReader* pReader = ParamReader::Instance();\
		std::string strClassName(#ClassName);
#define READ_PARAM(ParamName) \
		ParamName = pReader->getParam(strClassName, #ParamName, ParamName);
#define DECLARE_PARAM_READER_END \
		paramRead = true;\
	}

// Define the macro-group for param saving
#define DECLARE_PARAM_SAVER_BEGIN(ClassName) \
	{ \
		CParamReader* pReader = ParamReader::Instance();\
		std::string strClassName(#ClassName);
#define SAVE_PARAM(ParamName) \
		pReader->saveParam(strClassName, #ParamName, ParamName);
#define DECLARE_PARAM_SAVER_END \
		pReader->saveParams(); \
	}

#endif	// ~_UGLY_BUT_USEFUL_PARAM_READER_MACROS_H_
