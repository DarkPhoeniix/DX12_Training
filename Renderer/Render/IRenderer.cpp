#include "stdafx.h"

#include "IRenderer.h"

IRenderer::IRenderer(HWND windowHandle)
    : _windowHandle(windowHandle)
    , _contentLoaded(false)
{   }

IRenderer::~IRenderer()
{   }

bool IRenderer::LoadContent(TaskGPU* loadTask)
{
    return false;
}

void IRenderer::UnloadContent()
{   }

void IRenderer::OnUpdate(Core::Events::UpdateEvent&)
{   }

void IRenderer::OnRender(Core::Events::RenderEvent&, Frame& frame)
{   }

void IRenderer::OnKeyPressed(Core::Events::KeyEvent&)
{   }

void IRenderer::OnKeyReleased(Core::Events::KeyEvent&)
{   }

void IRenderer::OnMouseMoved(Core::Events::MouseMoveEvent&)
{   }

void IRenderer::OnMouseButtonPressed(Core::Events::MouseButtonEvent&)
{   }

void IRenderer::OnMouseButtonReleased(Core::Events::MouseButtonEvent&)
{   }

void IRenderer::OnMouseScroll(Core::Events::MouseScrollEvent&)
{   }

void IRenderer::OnResize(Core::Events::ResizeEvent&)
{   }
