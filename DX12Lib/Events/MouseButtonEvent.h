#pragma once

#include "IEvent.h"

namespace Core
{
    namespace Input
    {
        class MouseButtonEvent : public IEvent
        {
        public:
            enum MouseButton
            {
                None = 0,
                Left = 1,
                Right = 2,
                Middle = 3
            };
            enum ButtonState
            {
                Released = 0,
                Pressed = 1
            };

            MouseButtonEvent(MouseButton buttonID, ButtonState state, bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y)
                : button(buttonID)
                , state(state)
                , leftButton(leftButton)
                , middleButton(middleButton)
                , rightButton(rightButton)
                , control(control)
                , shift(shift)
                , x(x)
                , y(y)
            {   }

            MouseButton button;     // The mouse button that was pressed or released.
            ButtonState state;      // Was the button pressed or released?
            bool leftButton;        // Is the left mouse button down?
            bool middleButton;      // Is the middle mouse button down?
            bool rightButton;       // Is the right mouse button down?
            bool control;           // Is the CTRL key down?
            bool shift;             // Is the Shift key down?

            int x;                  // The X-position of the cursor relative to the upper-left corner of the client area.
            int y;                  // The Y-position of the cursor relative to the upper-left corner of the client area.
        };
    } // namespace Input
} // namespace Core
