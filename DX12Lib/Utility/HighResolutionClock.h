#pragma once

class HighResolutionClock
{
public:
    HighResolutionClock();

    void tick();

    void reset();

    double getDeltaNanoseconds() const;
    double getDeltaMicroseconds() const;
    double getDeltaMilliseconds() const;
    double getDeltaSeconds() const;

    double getTotalNanoseconds() const;
    double getTotalMicroseconds() const;
    double getTotalMilliSeconds() const;
    double getTotalSeconds() const;

private:
    std::chrono::high_resolution_clock::time_point _startTime;
    std::chrono::high_resolution_clock::duration _deltaTime;
    std::chrono::high_resolution_clock::duration _totalTime;
};
