#ifndef _TIME_COUNTER_H_
#define _TIME_COUNTER_H_
/************************************************************************/
/*                       ΢�뼶�ļ�ʱ��                                 */
/************************************************************************/
class CUsecTimer{
public:
	CUsecTimer();
	void start();
	void stop();
	long time() { return (_stopCount - _startCount) * 1000000 / _frequency ; }
private:
	long long _frequency;
	long long _startCount;
	long long _stopCount;
};
#endif // _TIME_COUNTER_H_
