#pragma once

#include "Event.h"

class UpdateEvent : public Event
{
public:
    typedef Event base;

    UpdateEvent(double fDeltaTime, double fTotalTime)
        : elapsedTime(fDeltaTime)
        , totalTime(fTotalTime)
    {   }

    double elapsedTime;
    double totalTime;
    unsigned int frameIndex;
};
