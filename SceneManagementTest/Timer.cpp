
#include "Timer.h"
void Timer :: StartTimer( ) {
    QueryPerformanceCounter(&(this->startCount)) ;
}

void Timer :: StopTimer() {
	QueryPerformanceCounter(&(this->stopCount)) ;
}

void Timer :: GetPerformanceCount(LARGE_INTEGER &perfCount)
{
	QueryPerformanceCounter(&(perfCount)) ;
}
/*-------64 bit double------------*/
double Timer :: PerfCountToSecsD( LARGE_INTEGER & perfCount) {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency( &frequency ) ;
	return ((double)perfCount.QuadPart /(double)frequency.QuadPart) ;
}
double Timer ::GetElapsedTimeD(const LARGE_INTEGER &count1 ,const LARGE_INTEGER &count2)
{
    LARGE_INTEGER time;
    time.QuadPart = count2.QuadPart - count1.QuadPart;
    return this->PerfCountToSecsD( time) ;
}
double Timer :: GetElapsedTimeD() {
	return this->GetElapsedTimeD(this->startCount , this->stopCount);
}

double Timer :: GetElapsedTimeD(const Timer &timer2) {
    return this->GetElapsedTimeD(this->startCount , timer2.stopCount);
}

/*-------32 bit float------------*/
float Timer :: PerfCountToSecsF( LARGE_INTEGER & perfCount) {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency( &frequency ) ;
	return ((float)perfCount.QuadPart /(float)frequency.QuadPart) ;
}
float Timer ::GetElapsedTimeF(const LARGE_INTEGER &count1 ,const LARGE_INTEGER &count2)
{
    LARGE_INTEGER time;
    time.QuadPart = count2.QuadPart - count1.QuadPart;
    return this->PerfCountToSecsF( time) ;
}
float Timer :: GetElapsedTimeF() {
	return this->GetElapsedTimeF(this->startCount , this->stopCount);
}

float Timer :: GetElapsedTimeF(const Timer &timer2) {
    return this->GetElapsedTimeF(this->startCount , timer2.stopCount);
}


