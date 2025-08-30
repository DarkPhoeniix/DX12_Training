#pragma once

#include "events/IEvent.h"
#include "input/DirectinputKeyCodes.h"

namespace core
{
    namespace events
    {
        class KeyEvent : public IEvent
        {
        public:
            KeyEvent(DIKeyCode keyCode)
                : keyCode(keyCode)
            {
            }

            DIKeyCode keyCode;
        };
    } // namespace events
} // namespace core
