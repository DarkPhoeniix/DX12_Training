#pragma once

#include "DXObjects/DescriptorHeap.h"

namespace Core
{
    class GraphicsCommandList;
    class SwapChain;
}

LRESULT GUI_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class GUI
{
public:
    static void Init(HWND windowHandle, const Core::SwapChain& swapChain);
    static void NewFrame();
    static void Render(Core::GraphicsCommandList& commandList);
    static void Destroy();

private:
    GUI();
    ~GUI();

    // TODO: fix live DXDevice, change singleton
    static GUI& Instance();

    Core::DescriptorHeap _srvDescriptorHeap;
};
