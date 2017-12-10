#ifndef _WE_ERROR_H_
#define _WE_ERROR_H_

#include <iostream>
class WEError { 
public:
	WEError(char* strFile,int nLine,char* strMsg) : _strFile(strFile),_nLine(nLine),_strMsg(strMsg) { } 
	void print() { std::cerr << _strFile << ':' << _nLine << ':' << _strMsg << std::endl; } // ���ش�����Ϣ
private:
	char* _strFile; // �����Դ�ļ���
	int	  _nLine;	// ������к�
	char* _strMsg ; // ���������Ϣ
};

#endif // _WE_ERROR_H_
