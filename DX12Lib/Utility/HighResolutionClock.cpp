#include "stdafx.h"

#include "HighResolutionClock.h"

HighResolutionClock::HighResolutionClock()
    : _deltaTime(0)
    , _totalTime(0)
{
    _startTime = std::chrono::high_resolution_clock::now();
}

void HighResolutionClock::tick()
{
    auto t1 = std::chrono::high_resolution_clock::now();
    _deltaTime = t1 - _startTime;
    _totalTime += _deltaTime;
    _startTime = t1;
}

void HighResolutionClock::reset()
{
    _startTime = std::chrono::high_resolution_clock::now();
    _deltaTime = std::chrono::high_resolution_clock::duration();
    _totalTime = std::chrono::high_resolution_clock::duration();
}

double HighResolutionClock::getDeltaNanoseconds() const
{
    return static_cast<double>(_deltaTime.count());
}
double HighResolutionClock::getDeltaMicroseconds() const
{
    return _deltaTime.count() * 1e-3;
}

double HighResolutionClock::getDeltaMilliseconds() const
{
    return _deltaTime.count() * 1e-6;
}

double HighResolutionClock::getDeltaSeconds() const
{
    return _deltaTime.count() * 1e-9;
}

double HighResolutionClock::getTotalNanoseconds() const
{
    return static_cast<double>(_totalTime.count());
}

double HighResolutionClock::getTotalMicroseconds() const
{
    return _totalTime.count() * 1e-3;
}

double HighResolutionClock::getTotalMilliSeconds() const
{
    return _totalTime.count() * 1e-6;
}

double HighResolutionClock::getTotalSeconds() const
{
    return _totalTime.count() * 1e-9;
}
