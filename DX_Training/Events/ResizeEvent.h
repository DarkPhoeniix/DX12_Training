#pragma once

#include "Event.h"

class ResizeEvent : public Event
{
public:
    typedef Event base;
    ResizeEvent(int width, int height)
        : width(width)
        , height(height)
    {}

    // The new width of the window
    int width;
    // The new height of the window.
    int height;
};
