#include "stdafx.h"

#include "Game.h"

#include "Application.h"
#include "Events/ResizeEvent.h"
#include "RenderWindow.h"

Game::Game(const std::wstring& name, int width, int height, bool vSync)
    : _name(name)
    , m_Width(width)
    , _height(height)
    , _vSync(vSync)
{   }

Game::~Game()
{
    assert(!_window && "Use Game::Destroy() before destruction.");
}

bool Game::initialize()
{
    // Check for DirectX Math library support.
    if (!DirectX::XMVerifyCPUSupport())
    {
        MessageBoxA(NULL, "Failed to verify DirectX Math library support.", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    _window = Application::get().createWindow(_name, m_Width, _height, _vSync);
    _window->registerCallbacks(shared_from_this());
    _window->show();

    return true;
}

void Game::destroy()
{
    Application::get().destroyWindow(_window);
    _window.reset();
}

void Game::onUpdate(UpdateEvent& e)
{   }

void Game::onRender(RenderEvent& e)
{   }

void Game::onKeyPressed(KeyEvent& e)
{   }

void Game::onKeyReleased(KeyEvent& e)
{   }

void Game::onMouseMoved(class MouseMoveEvent& e)
{   }

void Game::onMouseButtonPressed(MouseButtonEvent& e)
{   }

void Game::onMouseButtonReleased(MouseButtonEvent& e)
{   }

void Game::onMouseScroll(MouseScrollEvent& e)
{   }

void Game::onResize(ResizeEvent& e)
{
    m_Width = e.width;
    _height = e.height;
}

void Game::onWindowDestroy()
{
    // If the Window which we are registered to is 
    // destroyed, then any resources which are associated 
    // to the window must be released.
    unloadContent();
}
