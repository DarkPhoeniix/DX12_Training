
#include "Timer.h"

void Timer::Start()
{
    _startTime = std::chrono::high_resolution_clock::now();
}

void Timer::Stop()
{
    _endTime = std::chrono::high_resolution_clock::now();
    _elapsedTime = _endTime - _startTime;
}

bool Timer::IsRunning() const
{
    return false;
}

double Timer::GetElapsedNanoseconds() const
{
    return _elapsedTime.count();
}

double Timer::GetElapsedMicroseconds() const
{
    return _elapsedTime.count() * 1e-3;
}

double Timer::GetElapsedMilliseconds() const
{
    return _elapsedTime.count() * 1e-6;
}

double Timer::GetElapsedSeconds() const
{
    return _elapsedTime.count() * 1e-9;
}
