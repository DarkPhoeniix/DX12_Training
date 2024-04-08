#include "stdafx.h"

#include "IWindow.h"

namespace Core
{
    IWindow::IWindow(int width, int height, const std::wstring& title)
        : _title(title)
        , _width(width)
        , _height(height)
    {   }

    int IWindow::GetWidth() const
    {
        return _width;
    }

    int IWindow::GetHeight() const
    {
        return _height;
    }

    std::wstring IWindow::GetTitle() const
    {
        return _title;
    }
} // namespace Core
