#ifndef _MULTI_THREAD_H_
#define _MULTI_THREAD_H_
#include <windows.h>
#include <exception>
#include <algorithm>
#include <MMSystem.h>
#pragma comment(lib,"winmm.lib")
class CMutex{
public:
	CMutex(){ InitializeCriticalSection(&_crtclSection); }
	~CMutex(){ DeleteCriticalSection(&_crtclSection); }
	void lock(){ EnterCriticalSection(&_crtclSection); }
	void unlock(){ LeaveCriticalSection(&_crtclSection); }
private:
	CRITICAL_SECTION _crtclSection;
};
class CSelfUnlockMutexLocker{
public:
	CSelfUnlockMutexLocker(CMutex* pMutex) : _pMutex(pMutex){ pMutex->lock(); }
	~CSelfUnlockMutexLocker() { _pMutex->unlock(); }
private:
	CMutex* _pMutex;
};
class CEvent{
public:
	CEvent(){ _event = CreateEvent(NULL, TRUE, FALSE, NULL); }
	~CEvent(){ CloseHandle(_event); }
	void wait(){ WaitForSingleObject(_event, INFINITE);}
	void set(){ SetEvent(_event); }
	void pulse(){ PulseEvent(_event); }
private:
	HANDLE _event;
};
#define THREAD_CALLBACK WINAPI
class CThreadCreator{
public:
	typedef DWORD CallBackReturnType; // 回调函数的返回类型
	typedef LPVOID CallBackParamType; // 回调函数的参数类型
	typedef CallBackReturnType (THREAD_CALLBACK *CallBackFunc)(CallBackParamType);
	CThreadCreator(CallBackFunc callBack, CallBackParamType param)
	{
		_thread = CreateThread(NULL, 0, callBack, param, 0, &_threadID);
		if( _thread == 0 ){
			throw std::exception("Can not create thread!");
		}
	}
	~CThreadCreator(){	}
private:
	HANDLE _thread;
	DWORD _threadID;
};
#define TIMER_CALLBACK CALLBACK
#define TIMER_FUNC_PARAM UINT, UINT, DWORD, DWORD, DWORD
class CTimerCreator{
public:
	typedef void CallBackReturnType;
	typedef CallBackReturnType (TIMER_CALLBACK *CallBackFunc)(TIMER_FUNC_PARAM);
	CTimerCreator(CallBackFunc callBack, int ms)
	{
		TIMECAPS tc;
		if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR) {
			throw std::exception("System does not support multimedia timer");
		}
		UINT     wTimerRes;
		wTimerRes = (std::min)((std::max)(tc.wPeriodMin, (UINT)1), tc.wPeriodMax);
		timeBeginPeriod(wTimerRes); 
		_timer = timeSetEvent(ms, wTimerRes, callBack, 0, TIME_PERIODIC);
	}
	~CTimerCreator(){ timeKillEvent(_timer); }
private:
	UINT _timer;
};
#endif // _MULTI_THREAD_H_
