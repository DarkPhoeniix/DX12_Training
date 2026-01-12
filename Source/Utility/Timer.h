#pragma once

#include <chrono>

class Timer
{
public:
    void Start();
    void Stop();

    bool IsRunning() const;

    double GetElapsedNanoseconds() const;
    double GetElapsedMicroseconds() const;
    double GetElapsedMilliseconds() const;
    double GetElapsedSeconds() const;

private:
    std::chrono::high_resolution_clock::time_point _startTime;
    std::chrono::high_resolution_clock::time_point _endTime;
    std::chrono::high_resolution_clock::duration _elapsedTime;
    bool _running;
};
