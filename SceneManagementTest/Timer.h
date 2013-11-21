#ifndef _TIMER_H_
#define _TIMER_H_
#include <windows.h>
/*-----------------------------------------------------
this class can calculate time with very high precision
-----------------------------------------------------*/

class Timer{
public:
	Timer(bool start = false) {StartTimer();}

	const LARGE_INTEGER& GetStartCount()const  {return startCount;}
	const LARGE_INTEGER& GetStopCount()const  {return stopCount;}
	void StartTimer() ; //start timer
	void StopTimer() ; //stop timer
	void GetPerformanceCount(LARGE_INTEGER &perfCount);//get current performance count
	/*------64 bit floating point version-----------*/
	double GetElapsedTimeD(const LARGE_INTEGER &count1 ,const LARGE_INTEGER &count2);
	double GetElapsedTimeD() ;//get elapsed time (in seconds) between the time StopTimer() is called and the time StartTimer() is called
	double GetElapsedTimeD(const Timer &timer2) ;//get elapsed time (in seconds) between start time of this timer and stop time of timer <timer2>
	/*------32 bit floating point version-----------*/
	float GetElapsedTimeF(const LARGE_INTEGER &count1 ,const LARGE_INTEGER &count2);
	float GetElapsedTimeF() ;//get elapsed time (in seconds) between the time StopTimer() is called and the time StartTimer() is called
	float GetElapsedTimeF(const Timer &timer2) ;//get elapsed time (in seconds) between start time of this timer and stop time of timer <timer2>
private:
	LARGE_INTEGER startCount;
	LARGE_INTEGER stopCount;
	double PerfCountToSecsD( LARGE_INTEGER & perfCount) ;//convert performance count to seconds
	float PerfCountToSecsF( LARGE_INTEGER & perfCount) ;//convert performance count to seconds
} ;

#endif
