#pragma once

#include "Events/UpdateEvent.h"
#include "Events/RenderEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseMoveEvent.h"
#include "Events/MouseButtonEvent.h"
#include "Events/MouseScrollEvent.h"
#include "Events/ResizeEvent.h"

class Frame;

namespace Core
{
    namespace Events
    {
        class IWindowEventListener
        {
        public:
            virtual void OnUpdate(UpdateEvent& e) {}
            virtual void OnRender(RenderEvent& e, Frame& frame) {}
            virtual void OnKeyPressed(KeyEvent& e) {}
            virtual void OnKeyReleased(KeyEvent& e) {}
            virtual void OnMouseMoved(MouseMoveEvent& e) {}
            virtual void OnMouseButtonPressed(MouseButtonEvent& e) {}
            virtual void OnMouseButtonReleased(MouseButtonEvent& e) {}
            virtual void OnMouseScroll(MouseScrollEvent& e) {}
            virtual void OnResize(ResizeEvent& e) {}
        };
    } // namespace Events
} // namespace Core
