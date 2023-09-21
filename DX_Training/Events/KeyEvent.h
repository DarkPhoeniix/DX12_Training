#pragma once

#include "Event.h"

class KeyEvent : public EventArgs
{
public:
    enum KeyState
    {
        Released = 0,
        Pressed = 1
    };

    typedef EventArgs base;
    KeyEvent(KeyCode::Key key, unsigned int c, KeyState state, bool control, bool shift, bool alt)
        : Key(key)
        , Char(c)
        , State(state)
        , Control(control)
        , Shift(shift)
        , Alt(alt)
    {}

    KeyCode::Key    Key;    // The Key Code that was pressed or released.
    unsigned int    Char;   // The 32-bit character code that was pressed. This value will be 0 if it is a non-printable character.
    KeyState        State;  // Was the key pressed or released?
    bool            Control;// Is the Control modifier pressed
    bool            Shift;  // Is the Shift modifier pressed
    bool            Alt;    // Is the Alt modifier pressed
};
