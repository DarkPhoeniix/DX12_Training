
#include "stdafx.h"

#include "Application.h"
#include "RenderCubeExample.h"

#include <dxgidebug.h>

#include "pathcch.h"

void ReportLiveObjects()
{
    IDXGIDebug1* dxgiDebug;
    DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));

    dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
    dxgiDebug->Release();
}

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ PWSTR lpCmdLine, _In_ int nCmdShow)
{
    int retCode = 0;

    // Set the working directory to the path of the executable.
    WCHAR path[MAX_PATH];
    HMODULE hModule = GetModuleHandleW(NULL);
    if (GetModuleFileNameW(hModule, path, MAX_PATH) > 0)    // Retrieves the path of the executable file of the current process
    {
        PathCchRemoveFileSpec(path, MAX_PATH);              // Removes the last element in a path string, whether that element is a file name or a directory name
        SetCurrentDirectoryW(path);                         // Changes the current directory for the current process
    }

    Application::create(hInstance);
    {
        std::shared_ptr<RenderCubeExample> demo = std::make_shared<RenderCubeExample>(L"D3DX12 Render Cube Training", 1280, 720, false);
        retCode = Application::get().run(demo);
    }
    Application::destroy();

    atexit(&ReportLiveObjects);

    return retCode;
}
