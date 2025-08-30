#pragma once

#include "IEvent.h"

namespace core
{
    namespace events
    {
        class MouseScrollEvent : public IEvent
        {
        public:
            MouseScrollEvent(float wheelDelta)
                : scrollDelta(wheelDelta)
            {
            }

            float scrollDelta;
        };
    } // namespace events
} // namespace core
