#pragma once

#include "DescriptorHeap.h"

namespace dx12
{
    class CommandList;
} // namespace dx12

LRESULT GUI_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace gui
{
    class Editor
    {
    public:
        Editor(const Editor& copy) = delete;
        Editor operator=(const Editor& copy) = delete;

        static void Init(HWND windowhandle);
        static void Destroy();

        static void NewFrame();
        static void Render(dx12::CommandList& commandList);

    private:
        Editor();
        ~Editor() = default;

        static Editor& Instance();

        std::shared_ptr<dx12::DescriptorHeap> _srvDescriptorHeap;

        static Editor* _instance;
    };
} // namespace gui
