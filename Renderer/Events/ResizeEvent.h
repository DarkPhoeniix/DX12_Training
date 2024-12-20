#pragma once

#include "IEvent.h"

namespace Core
{
    namespace Events
    {
        class ResizeEvent : public IEvent
        {
        public:
            ResizeEvent(int width, int height, bool fullscreen = false)
                : width(width)
                , height(height)
                , isFullscreen(fullscreen)
            {   }

            int width;      // The new width of the window
            int height;     // The new height of the window
            bool isFullscreen;
        };
    } // namespace Events
} // namespace Core
