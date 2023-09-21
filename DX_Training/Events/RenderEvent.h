#pragma once

#include "Event.h"

class RenderEvent : public Event
{
public:
    typedef Event base;
    RenderEvent(double fDeltaTime, double fTotalTime)
        : ElapsedTime(fDeltaTime)
        , TotalTime(fTotalTime)
    {}

    double ElapsedTime;
    double TotalTime;
};
