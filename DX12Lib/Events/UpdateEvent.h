#pragma once

#include "IEvent.h"

namespace Core
{
    namespace Events
    {
        class UpdateEvent : public IEvent
        {
        public:
            UpdateEvent(double fDeltaTime, double fTotalTime, unsigned int frameIndex)
                : elapsedTime(fDeltaTime)
                , totalTime(fTotalTime)
                , frameIndex(frameIndex)
            {   }

            double elapsedTime;
            double totalTime;
            unsigned int frameIndex;
        };
    } // namespace Events
} // namespace Core
