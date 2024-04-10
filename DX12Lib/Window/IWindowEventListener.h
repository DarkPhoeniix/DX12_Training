#pragma once

class UpdateEvent;
class RenderEvent;
class KeyEvent;
class MouseMoveEvent;
class MouseButtonEvent;
class MouseScrollEvent;
class ResizeEvent;

namespace Core
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
}
