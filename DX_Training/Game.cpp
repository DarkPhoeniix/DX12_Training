#include "stdafx.h"

#include "Game.h"

#include "Application.h"
#include "Events/ResizeEvent.h"
#include "RenderWindow.h"

Game::Game(const std::wstring& name, int width, int height, bool vSync)
    : m_Name(name)
    , m_Width(width)
    , m_Height(height)
    , m_vSync(vSync)
{   }

Game::~Game()
{
    assert(!m_pWindow && "Use Game::Destroy() before destruction.");
}

inline int Game::getClientWidth() const
{
    return m_Width;
}

inline int Game::getClientHeight() const
{
    return m_Height;
}

bool Game::initialize()
{
    // Check for DirectX Math library support.
    if (!DirectX::XMVerifyCPUSupport())
    {
        MessageBoxA(NULL, "Failed to verify DirectX Math library support.", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    m_pWindow = Application::get().createRenderWindow(m_Name, m_Width, m_Height, m_vSync);
    m_pWindow->registerCallbacks(shared_from_this());
    m_pWindow->show();

    return true;
}

void Game::destroy()
{
    Application::Get().DestroyWindow(m_pWindow);
    m_pWindow.reset();
}

void Game::OnUpdate(UpdateEvent& e)
{

}

void Game::OnRender(RenderEvent& e)
{

}

void Game::OnKeyPressed(KeyEvent& e)
{
    // By default, do nothing.
}

void Game::OnKeyReleased(KeyEvent& e)
{
    // By default, do nothing.
}

void Game::OnMouseMoved(class MouseMoveEvent& e)
{
    // By default, do nothing.
}

void Game::OnMouseButtonPressed(MouseButtonEvent& e)
{
    // By default, do nothing.
}

void Game::OnMouseButtonReleased(MouseButtonEvent& e)
{
    // By default, do nothing.
}

void Game::OnMouseWheel(MouseWheelEvent& e)
{
    // By default, do nothing.
}

void Game::OnResize(ResizeEvent& e)
{
    m_Width = e.width;
    m_Height = e.height;
}

void Game::OnWindowDestroy()
{
    // If the Window which we are registered to is 
    // destroyed, then any resources which are associated 
    // to the window must be released.
    unloadContent();
}
