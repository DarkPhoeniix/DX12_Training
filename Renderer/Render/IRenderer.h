#pragma once

#include "Window/IWindowEventListener.h"

class Frame;
class TaskGPU;

class IRenderer : public Core::Events::IWindowEventListener
{
public:
    IRenderer(HWND windowHandle);
    ~IRenderer();

    virtual bool LoadContent(TaskGPU* loadTask) = 0;
    virtual void UnloadContent() = 0;

    virtual void OnUpdate(Core::Events::UpdateEvent&) override;
    virtual void OnRender(Core::Events::RenderEvent&, Frame& frame) override;
    virtual void OnKeyPressed(Core::Events::KeyEvent&) override;
    virtual void OnKeyReleased(Core::Events::KeyEvent&) override;
    virtual void OnMouseMoved(Core::Events::MouseMoveEvent&) override;
    virtual void OnMouseButtonPressed(Core::Events::MouseButtonEvent&) override;
    virtual void OnMouseButtonReleased(Core::Events::MouseButtonEvent&) override;
    virtual void OnMouseScroll(Core::Events::MouseScrollEvent&) override;
    virtual void OnResize(Core::Events::ResizeEvent&) override;

protected:
    HWND _windowHandle;

    bool _contentLoaded;
};