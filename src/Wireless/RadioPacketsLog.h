/************************************************************************/
/* file created by yys, 2014.04.02                                      */
/* ����������İ���¼���ļ���                                              */
/************************************************************************/
#ifndef __SAVE_TO_FILE__
#define __SAVE_TO_FILE__

#include <iostream>
#include <fstream>
#include "atltime.h"
#include "atlstr.h"

class CRadioPacketsLog {
public:
	CRadioPacketsLog();
	CRadioPacketsLog(int mfs);
	~CRadioPacketsLog();
	void writeLog(BYTE* str, std::streamsize size);
	void createFile();
	bool checkSize(); // ���������ֵ�򷵻�true
	long int getLen(std::fstream *fs);
	bool isLogging;
private:
	char *logName;
	std::fstream *log;
	int maxFileSize;  // �����ļ��Ĵ�С����λKB, Ĭ��Ϊ200KB�� ���1��������, ÿ��3KB����
	int size;
};

#endif