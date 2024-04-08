#pragma once

#include "IEvent.h"

namespace Core
{
    namespace Input
    {
        class MouseEvent : public IEvent
        {
        public:
            MouseEvent(int x, int y, int offsetX, int offsetY, bool leftButton, bool middleButton, bool rightButton)
                : x(x)
                , y(y)
                , offsetX(offsetX)
                , offsetY(offsetY)
                , leftButton(leftButton)
                , middleButton(middleButton)
                , rightButton(rightButton)
            {   }

            int x;
            int y;
            int offsetX;
            int offsetY;

            bool leftButton;
            bool middleButton;
            bool rightButton;
        };
    }
}
