#pragma once

#include "Events/Event.h"

class RenderWindow;
class UpdateEvent;
class RenderEvent;
class MouseButtonEvent;
class MouseMoveEvent;
class MouseWheelEvent;
class KeyEvent;
class ResizeEvent;

class Game : public std::enable_shared_from_this<Game>
{
public:
    Game(const std::wstring& name, int width, int height, bool vSync);
    virtual ~Game();

    int getClientWidth() const;
    int getClientHeight() const;

    virtual bool initialize();
    virtual void destroy();

    virtual bool loadContent() = 0;
    virtual void unloadContent() = 0;

protected:
    friend class Window;

    virtual void OnUpdate(UpdateEvent& e);
    virtual void OnRender(RenderEvent& e);
    virtual void OnKeyPressed(KeyEvent& e);
    virtual void OnKeyReleased(KeyEvent& e);
    virtual void OnMouseMoved(MouseMoveEvent& e);
    virtual void OnMouseButtonPressed(MouseButtonEvent& e);
    virtual void OnMouseButtonReleased(MouseButtonEvent& e);
    virtual void OnMouseWheel(MouseWheelEvent& e);
    virtual void OnResize(ResizeEvent& e);
    virtual void OnWindowDestroy();

    std::shared_ptr<RenderWindow> m_pWindow;

private:
    std::wstring m_Name;
    int m_Width;
    int m_Height;
    bool m_vSync;
};