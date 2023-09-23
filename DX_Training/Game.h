#pragma once

class Window;
class UpdateEvent;
class RenderEvent;
class MouseButtonEvent;
class MouseMoveEvent;
class MouseScrollEvent;
class KeyEvent;
class ResizeEvent;

class Game : public std::enable_shared_from_this<Game>
{
public:
    Game(const std::wstring& name, int width, int height, bool vSync);
    virtual ~Game();

    inline int getClientWidth() const
    {
        return m_Width;
    }
    inline int getClientHeight() const
    {
        return _height;
    }

    virtual bool initialize();
    virtual void destroy();

    virtual bool loadContent() = 0;
    virtual void unloadContent() = 0;

protected:
    friend class Window;

    virtual void onUpdate(UpdateEvent& e);
    virtual void onRender(RenderEvent& e);
    virtual void onKeyPressed(KeyEvent& e);
    virtual void onKeyReleased(KeyEvent& e);
    virtual void onMouseMoved(MouseMoveEvent& e);
    virtual void onMouseButtonPressed(MouseButtonEvent& e);
    virtual void onMouseButtonReleased(MouseButtonEvent& e);
    virtual void onMouseScroll(MouseScrollEvent& e);
    virtual void onResize(ResizeEvent& e);
    virtual void onWindowDestroy();

    std::shared_ptr<Window> _window;

private:
    std::wstring _name;
    int m_Width;
    int _height;
    bool _vSync;
};