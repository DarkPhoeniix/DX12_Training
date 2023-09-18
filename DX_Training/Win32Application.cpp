
#include "stdafx.h"

#include "Win32Application.h"

HWND Win32Application::_hwnd = nullptr;

int Win32Application::run(IRenderWindow* window, HINSTANCE hInstance, int nCmdShow)
{
    WNDCLASSEXW windowClass = WNDCLASSEXW();
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = &WindowProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = L"D3DX12 Simple Rendering";
    windowClass.hIconSm = ::LoadIcon(hInstance, NULL);

    static ATOM atom = ::RegisterClassExW(&windowClass);
    assert(atom > 0);

    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, static_cast<LONG>(window->getWidth()), static_cast<LONG>(window->getHeight()) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.
    _hwnd = CreateWindow(
        windowClass.lpszClassName,
        window->getTitle().c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,        // Object has no parent window
        nullptr,        // Not using menus
        hInstance,
        window);

    window->onInit();
    ShowWindow(_hwnd, nCmdShow);

    MSG msg = {};
    while (msg.message != WM_QUIT)                          // Main loop
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))       // Process any messages in the queue
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    window->onDestroy();

    return static_cast<char>(msg.wParam);                   // Return this part of the WM_QUIT message to Windows
}

HWND Win32Application::getHwnd()
{
    return _hwnd;
}

LRESULT Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    IRenderWindow* window = reinterpret_cast<IRenderWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
        {
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        }
        return 0;

    case WM_PAINT:
        if (window)
        {
            window->onUpdate();
            window->onRender();
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}
