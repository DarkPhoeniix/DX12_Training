#pragma once

#include "Event.h"

class MouseMoveEvent : public Event
{
public:
    typedef Event base;

    MouseMoveEvent(bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y)
        : LeftButton(leftButton)
        , MiddleButton(middleButton)
        , RightButton(rightButton)
        , Control(control)
        , Shift(shift)
        , X(x)
        , Y(y)
    {}

    bool LeftButton;    // Is the left mouse button down?
    bool MiddleButton;  // Is the middle mouse button down?
    bool RightButton;   // Is the right mouse button down?
    bool Control;       // Is the CTRL key down?
    bool Shift;         // Is the Shift key down?

    int X;              // The X-position of the cursor relative to the upper-left corner of the client area.
    int Y;              // The Y-position of the cursor relative to the upper-left corner of the client area.
    int RelX;           // How far the mouse moved since the last event.
    int RelY;           // How far the mouse moved since the last event.

};
