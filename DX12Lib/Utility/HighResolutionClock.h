#pragma once

class HighResolutionClock
{
public:
    HighResolutionClock();

    void Tick();
    void Reset();

    double GetDeltaNanoseconds() const;
    double GetDeltaMicroseconds() const;
    double GetDeltaMilliseconds() const;
    double GetDeltaSeconds() const;

    double GetTotalNanoseconds() const;
    double GetTotalMicroseconds() const;
    double GetTotalMilliSeconds() const;
    double GetTotalSeconds() const;

private:
    std::chrono::high_resolution_clock::time_point _startTime;
    std::chrono::high_resolution_clock::duration _deltaTime;
    std::chrono::high_resolution_clock::duration _totalTime;
};
