#include "stdafx.h"

#include "IGame.h"

#include "Application.h"
#include "Events/ResizeEvent.h"
#include "RenderWindow.h"

IGame::IGame(const std::wstring& name, int width, int height, bool vSync)
    : _name(name)
    , _width(width)
    , _height(height)
    , _vSync(vSync)
{   }

IGame::~IGame()
{
    assert(!_window && "Use IGame::Destroy() before destruction.");
}

bool IGame::initialize()
{
    // Check for DirectX Math library support.
    if (!DirectX::XMVerifyCPUSupport())
    {
        MessageBoxA(NULL, "Failed to verify DirectX Math library support.", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    _window = Application::get().createWindow(_name, _width, _height, _vSync);
    _window->registerCallbacks(shared_from_this());
    _window->show();

    return true;
}

void IGame::destroy()
{
    Application::get().destroyWindow(_window);
    _window.reset();
}

void IGame::onUpdate(UpdateEvent&)
{   }

void IGame::onRender(RenderEvent&)
{   }

void IGame::onKeyPressed(KeyEvent&)
{   }

void IGame::onKeyReleased(KeyEvent&)
{   }

void IGame::onMouseMoved(MouseMoveEvent&)
{   }

void IGame::onMouseButtonPressed(MouseButtonEvent&)
{   }

void IGame::onMouseButtonReleased(MouseButtonEvent&)
{   }

void IGame::onMouseScroll(MouseScrollEvent&)
{   }

void IGame::onResize(ResizeEvent& e)
{
    _width = e.width;
    _height = e.height;
}

void IGame::onWindowDestroy()
{
    // If the Window which we are registered to is 
    // destroyed, then any resources which are associated 
    // to the window must be released.
    unloadContent();
}
