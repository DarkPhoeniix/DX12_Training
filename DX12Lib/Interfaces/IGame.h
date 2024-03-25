#pragma once

class Window;
class UpdateEvent;
class RenderEvent;
class MouseButtonEvent;
class MouseMoveEvent;
class MouseScrollEvent;
class KeyEvent;
class ResizeEvent;

class IGame : public std::enable_shared_from_this<IGame>
{
public:
    IGame(const std::wstring& name, int width, int height, bool vSync);
    virtual ~IGame();

    inline int getWidth() const
    {
        return _width;
    }
    inline int getHeight() const
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


public:
    ComPtr<ID3D12CommandQueue> queueStream;
    ComPtr<ID3D12CommandQueue> queueCopy;
    ComPtr<ID3D12CommandQueue> queueCompute;

private:
    std::wstring _name;
    int _width;
    int _height;
    bool _vSync;


};