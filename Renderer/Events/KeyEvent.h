#pragma once

#include "Events/IEvent.h"
#include "Input/DirectInputKeyCodes.h"

namespace Core
{
    namespace Events
    {
        class KeyEvent : public IEvent
        {
        public:
            KeyEvent(DIKeyCode keyCode)
                : keyCode(keyCode)
            {   }

            DIKeyCode keyCode;
        };
    } // namespace Events
} // namespace Core
