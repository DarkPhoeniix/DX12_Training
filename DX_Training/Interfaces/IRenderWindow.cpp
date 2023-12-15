
#include "stdafx.h"

#include "IRenderWindow.h"

IRenderWindow::IRenderWindow(std::uint32_t width, std::uint32_t height, const std::wstring& name)
    : _width(width), _height(height), _title(name), _useWarpDevice(false)
{   }

IRenderWindow::~IRenderWindow()
{   }

std::uint32_t IRenderWindow::getWidth() const
{
    return _width;
}

std::uint32_t IRenderWindow::getHeight() const
{
    return _height;
}

std::wstring IRenderWindow::getTitle() const
{
    return _title;
}
