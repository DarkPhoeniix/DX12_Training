#pragma once

#include "IEvent.h"

namespace Core
{
    namespace Events
    {
        class ResizeEvent : public IEvent
        {
        public:
            ResizeEvent(int width, int height)
                : width(width)
                , height(height)
            {   }

            int width;      // The new width of the window
            int height;     // The new height of the window
        };
    } // namespace Input
} // namespace Core
