#pragma once

#include "IEvent.h"

namespace Core
{
    namespace Input
    {
        class UpdateEvent : public IEvent
        {
        public:
            UpdateEvent(double fDeltaTime, double fTotalTime)
                : elapsedTime(fDeltaTime)
                , totalTime(fTotalTime)
            {   }

            double elapsedTime;
            double totalTime;
            unsigned int frameIndex;
        };
    } // namespace Input
} // namespace Core
