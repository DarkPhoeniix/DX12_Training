#pragma once

#include "IEvent.h"

namespace Core
{
    namespace Events
    {
        class MouseMoveEvent : public IEvent
        {
        public:
            MouseMoveEvent()
                : leftButton(false)
                , middleButton(false)
                , rightButton(false)
                , x(0)
                , y(0)
                , relativeX(0)
                , relativeY(0)
            {   }

            bool leftButton;    // Is the left mouse button down?
            bool middleButton;  // Is the middle mouse button down?
            bool rightButton;   // Is the right mouse button down?

            int x;              // The X-position of the cursor relative to the upper-left corner of the client area.
            int y;              // The Y-position of the cursor relative to the upper-left corner of the client area.
            int relativeX;           // How far the mouse moved since the last event.
            int relativeY;           // How far the mouse moved since the last event.
        };
    } // namespace Events
} // namespace Core
