#pragma once

#include "Event.h"

class ResizeEvent : public Event
{
public:
    typedef Event base;

    ResizeEvent(int width, int height)
        : width(width)
        , height(height)
    {   }

    int width;      // The new width of the window
    int height;     // The new height of the window
};
