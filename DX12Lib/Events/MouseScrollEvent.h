#pragma once

#include "IEvent.h"

namespace Core
{
    namespace Events
    {
        class MouseScrollEvent : public IEvent
        {
        public:
            MouseScrollEvent(float wheelDelta)
                : scrollDelta(wheelDelta)
            {   }

            float scrollDelta;
        };
    } // namespace Input
} // namespace Core
