#pragma once

#include "Event.h"

class KeyEvent : public Event
{
public:
    typedef Event base;

    enum KeyState
    {
        Released = 0,
        Pressed = 1
    };

    KeyEvent(KeyCode::Key key, unsigned int c, KeyState state, bool control, bool shift, bool alt)
        : key(key)
        , character(c)
        , state(state)
        , control(control)
        , shift(shift)
        , alt(alt)
    {   }

    KeyCode::Key key;           // The Key Code that was pressed or released.
    unsigned int character;     // The 32-bit character code that was pressed. This value will be 0 if it is a non-printable character.
    KeyState state;             // Was the key pressed or released?
    bool control;               // Is the Control modifier pressed
    bool shift;                 // Is the Shift modifier pressed
    bool alt;                   // Is the Alt modifier pressed
};
