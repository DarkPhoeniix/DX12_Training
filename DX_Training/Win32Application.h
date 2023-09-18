#pragma once

#include "IRenderWindow.h"

class IRenderWindow;

class Win32Application
{
public:
    static int run(IRenderWindow* window, HINSTANCE hInstance, int nCmdShow);
    static HWND getHwnd();

protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    static HWND _hwnd;
};
