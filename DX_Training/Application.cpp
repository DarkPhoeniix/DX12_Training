
#include "stdafx.h"

#include "Application.h"

#include "Interfaces/IGame.h"
#include "Render/RenderWindow.h"
#include "Render/CommandQueue.h"
#include "Events/KeyEvent.h"
#include "Events/MouseButtonEvent.h"
#include "Events/MouseMoveEvent.h"
#include "Events/MouseScrollEvent.h"
#include "Events/RenderEvent.h"
#include "Events/ResizeEvent.h"
#include "Events/UpdateEvent.h"

namespace
{
    constexpr wchar_t WINDOW_CLASS_NAME[] = L"DX12WindowClass";

    using WindowPtr = std::shared_ptr<Window>;
    using WindowMap = std::map<HWND, WindowPtr>;
    using WindowNameMap = std::map<std::wstring, WindowPtr>;

    static WindowMap g_windows;
    static WindowNameMap g_windowsByName;

    static Application* g_ApplicationInstance = nullptr;

    // A wrapper struct to allow shared pointers for the window class.
    struct MakeWindow : public Window
    {
        MakeWindow(HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync)
            : Window(hWnd, windowName, clientWidth, clientHeight, vSync)
        {   }
    };

    static void RemoveWindow(HWND hWnd)
    {
        WindowMap::iterator windowIter = g_windows.find(hWnd);
        if (windowIter != g_windows.end())
        {
            WindowPtr pWindow = windowIter->second;
            g_windowsByName.erase(pWindow->getWindowName());
            g_windows.erase(windowIter);
        }
    }

    // Convert the message ID into a MouseButton ID
    MouseButtonEvent::MouseButton DecodeMouseButton(UINT messageID)
    {
        MouseButtonEvent::MouseButton mouseButton = MouseButtonEvent::None;
        switch (messageID)
        {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
        {
            mouseButton = MouseButtonEvent::Left;
        }
        break;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
        {
            mouseButton = MouseButtonEvent::Right;
        }
        break;
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
        {
            mouseButton = MouseButtonEvent::Middle;
        }
        break;
        }

        return mouseButton;
    }
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WindowPtr pWindow;
    {
        WindowMap::iterator iter = g_windows.find(hwnd);
        if (iter != g_windows.end())
        {
            pWindow = iter->second;
        }
    }

    if (pWindow)
    {
        switch (message)
        {
        case WM_PAINT:
        {
            // Delta time will be filled in by the Window.
            UpdateEvent updateEventArgs(0.0f, 0.0f);
            pWindow->onUpdate(updateEventArgs);
            RenderEvent renderEventArgs(0.0f, 0.0f);
            // Delta time will be filled in by the Window.
            pWindow->onRender(renderEventArgs);
        }
        break;
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        {
            MSG charMsg;
            // Get the Unicode character (UTF-16)
            unsigned int c = 0;
            // For printable characters, the next message will be WM_CHAR.
            // This message contains the character code we need to send the KeyPressed event.
            // Inspired by the SDL 1.2 implementation.
            if (PeekMessage(&charMsg, hwnd, 0, 0, PM_NOREMOVE) && charMsg.message == WM_CHAR)
            {
                GetMessage(&charMsg, hwnd, 0, 0);
                c = static_cast<unsigned int>(charMsg.wParam);
            }
            bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
            bool control = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
            bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
            KeyCode::Key key = (KeyCode::Key)wParam;
            unsigned int scanCode = (lParam & 0x00FF0000) >> 16;
            KeyEvent keyEventArgs(key, c, KeyEvent::Pressed, shift, control, alt);
            pWindow->onKeyPressed(keyEventArgs);
        }
        break;
        case WM_SYSKEYUP:
        case WM_KEYUP:
        {
            bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
            bool control = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
            bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
            KeyCode::Key key = (KeyCode::Key)wParam;
            unsigned int c = 0;
            unsigned int scanCode = (lParam & 0x00FF0000) >> 16;

            // Determine which key was released by converting the key code and the scan code
            // to a printable character (if possible).
            // Inspired by the SDL 1.2 implementation.
            unsigned char keyboardState[256];
            GetKeyboardState(keyboardState);
            wchar_t translatedCharacters[4];
            if (int result = ToUnicodeEx(static_cast<UINT>(wParam), scanCode, keyboardState, translatedCharacters, 4, 0, NULL) > 0)
            {
                c = translatedCharacters[0];
            }

            KeyEvent keyEventArgs(key, c, KeyEvent::Released, shift, control, alt);
            pWindow->onKeyReleased(keyEventArgs);
        }
        break;
        // The default window procedure will play a system notification sound 
        // when pressing the Alt+Enter keyboard combination if this message is 
        // not handled.
        case WM_SYSCHAR:
            break;
        case WM_MOUSEMOVE:
        {
            bool lButton = (wParam & MK_LBUTTON) != 0;
            bool rButton = (wParam & MK_RBUTTON) != 0;
            bool mButton = (wParam & MK_MBUTTON) != 0;
            bool shift = (wParam & MK_SHIFT) != 0;
            bool control = (wParam & MK_CONTROL) != 0;

            int x = ((int)(short)LOWORD(lParam));
            int y = ((int)(short)HIWORD(lParam));

            MouseMoveEvent mouseMotionEventArgs(lButton, mButton, rButton, control, shift, x, y);
            pWindow->onMouseMoved(mouseMotionEventArgs);
        }
        break;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        {
            bool lButton = (wParam & MK_LBUTTON) != 0;
            bool rButton = (wParam & MK_RBUTTON) != 0;
            bool mButton = (wParam & MK_MBUTTON) != 0;
            bool shift = (wParam & MK_SHIFT) != 0;
            bool control = (wParam & MK_CONTROL) != 0;

            int x = ((int)(short)LOWORD(lParam));
            int y = ((int)(short)HIWORD(lParam));

            MouseButtonEvent mouseButtonEventArgs(DecodeMouseButton(message), MouseButtonEvent::Pressed, lButton, mButton, rButton, control, shift, x, y);
            pWindow->onMouseButtonPressed(mouseButtonEventArgs);
        }
        break;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        {
            bool lButton = (wParam & MK_LBUTTON) != 0;
            bool rButton = (wParam & MK_RBUTTON) != 0;
            bool mButton = (wParam & MK_MBUTTON) != 0;
            bool shift = (wParam & MK_SHIFT) != 0;
            bool control = (wParam & MK_CONTROL) != 0;

            int x = ((int)(short)LOWORD(lParam));
            int y = ((int)(short)HIWORD(lParam));

            MouseButtonEvent mouseButtonEventArgs(DecodeMouseButton(message), MouseButtonEvent::Released, lButton, mButton, rButton, control, shift, x, y);
            pWindow->onMouseButtonReleased(mouseButtonEventArgs);
        }
        break;
        case WM_MOUSEWHEEL:
        {
            // The distance the mouse wheel is rotated.
            // A positive value indicates the wheel was rotated to the right.
            // A negative value indicates the wheel was rotated to the left.
            float zDelta = ((int)(short)HIWORD(wParam)) / (float)WHEEL_DELTA;
            short keyStates = (short)LOWORD(wParam);

            bool lButton = (keyStates & MK_LBUTTON) != 0;
            bool rButton = (keyStates & MK_RBUTTON) != 0;
            bool mButton = (keyStates & MK_MBUTTON) != 0;
            bool shift = (keyStates & MK_SHIFT) != 0;
            bool control = (keyStates & MK_CONTROL) != 0;

            int x = ((int)(short)LOWORD(lParam));
            int y = ((int)(short)HIWORD(lParam));

            // Convert the screen coordinates to client coordinates.
            POINT clientToScreenPoint;
            clientToScreenPoint.x = x;
            clientToScreenPoint.y = y;
            ScreenToClient(hwnd, &clientToScreenPoint);

            MouseScrollEvent mouseWheelEventArgs(zDelta, lButton, mButton, rButton, control, shift, (int)clientToScreenPoint.x, (int)clientToScreenPoint.y);
            pWindow->onMouseScroll(mouseWheelEventArgs);
        }
        break;
        case WM_SIZE:
        {
            int width = ((int)(short)LOWORD(lParam));
            int height = ((int)(short)HIWORD(lParam));

            ResizeEvent resizeEventArgs(width, height);
            pWindow->onResize(resizeEventArgs);
        }
        break;
        case WM_DESTROY:
        {
            // If a window is being destroyed, remove it from the 
            // window maps.
            RemoveWindow(hwnd);

            if (g_windows.empty())
            {
                // If there are no more windows, quit the application.
                PostQuitMessage(0);
            }
        }
        break;
        default:
            return DefWindowProcW(hwnd, message, wParam, lParam);
        }
    }
    else
    {
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    return 0;
}

std::shared_ptr<Window> Application::createWindow(const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync)
{
    WindowNameMap::iterator windowIter = g_windowsByName.find(windowName);
    if (windowIter != g_windowsByName.end())
    {
        return windowIter->second;
    }

    RECT windowRect = { 0, 0, clientWidth, clientHeight };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hWnd = CreateWindowW(WINDOW_CLASS_NAME, windowName.c_str(),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr, nullptr, _hInstance, nullptr);

    if (!hWnd)
    {
        MessageBoxA(NULL, "Could not create the render window.", "Error", MB_OK | MB_ICONERROR);
        return nullptr;
    }

    WindowPtr pWindow = std::make_shared<MakeWindow>(hWnd, windowName, clientWidth, clientHeight, vSync); // TODO: wtf?

    g_windows.insert(WindowMap::value_type(hWnd, pWindow));
    g_windowsByName.insert(WindowNameMap::value_type(windowName, pWindow));

    return pWindow;
}

void Application::destroyWindow(std::shared_ptr<Window> window)
{
    if (window) window->destroy();
}

void Application::destroyWindow(const std::wstring& windowName)
{
    WindowPtr pWindow = getWindowByName(windowName);
    if (pWindow)
    {
        destroyWindow(pWindow);
    }
}

std::shared_ptr<Window> Application::getWindowByName(const std::wstring& windowName)
{
    std::shared_ptr<Window> window;
    WindowNameMap::iterator iter = g_windowsByName.find(windowName);
    if (iter != g_windowsByName.end())
    {
        window = iter->second;
    }

    return window;
}

void Application::create(HINSTANCE hInstance)
{
    if (!g_ApplicationInstance)
    {
        g_ApplicationInstance = new Application(hInstance);
    }
}

void Application::destroy()
{
    delete g_ApplicationInstance;
}

Application& Application::get()
{
    assert(g_ApplicationInstance);
    return *g_ApplicationInstance;
}

int Application::run(std::shared_ptr<IGame> pGame)
{
    if (!pGame->initialize()) return 1;
    if (!pGame->loadContent()) return 2;

    MSG msg = { 0 };
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // RENDER
        }
        
    }

    // Flush any commands in the commands queues before quiting.
    flush();

    pGame->unloadContent();
    pGame->destroy();

    return static_cast<int>(msg.wParam);
}

void Application::quit(int exitCode)
{
    PostQuitMessage(exitCode);
}

bool Application::isTearingSupported() const
{
    return _isTearingSupported;
}

ComPtr<ID3D12Device2> Application::getDevice() const
{
    return _d3d12Device;
}

std::shared_ptr<CommandQueue> Application::getCommandQueue(D3D12_COMMAND_LIST_TYPE type) const
{
    std::shared_ptr<CommandQueue> commandQueue;
    switch (type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        commandQueue = _directCommandQueue;
        break;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        commandQueue = _computeCommandQueue;
        break;
    case D3D12_COMMAND_LIST_TYPE_COPY:
        commandQueue = _copyCommandQueue;
        break;
    default:
        assert(false && "Invalid command queue type.");
    }

    return commandQueue;
}

void Application::flush()
{
    _directCommandQueue->flush();
    _computeCommandQueue->flush();
    _copyCommandQueue->flush();
}

ComPtr<ID3D12DescriptorHeap> Application::createDescriptorHeap(UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = type;
    desc.NumDescriptors = numDescriptors;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;

    ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    Helper::throwIfFailed(_d3d12Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

    return descriptorHeap;
}

UINT Application::getDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const
{
    return _d3d12Device->GetDescriptorHandleIncrementSize(type);
}

Application::Application(HINSTANCE)
{
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

#if defined(_DEBUG)
    ComPtr<ID3D12Debug> debugInterface;
    Helper::throwIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();
#endif

    WNDCLASSEXW wndClass = { 0 };

    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = &WndProc;
    wndClass.hInstance = _hInstance;
    wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    //wndClass.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(APP_ICON));
    wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wndClass.lpszMenuName = nullptr;
    wndClass.lpszClassName = WINDOW_CLASS_NAME;
    //wndClass.hIconSm = LoadIcon(m_hInstance, MAKEINTRESOURCE(APP_ICON));

    if (!RegisterClassExW(&wndClass))
    {
        MessageBoxA(NULL, "Unable to register the window class.", "Error", MB_OK | MB_ICONERROR);
    }

    _dxgiAdapter = getAdapter(false);
    if (_dxgiAdapter)
    {
        _d3d12Device = createDevice(_dxgiAdapter);
    }
    if (_d3d12Device)
    {
        _directCommandQueue = std::make_shared<CommandQueue>(_d3d12Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
        _computeCommandQueue = std::make_shared<CommandQueue>(_d3d12Device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
        _copyCommandQueue = std::make_shared<CommandQueue>(_d3d12Device, D3D12_COMMAND_LIST_TYPE_COPY);

        _isTearingSupported = checkTearingSupport();
    }
}

Application::~Application()
{
    flush();
}

ComPtr<ID3D12Device2> Application::createDevice(ComPtr<IDXGIAdapter4> adapter)
{
    ComPtr<ID3D12Device2> d3d12Device2;
    Helper::throwIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&d3d12Device2)));
    // NAME_D3D12_OBJECT(d3d12Device2);

    // Enable debug messages in debug mode.
#if defined(_DEBUG)
    ComPtr<ID3D12InfoQueue> pInfoQueue;
    if (SUCCEEDED(d3d12Device2.As(&pInfoQueue)))
    {
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

        // Suppress whole categories of messages
        //D3D12_MESSAGE_CATEGORY Categories[] = {};

        // Suppress messages based on their severity level
        D3D12_MESSAGE_SEVERITY Severities[] =
        {
            D3D12_MESSAGE_SEVERITY_INFO
        };

        // Suppress individual messages by their ID
        D3D12_MESSAGE_ID DenyIds[] = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
        };

        D3D12_INFO_QUEUE_FILTER NewFilter = {};
        //NewFilter.DenyList.NumCategories = _countof(Categories);
        //NewFilter.DenyList.pCategoryList = Categories;
        NewFilter.DenyList.NumSeverities = _countof(Severities);
        NewFilter.DenyList.pSeverityList = Severities;
        NewFilter.DenyList.NumIDs = _countof(DenyIds);
        NewFilter.DenyList.pIDList = DenyIds;

        Helper::throwIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
    }
#endif

    return d3d12Device2;
}

ComPtr<IDXGIAdapter4> Application::getAdapter(bool bUseWarp)
{
    ComPtr<IDXGIFactory4> dxgiFactory;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    Helper::throwIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

    ComPtr<IDXGIAdapter1> dxgiAdapter1;
    ComPtr<IDXGIAdapter4> dxgiAdapter4;

    if (bUseWarp)
    {
        Helper::throwIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
        Helper::throwIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
    }
    else
    {
        SIZE_T maxDedicatedVideoMemory = 0;
        for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
        {
            DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
            dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

            // Check to see if the adapter can create a D3D12 device without actually 
            // creating it. The adapter with the largest dedicated video memory
            // is favored.
            if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
                SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(),
                    D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)) &&
                dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
            {
                maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
                Helper::throwIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
            }
        }
    }

    return dxgiAdapter4;
}

bool Application::checkTearingSupport()
{
    BOOL allowTearing = FALSE;

    // Rather than create the DXGI 1.5 factory interface directly, we create the
    // DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
    // graphics debugging tools which will not support the 1.5 factory interface 
    // until a future update.
    ComPtr<IDXGIFactory4> factory4;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
    {
        ComPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(factory4.As(&factory5)))
        {
            factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                &allowTearing, sizeof(allowTearing));
        }
    }

    return allowTearing == TRUE;
}
