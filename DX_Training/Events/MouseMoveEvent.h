#pragma once

#include "Event.h"

class MouseMoveEvent : public Event
{
public:
    typedef Event base;

    MouseMoveEvent(bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y)
        : leftButton(leftButton)
        , middleButton(middleButton)
        , rightButton(rightButton)
        , control(control)
        , shift(shift)
        , x(x)
        , y(y)
        , relativeX(0)
        , relativeY(0)
    {   }

    bool leftButton;    // Is the left mouse button down?
    bool middleButton;  // Is the middle mouse button down?
    bool rightButton;   // Is the right mouse button down?
    bool control;       // Is the CTRL key down?
    bool shift;         // Is the Shift key down?

    int x;              // The X-position of the cursor relative to the upper-left corner of the client area.
    int y;              // The Y-position of the cursor relative to the upper-left corner of the client area.
    int relativeX;           // How far the mouse moved since the last event.
    int relativeY;           // How far the mouse moved since the last event.
};
