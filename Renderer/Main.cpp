
#include "stdafx.h"

#include "Application.h"
#include "Render/DXRenderer.h"
#include "Window/Win32Window.h"

#include <dxgidebug.h>
#include <pathcch.h>

void ReportLiveObjects();
void SetWorkingPath();

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ PWSTR lpCmdLine, _In_ int nCmdShow)
{
    int retCode = 0;

    SetWorkingPath();

    {
        Application::Init(hInstance);
        std::shared_ptr<Core::Win32Window> mainWindow = Application::CreateWin32Window(1280, 720, L"DX12 Sandbox");
        std::shared_ptr<DXRenderer> demo = std::make_shared<DXRenderer>(mainWindow->GetWindowHandle());
        mainWindow->AddEventListener(demo.get());
        {
            retCode = Application::Instance()->Run(demo);
        }
        Application::Quit();
    }

    ReportLiveObjects();

    return retCode;
}

void ReportLiveObjects()
{
    Sleep(200);
    IDXGIDebug* dxgiDebug;
    DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));

    dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
    dxgiDebug->Release();
}

void SetWorkingPath()
{
    // Set the working directory to the path of the executable.
    WCHAR path[MAX_PATH];
    HMODULE hModule = GetModuleHandleW(NULL);
    if (GetModuleFileNameW(hModule, path, MAX_PATH) > 0)    // Retrieves the path of the executable file of the current process
    {
        PathCchRemoveFileSpec(path, MAX_PATH);              // Removes the last element in a path string, whether that element is a file name or a directory name
        SetCurrentDirectoryW(path);                         // Changes the current directory for the current process
    }
    else
    {
        Logger::Log(LogType::Error, "Failed to retrieve the path of the executable file");
    }
}
