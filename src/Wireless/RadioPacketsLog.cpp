#include "RadioPacketsLog.h"
#include <string>
#include <sstream>

CRadioPacketsLog::CRadioPacketsLog() {
	isLogging = false; 
	log = new std::fstream;
	log->close();
	log->clear();
	maxFileSize = 1;
	size = maxFileSize * 1024;
}

CRadioPacketsLog::CRadioPacketsLog(int mfs = 1) {
	isLogging = false;
	log = new std::fstream;
	log->close();
	log->clear();
	maxFileSize = mfs;
	size = maxFileSize * 1024;
}

CRadioPacketsLog::~CRadioPacketsLog() {
	log->close(); 
	log->clear();
	delete log;
}

void CRadioPacketsLog::writeLog(BYTE* str, std::streamsize size) {
	if (log->is_open()) {
		log->write((char *)str, size);
	}
	else {
		std::cerr << "Error: can't open log file!" << std::endl;
		return;
	}
}

void CRadioPacketsLog::createFile() {
	log->close();
	log->clear();
	isLogging = true;
	CTime time = CTime::GetCurrentTime();
	CString strtime = time.Format("%Y%m%d%H%M%S");
	CString str = ".\\params\\RadioPacketsLog\\" + strtime + ".log";
	logName = str.GetBuffer();
	log->open(logName, std::fstream::in | std::fstream::out | std::fstream::app | std::fstream::binary);
}

bool CRadioPacketsLog::checkSize() {
	if (log->is_open()) {
		// std::cout << getLen(log) << std::endl;
		if (getLen(log) >= size) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		std::cerr << "error: can't open log file!" << std::endl;
		return false;
	}
}

long int CRadioPacketsLog::getLen(std::fstream *fs) {
	fs->seekg(0, std::fstream::end);
	long int len = fs->tellg();
	return len;
}