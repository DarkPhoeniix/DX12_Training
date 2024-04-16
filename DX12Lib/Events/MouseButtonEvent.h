#pragma once

#include "IEvent.h"

namespace Core
{
    namespace Events
    {
        class MouseButtonEvent : public IEvent
        {
        public:
            MouseButtonEvent(bool leftButton, bool middleButton, bool rightButton, int x, int y)
                : leftButton(leftButton)
                , middleButton(middleButton)
                , rightButton(rightButton)
                , x(x)
                , y(y)
            {   }

            bool leftButton;        // Is the left mouse button down?
            bool middleButton;      // Is the middle mouse button down?
            bool rightButton;       // Is the right mouse button down?

            int x;                  // The X-position of the cursor relative to the upper-left corner of the client area.
            int y;                  // The Y-position of the cursor relative to the upper-left corner of the client area.
        };
    } // namespace Input
} // namespace Core
