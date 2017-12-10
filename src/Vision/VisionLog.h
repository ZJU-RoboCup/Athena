/************************************************************************/
/* file created by liyi, 2008.2.20                                      */
/************************************************************************/
#ifndef __VISION_LOG_H__
#define __VISION_LOG_H__

#include "atltime.h"
#include "atlstr.h"
#include <vector>
#include <ServerInterface.h>

class VisionLog
{
public:
	VisionLog(void);
	~VisionLog(void);

	//LOG���--����ͼ����
	FILE *m_pfLog;
	char *m_pLogName;
	bool m_bIsLogging;
	void createFile(void);
	void writeLog(FILE *fp,CServerInterface::VisualInfo msg);
	void addRecentLog(CServerInterface::VisualInfo msg);
	void writeRecentLog(void);
	void delEmptyFile(void);

	//�����ʱ���LOG
	FILE *m_pfRecentLog;
	char *m_pRecentLogName;
	long lCntOfLog;
	//CArray<CServerInterface::VisualInfo> m_arr_oRecentMsg;
	std::vector<CServerInterface::VisualInfo> m_arr_oRecentMsg[60 * 30];

	// ��켣��¼
	char *m_pBallTrackLogName;
	FILE *m_pBallTrackLog;
};

#endif