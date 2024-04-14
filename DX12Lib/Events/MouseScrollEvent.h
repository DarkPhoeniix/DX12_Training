#pragma once

#include "IEvent.h"

namespace Core
{
    namespace Input
    {
        class MouseScrollEvent : public IEvent
        {
        public:
            MouseScrollEvent(float wheelDelta, bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y)
                : scrollDelta(wheelDelta)
                , leftButton(leftButton)
                , middleButton(middleButton)
                , rightButton(rightButton)
                , control(control)
                , shift(shift)
                , x(x)
                , y(y)
            {   }

            float scrollDelta;   // How much the mouse wheel has moved. A positive value indicates that the wheel was moved to the right. A negative value indicates the wheel was moved to the left.
            bool leftButton;    // Is the left mouse button down?
            bool middleButton;  // Is the middle mouse button down?
            bool rightButton;   // Is the right mouse button down?
            bool control;       // Is the CTRL key down?
            bool shift;         // Is the Shift key down?

            int x;              // The X-position of the cursor relative to the upper-left corner of the client area.
            int y;              // The Y-position of the cursor relative to the upper-left corner of the client area.
        };
    } // namespace Input
} // namespace Core
