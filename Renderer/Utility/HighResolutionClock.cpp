#include "stdafx.h"

#include "HighResolutionClock.h"

HighResolutionClock::HighResolutionClock()
    : _deltaTime(0)
    , _totalTime(0)
    , _startTime(std::chrono::high_resolution_clock::now())
{
}

void HighResolutionClock::Tick()
{
    auto t1 = std::chrono::high_resolution_clock::now();
    _deltaTime = t1 - _startTime;
    _totalTime += _deltaTime;
    _startTime = t1;
}

void HighResolutionClock::Reset()
{
    _startTime = std::chrono::high_resolution_clock::now();
    _deltaTime = std::chrono::high_resolution_clock::duration();
    _totalTime = std::chrono::high_resolution_clock::duration();
}

double HighResolutionClock::GetDeltaNanoseconds() const
{
    return static_cast<double>(_deltaTime.count());
}
double HighResolutionClock::GetDeltaMicroseconds() const
{
    return _deltaTime.count() * 1e-3;
}

double HighResolutionClock::GetDeltaMilliseconds() const
{
    return _deltaTime.count() * 1e-6;
}

double HighResolutionClock::GetDeltaSeconds() const
{
    return _deltaTime.count() * 1e-9;
}

double HighResolutionClock::GetTotalNanoseconds() const
{
    return static_cast<double>(_totalTime.count());
}

double HighResolutionClock::GetTotalMicroseconds() const
{
    return _totalTime.count() * 1e-3;
}

double HighResolutionClock::GetTotalMilliSeconds() const
{
    return _totalTime.count() * 1e-6;
}

double HighResolutionClock::GetTotalSeconds() const
{
    return _totalTime.count() * 1e-9;
}
