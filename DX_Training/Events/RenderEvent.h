#pragma once

#include "Event.h"

class RenderEvent : public Event
{
public:
    typedef Event base;

    RenderEvent(double fDeltaTime, double fTotalTime)
        : elapsedTime(fDeltaTime)
        , totalTime(fTotalTime)
    {   }

    double elapsedTime;
    double totalTime;
};
