#pragma once

#include "Event.h"

class UpdateEvent : public Event
{
public:
    typedef Event base;

    UpdateEvent(double fDeltaTime, double fTotalTime)
        : ElapsedTime(fDeltaTime)
        , TotalTime(fTotalTime)
    {   }

    double ElapsedTime;
    double TotalTime;
};
