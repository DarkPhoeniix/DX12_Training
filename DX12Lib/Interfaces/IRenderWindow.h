#pragma once

#include "Application.h"

class IRenderWindow
{
public:
    IRenderWindow(std::uint32_t width, std::uint32_t height, const std::wstring& name);
    virtual ~IRenderWindow();

    virtual void onInit() = 0;
    virtual void onUpdate() = 0;
    virtual void onRender() = 0;
    virtual void onDestroy() = 0;

    std::uint32_t getWidth() const;
    std::uint32_t getHeight() const;
    std::wstring getTitle() const;

protected:
    std::uint32_t _width;
    std::uint32_t _height;

    bool _useWarpDevice;

private:
    std::wstring _title;
};

