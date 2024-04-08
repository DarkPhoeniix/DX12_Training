#pragma once

#include <dinput.h>

#define KEYDOWN(state, key) (state[key] & 0x80)

namespace Core
{
    namespace Input
    {
        class IInputObserver
        {
        public:
            virtual void OnMouseEvent(DIMOUSESTATE2 mouseState) {};
            virtual void OnKeyboardEvent(BYTE keyboardState[256]) {};
        };
    }
}
