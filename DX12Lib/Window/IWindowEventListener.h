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
    class IWindowEventListener
    {
    public:
        virtual void OnUpdate(Input::UpdateEvent& e) {}
        virtual void OnRender(Input::RenderEvent& e, Frame& frame) {}
        virtual void OnKeyPressed(Input::KeyEvent& e) {}
        virtual void OnKeyReleased(Input::KeyEvent& e) {}
        virtual void OnMouseMoved(Input::MouseMoveEvent& e) {}
        virtual void OnMouseButtonPressed(Input::MouseButtonEvent& e) {}
        virtual void OnMouseButtonReleased(Input::MouseButtonEvent& e) {}
        virtual void OnMouseScroll(Input::MouseScrollEvent& e) {}
        virtual void OnResize(Input::ResizeEvent& e) {}
    };
}
