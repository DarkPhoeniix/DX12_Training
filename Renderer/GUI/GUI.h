#pragma once

#include "DescriptorHeap.h"

namespace dx12
{
    class CommandList;
    class SwapChain;
}

LRESULT GUI_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class GUI
{
public:
    static void Init(HWND windowHandle);
    static void NewFrame();
    static void Render(dx12::CommandList& commandList);
    static void Destroy();

private:
    GUI();
    ~GUI();

    // TODO: fix live DXDevice, change singleton
    static GUI& Instance();

    dx12::DescriptorHeap* _srvDescriptorHeap;
};
