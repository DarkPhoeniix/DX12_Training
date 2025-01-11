#pragma once

#include "events/UpdateEvent.h"
#include "events/RenderEvent.h"
#include "events/KeyEvent.h"
#include "events/MouseMoveEvent.h"
#include "events/MouseButtonEvent.h"
#include "events/MouseScrollEvent.h"
#include "events/ResizeEvent.h"

class Frame;

namespace core
{
    namespace events
    {
        class IWindowEventListener
        {
        public:
            virtual void OnUpdate(UpdateEvent& e) {}
            virtual void OnRender(RenderEvent& e) {}
            virtual void OnKeyPressed(KeyEvent& e) {}
            virtual void OnKeyReleased(KeyEvent& e) {}
            virtual void OnMouseMoved(MouseMoveEvent& e) {}
            virtual void OnMouseButtonPressed(MouseButtonEvent& e) {}
            virtual void OnMouseButtonReleased(MouseButtonEvent& e) {}
            virtual void OnMouseScroll(MouseScrollEvent& e) {}
            virtual void OnResize(ResizeEvent& e) {}
        };
    } // namespace events
} // namespace core
