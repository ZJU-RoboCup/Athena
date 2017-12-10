/// FileName : 		SeriaPort.cpp
/// 				implementation file
/// Description :	serial handling
///	Keywords :		com send
/// Organization : 	ZJUNlict@Small Size League
/// Author : 		cliffyin
/// E-mail : 		cliffyin@zju.edu.cn
///					cliffyin007@gmail.com
/// Create Date : 	2011-07-25
/// Modified Date :	2011-07-25 
/// History :

#include ".\serialport.h"
#include "crc.h"
#include <process.h>
#include "commcontrol.h"

CSerialPort::CSerialPort(void)
{
	bInited=false;

	dcb.DCBlength = sizeof( DCB ) ;

	ovRead.hEvent=NULL;
	ovRead.Offset=0;
	ovRead.OffsetHigh=0;
	ovWrite.hEvent=NULL;
	ovWrite.Offset=0;
	ovWrite.OffsetHigh=0;
	ovWait.hEvent=NULL;
	ovWait.Offset=0;
	ovWait.OffsetHigh=0;

	hThreadStop=NULL;

	bReceiveFlag=false;
	memset(ReceiveBuff,0,BUFLEN);
	ReceiveBuffOffset=0;
}

CSerialPort::~CSerialPort(void)
{
	if (bInited) {
		ClosePort();
	}
}

bool CSerialPort::OpenPort(int port,DWORD rate,BYTE bytesize,BYTE stop,BYTE parity)
{
	BOOL bRes;

	if (bInited) {
		bRes = ClosePort();
		if (! bRes) {
			return false;
		}
	}

	// 创建事件
	ovRead.hEvent	=	CreateEvent(NULL,true,false,NULL);
	ovWrite.hEvent	=	CreateEvent(NULL,true,false,NULL);
	ovWait.hEvent	=	CreateEvent(NULL,true,false,NULL);
	hThreadStop		=	CreateEvent(NULL,true,false,NULL);

	// 创建对应的文件
	char       szPort[ 15 ] ;
	wsprintf( szPort, "COM%d", port);
	hComm = CreateFile( szPort, GENERIC_READ | GENERIC_WRITE,
		0,                    // exclusive access
		NULL,                 // no security attrs
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL| FILE_FLAG_OVERLAPPED,// 
		NULL );

	if (INVALID_HANDLE_VALUE == hComm) {
		return false;
	}

	// 串口参数设置
	bRes = SetCommMask(hComm,0);   //" EV_RXCHAR ) ;
	/*bRes=SetCommMask(hComm,EV_RXCHAR|EV_TXEMPTY|EV_CTS);*/
	if (! bRes) {
		return false;
	}

	bRes = SetupComm(hComm, 4096, 4096);
	if (! bRes) {
		return false;
	}

	bRes = PurgeComm(hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	if (! bRes) {
		return false;
	}

	COMMTIMEOUTS  CommTimeOuts ;
	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF ;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0 ;
	CommTimeOuts.ReadTotalTimeoutConstant = 5000 ;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0 ;
	CommTimeOuts.WriteTotalTimeoutConstant = 5000 ;

	bRes = SetCommTimeouts(hComm, &CommTimeOuts) ;
	if (! bRes) {
		return false;
	}

	GetCommState(hComm, &dcb);
	dcb.BaudRate = rate;
	dcb.ByteSize = bytesize ;
	dcb.Parity = parity;
	dcb.StopBits = stop;
	dcb.fOutxDsrFlow =0 ;
	dcb.fDtrControl = DTR_CONTROL_DISABLE ;
	dcb.fOutxCtsFlow = 0 ;
	dcb.fRtsControl = RTS_CONTROL_DISABLE ;
	dcb.fTXContinueOnXoff=true;
	dcb.fInX = dcb.fOutX = false ; 
	dcb.fNull=false;
	dcb.XonChar = ASCII_XON ;
	dcb.XoffChar = ASCII_XOFF ;
	dcb.XonLim = 100 ;
	dcb.XoffLim = 100 ;
	dcb.fBinary = TRUE ;
	dcb.fParity = TRUE ;

	bRes = SetCommState(hComm, &dcb) ; 

	if (! bRes) {
		CloseHandle(hComm);
		return false;
	}

	// 创建对应的线程
	//Create Thread for receive;
	hThread = (HANDLE)_beginthreadex( NULL, 0, &SerialPortWatch, this, 0, &_threadID );
	//hThread = CreateThread(NULL, 0, &SerialPortWatch, this, 0, &_threadID);

	bInited = true;
	return true;
}

bool CSerialPort::ClosePort()
{
	if (! bInited) {
		return false;
	}

	BOOL bRes;
	SetEvent(hThreadStop);

	bRes = SetCommMask(hComm, 0) ;
	if (! bRes) {
		return false;
	}

	bRes = EscapeCommFunction(hComm, CLRDTR) ;
	if(!bRes)
		return false;

	bRes = PurgeComm( hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR ) ;
	if (! bRes) {
		return false;
	}

	CloseHandle(hComm);
	CloseHandle(ovRead.hEvent);
	CloseHandle(ovWrite.hEvent);
	CloseHandle(ovWait.hEvent);
	CloseHandle(hThreadStop);

	bInited = false;
	return true;
}

bool CSerialPort::WriteBlock(BYTE *lpByte , DWORD dwBytesToWrite)
{
	DWORD nWritten;
	BOOL bRes;

	if (! bInited) {
		return false;
	}

	bRes = WriteFile(hComm, lpByte, dwBytesToWrite,&nWritten,&ovWrite);

	WaitForSingleObject(ovWrite.hEvent,500);
	GetOverlappedResult(hComm, &ovWrite, &nWritten, FALSE);

	return true;
}

unsigned __stdcall SerialPortWatch(void *pVoid)
//CThreadCreator::CallBackReturnType THREAD_CALLBACK SerialPortWatch(CThreadCreator::CallBackParamType pVoid)
{
	CSerialPort *pPort;
	pPort = (CSerialPort*)pVoid;

	DWORD nRes; 
	BOOL bWaitingOnStatusHandle;
	BOOL bWaitingOnRead;

	bWaitingOnStatusHandle = FALSE;
	BYTE buf;
	DWORD dwReadBytes;

	while (true)
	{
		// 若是结束线程事件触发，则跳出
		if (WAIT_OBJECT_0 == WaitForSingleObject(pPort->hThreadStop,0)) {
			break;
		}

		bWaitingOnRead = FALSE;
		// TODO, 检查是否有接收错误的时候

		if (! ReadFile(pPort->hComm, &buf,1, &dwReadBytes, &pPort->ovRead)) {
			if (GetLastError() != ERROR_IO_PENDING)	{
			} else {
				bWaitingOnRead = TRUE;
			}
		} else {
			pPort->GetBYTE(buf);
		}

		if (bWaitingOnRead) {
			nRes = WaitForSingleObject(pPort->ovRead.hEvent, -1);
			switch(nRes)
			{
				// Read completed.
			case WAIT_OBJECT_0:
				if (! GetOverlappedResult(pPort->hComm, &pPort->ovRead, &dwReadBytes, FALSE)) {
					printf("GetOverLapped\n");
					// Error in communications; report it.
				} else {
					pPort->GetBYTE(buf);
				}
				// Read completed successfully.

				// Reset flag so that another opertion can be issued.
				bWaitingOnRead = FALSE;
				break;

			case WAIT_TIMEOUT:
				break;

			default:
				break;
			}
		}
	}

	_endthreadex( 0 );

	return 0;
}

void CSerialPort::WaitEventProcess(DWORD dwEvtMask)
{
	DWORD dwOut;
	BYTE buf;

	if( dwEvtMask & EV_RXCHAR) {
		ReadFile(hComm,&buf,1,&dwOut,&ovRead);
		GetBYTE(buf);
	}

	return;
}

void CSerialPort::GetBYTE(BYTE in)
{
	// 接收机器人本体返回的数据
	if (! bReceiveFlag) {	// 头处理
		if (0xff == in) {
			//printf("%02X \t %d \t %d \n",in,bReceiveFlag,ReceiveBuffOffset);
			bReceiveFlag = 1;
			ReceiveBuffOffset = 1;
			memset(ReceiveBuff,0,BUFLEN);
			ReceiveBuff[0] = in;
		} else {
			return;
		}
	} else {				// 主体处理
		if (0xff == in) {
			if (ReceiveBuffOffset < BUFLEN-3) {
				bReceiveFlag = 1;
				ReceiveBuffOffset = 1;
				memset(ReceiveBuff,0,BUFLEN);
				ReceiveBuff[0] = in;
			} else {
				ReceiveBuff[ReceiveBuffOffset] = in;
				++ReceiveBuffOffset;
			}
		} else {
			ReceiveBuff[ReceiveBuffOffset] = in;
			
			++ReceiveBuffOffset;
		}

		// TODO, len, 接收完毕，触发事件，通知上层接收
		if (ReceiveBuffOffset >= pControl->recv_packet_len()) {
			bReceiveFlag = 0;
			ReceiveBuffOffset = 0;
			CSelfUnlockMutexLocker lock(&(pControl->m_mutex));
			memcpy(pControl->RXBuff, ReceiveBuff, pControl->recv_packet_len());
			SetEvent(pControl->hEventControlReceive);
		}
	}

	return ;
}
