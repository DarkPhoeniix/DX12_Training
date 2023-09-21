#pragma once

#include "Event.h"

class MouseButtonEvent : public Event
{
public:
    enum MouseButton
    {
        None = 0,
        Left = 1,
        Right = 2,
        Middel = 3
    };
    enum ButtonState
    {
        Released = 0,
        Pressed = 1
    };

    typedef Event base;
    MouseButtonEvent(MouseButton buttonID, ButtonState state, bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y)
        : Button(buttonID)
        , State(state)
        , LeftButton(leftButton)
        , MiddleButton(middleButton)
        , RightButton(rightButton)
        , Control(control)
        , Shift(shift)
        , X(x)
        , Y(y)
    {}

    MouseButton Button; // The mouse button that was pressed or released.
    ButtonState State;  // Was the button pressed or released?
    bool LeftButton;    // Is the left mouse button down?
    bool MiddleButton;  // Is the middle mouse button down?
    bool RightButton;   // Is the right mouse button down?
    bool Control;       // Is the CTRL key down?
    bool Shift;         // Is the Shift key down?

    int X;              // The X-position of the cursor relative to the upper-left corner of the client area.
    int Y;              // The Y-position of the cursor relative to the upper-left corner of the client area.
};
