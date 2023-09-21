
#include "stdafx.h"

#include "RenderWindow.h"

uint32_t g_ClientWidth = 1280;
uint32_t g_ClientHeight = 720;

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    RenderWindow window(g_ClientWidth, g_ClientHeight, L"D3DX12 Simple Render");
    return Application::run(&window, hInstance, nCmdShow);
}
