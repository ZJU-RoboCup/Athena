#include "TimeCounter.h"
#include <windows.h>
#include <iostream>
#include <param.h>
/************************************************************************/
/*                          CUsecTimer                                  */
/************************************************************************/
CUsecTimer::CUsecTimer()
{
	LARGE_INTEGER frq;
	QueryPerformanceFrequency(&frq);
	_frequency = frq.QuadPart;
}

void CUsecTimer::start()
{
	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	_startCount = count.QuadPart;
}

void CUsecTimer::stop()
{
	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	_stopCount = count.QuadPart;
}
