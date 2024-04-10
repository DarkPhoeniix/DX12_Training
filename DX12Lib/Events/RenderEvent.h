#pragma once

#include "IEvent.h"

namespace Core
{
    namespace Input
    {
        class RenderEvent : public IEvent
        {
        public:
            RenderEvent(double fDeltaTime, double fTotalTime)
                : elapsedTime(fDeltaTime)
                , totalTime(fTotalTime)
            {   }

            double elapsedTime;
            double totalTime;
            unsigned int frameIndex;
        };
    } // namespace Input
} // namespace Core
