/************************************************************************/
/* file created by yys, 2014.04.02                                      */
/* 将发射机发的包记录到文件中                                              */
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
	bool checkSize(); // 若超过最大值则返回true
	long int getLen(std::fstream *fs);
	bool isLogging;
private:
	char *logName;
	std::fstream *log;
	int maxFileSize;  // 单个文件的大小，单位KB, 默认为200KB， 大概1分钟左右, 每秒3KB左右
	int size;
};

#endif