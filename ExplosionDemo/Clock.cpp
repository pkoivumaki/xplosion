#include "Clock.h"

#include <Windows.h>


Clock::Clock(void) :
	secPerCount(0),
		deltaTime(-1),
		time(0),
		speedMul(1),
		stopped(true),
		prevTime(0)
		
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	secPerCount = 1.0 / (double)countsPerSec;
	QueryPerformanceCounter((LARGE_INTEGER*)&prevTime);
}


Clock::~Clock(void)
{
}

float Clock::DeltaTime() const
{
	return static_cast<float>(deltaTime);
}

float Clock::TotalTime() const
{
	return static_cast<float>(time);
}


void Clock::Tick()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	if(!stopped)
	{
		deltaTime = (currTime-prevTime)*secPerCount;
		time += deltaTime;
		deltaTimeMul = deltaTime*speedMul;
	}
	prevTime = currTime;

	if (deltaTime<0)
		deltaTime = 0;

}

float Clock::DeltaTimeMul() const
{
	return (float)deltaTimeMul;
}

void Clock::SetSpeed( float speedMultiplier )
{
	speedMul = speedMultiplier;
}

void Clock::Start()
{
	if(stopped)
	{
		stopped			= false;
		deltaTime		= 0;
		deltaTimeMul	= 0;
		stopped			= false;
	}
}

void Clock::Stop()
{
	deltaTime			= 0;
	deltaTimeMul		= 0;
	stopped				= true;
}
