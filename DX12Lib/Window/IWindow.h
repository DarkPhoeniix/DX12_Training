#pragma once

namespace Core
{
    class IWindow
    {
    public:
        IWindow(int width, int height, const std::wstring& title);
        virtual ~IWindow() = default;

        virtual void OnInit() = 0;
        virtual void OnUpdate() = 0;
        virtual void OnRender() = 0;
        virtual void OnDestroy() = 0;

        int GetWidth() const;
        int GetHeight() const;
        std::wstring GetTitle() const;

    protected:
        int _width;
        int _height;

        std::wstring _title;
    };
}
