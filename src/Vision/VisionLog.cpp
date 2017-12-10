/************************************************************************/
/* file created by liyi, 2008.2.20                                      */
/************************************************************************/
#include "visionlog.h"
#include <utils.h>
#include <playmode.h>
#include <tinyxml/ParamReader.h>

using namespace std;
namespace{
	const int _RecentLogLength =  30; //seconds
	bool IS_SIMULATION = false;
	bool IS_TRACK_BALL = false;
	float InfoX2Msg(float x){ return x*10; }
	float InfoY2Msg(float y){ return -y*10;}
	float InfoAng2Msg(float ang){return -Utils::Rad2Deg(ang); }
}

VisionLog::VisionLog(void)
{
	DECLARE_PARAM_READER_BEGIN(General)
	READ_PARAM(IS_SIMULATION)
	DECLARE_PARAM_READER_END
	m_bIsLogging = false;
}

VisionLog::~VisionLog(void)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
//编写者：yg              
//创建时间：07.7.27
//功能： 写LOG
//输入参数：FILE *fp,Message msg
//输出参数：
//返回值：
//备注：Message中bool类型的变量没有写入，在Log播放器中可通过坐标值判断 -1000为无效
//历史：如果其他人对函数做了修改，最好在此标注一下
/////////////////////////////////////////////////////////////////////////////////////////

// modified by zhyaic 2013.5.13
void VisionLog::writeLog(FILE *fp,CServerInterface::VisualInfo msg)
{
	int i = 0;
	const short RobotAmount = Param::Field::MAX_PLAYER;
	const int nInvalidDir = -1000;
	const float fInvalidXY = -10000;
	float tempx,tempy;

	int tmp = sizeof(char);
	fwrite(&(playModePair[msg.mode].ch), sizeof(char), 1, fp); //李毅：写入当前帧的裁判盒指令
	
	if( msg.ball.valid )
	{ 
		tempx = InfoX2Msg(msg.ball.x);
		tempy = InfoY2Msg(msg.ball.y);

		fwrite(&tempx, sizeof(float),1,fp);
		fwrite(&tempy, sizeof(float),1,fp);
		fwrite(&(msg.imageBall.valid),sizeof(int),1,fp);
		fwrite(&(msg.imageBall.x),sizeof(int),1,fp);
		fwrite(&(msg.imageBall.y),sizeof(int),1,fp);
	}
	else
	{
		fwrite(&fInvalidXY,sizeof(float),1,fp);
		fwrite(&fInvalidXY,sizeof(float),1,fp);
		fwrite(&msg.imageBall.valid,sizeof(int),1,fp);
		fwrite(&nInvalidDir,sizeof(int),1,fp);
		fwrite(&nInvalidDir,sizeof(int),1,fp);
	}

	int temp;
	for(i=0;i<RobotAmount;i++)
	{
		//写我方车
		BYTE k = i + 1; 
		if (IS_SIMULATION)
			fwrite(&k, sizeof(BYTE), 1, fp);
		else
			fwrite(&(msg.ourRobotIndex[i]),sizeof(BYTE),1,fp);

		if ( msg.player[i].pos.valid )
		{
			tempx = InfoX2Msg(msg.player[i].pos.x);
			tempy = InfoY2Msg(msg.player[i].pos.y);

			fwrite(&tempx, sizeof(float),1,fp);
			fwrite(&tempy, sizeof(float),1,fp);
			temp = (int)(InfoAng2Msg(msg.player[i].angle));
			fwrite(&temp,sizeof(int),1,fp);
		}
		else
		{
			fwrite(&fInvalidXY,sizeof(float),1,fp);
			fwrite(&fInvalidXY,sizeof(float),1,fp);
			fwrite(&nInvalidDir,sizeof(int),1,fp);
		}

		//写敌方车 
		BYTE j = i + 1; 
		fwrite(&j,sizeof(BYTE),1,fp);
		if ( msg.player[i+RobotAmount].pos.valid )
		{
			tempx = InfoX2Msg(msg.player[i+RobotAmount].pos.x);
			tempy = InfoY2Msg(msg.player[i+RobotAmount].pos.y);

			fwrite(&tempx, sizeof(float), 1, fp);
			fwrite(&tempy, sizeof(float), 1, fp);
			temp = (int)(InfoAng2Msg(msg.player[i+RobotAmount].angle));
			fwrite(&temp,sizeof(int),1,fp);
		}
		else
		{
			fwrite(&fInvalidXY,sizeof(float),1,fp);
			fwrite(&fInvalidXY,sizeof(float),1,fp);
			fwrite(&nInvalidDir,sizeof(int),1,fp);
		}
	}

	// 记录的球的轨迹
	if ( IS_TRACK_BALL && m_pBallTrackLog ){
		fprintf(m_pBallTrackLog, "%d ", msg.ball.valid);
		fprintf(m_pBallTrackLog, "%.2f %.2f\n", msg.ball.x, msg.ball.y);
		//fprintf(m_pBallTrackLog, "%.2f, %.2f ", msg.imageBall.x, msg.imageBall.y);
		//fprintf(m_pBallTrackLog, "%d\n", msg.imageBall.valid);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//编写者： yg             
//创建时间：07.10.10
//功能： 写最近的LOG，
//输入参数：
//输出参数：
//返回值：
//备注：
//历史：如果其他人对函数做了修改，最好在此标注一下
/////////////////////////////////////////////////////////////////////////////////////////
void VisionLog::writeRecentLog(void)
{	
	long i = lCntOfLog;
	int nCnt = 0;

	rewind(m_pfRecentLog);
	while ( nCnt < _RecentLogLength * 60 )
	{
//		writeLog(m_pfRecentLog,m_arr_oRecentMsg[i]);
		i = (i + 1) % (_RecentLogLength * 60);
		nCnt++;
	}
	fflush(m_pfRecentLog);
}

/////////////////////////////////////////////////////////////////////////////////////////
//编写者： yg             
//创建时间：07.10.10
//功能： 删除无用的LOG文件，
//输入参数：
//输出参数：
//返回值：
//备注：
//历史：如果其他人对函数做了修改，最好在此标注一下
/////////////////////////////////////////////////////////////////////////////////////////
void VisionLog::delEmptyFile(void)
{
	//int nFileLen = 0;
	//FILE *fp;
	if ( ftell(this->m_pfLog) == 0 )
	{
		fclose(this->m_pfLog);
		remove(this->m_pLogName);
	}
	else
		fclose(this->m_pfLog);
	if ( ftell(this->m_pfRecentLog) == 0 )
	{
		fclose(this->m_pfRecentLog);
		remove(this->m_pRecentLogName);
	}
	else
		fclose(this->m_pfRecentLog);

	if (IS_TRACK_BALL){
		if ( ftell(this->m_pBallTrackLog) == 0 ){
			fclose(this->m_pBallTrackLog);
			remove(this->m_pBallTrackLogName);
		}
		else
			fclose(this->m_pBallTrackLog);
	}
}

void VisionLog::createFile(void)
{
	m_bIsLogging = true;
	CTime time = CTime::GetCurrentTime();
	CString strtime = time.Format("%Y%m%d%H%M");
	CString str = ".\\LOG\\"+strtime+".log";
	//CString strRct = ".\\LOG\\Recent"+strtime+".log";
	CString strBallTrack = ".\\LOG\\BallTrack" + strtime + ".txt";
	m_pLogName = str.GetBuffer();
	//m_pRecentLogName = strRct.GetBuffer();
	m_pBallTrackLogName = strBallTrack.GetBuffer();

	m_pfLog = fopen(m_pLogName,"wb");
	//m_pfRecentLog = fopen(m_pRecentLogName,"wb");
	if ( IS_TRACK_BALL )
		m_pBallTrackLog = fopen(m_pBallTrackLogName, "w");

	//m_arr_oRecentMsg.SetSize(60 * _RecentLogLength);
	lCntOfLog = 0;
}

void VisionLog::addRecentLog(CServerInterface::VisualInfo msg)
{
	//m_arr_oRecentMsg[lCntOfLog] = msg;
	lCntOfLog = (lCntOfLog + 1) % (60 * _RecentLogLength); //暂时只写半分钟
}