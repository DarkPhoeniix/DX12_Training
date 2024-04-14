#pragma once

#include "IEvent.h"

namespace Core
{
    namespace Events
    {
        class RenderEvent : public IEvent
        {
        public:
            RenderEvent(double fDeltaTime, double fTotalTime, unsigned int frameIndex)
                : elapsedTime(fDeltaTime)
                , totalTime(fTotalTime)
                , frameIndex(frameIndex)
            {   }

            double elapsedTime;
            double totalTime;
            unsigned int frameIndex;
        };
    } // namespace Input
} // namespace Core
