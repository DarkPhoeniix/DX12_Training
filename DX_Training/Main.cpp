
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

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
    int retCode = 0;

    // Set the working directory to the path of the executable.
    WCHAR path[MAX_PATH];
    HMODULE hModule = GetModuleHandleW(NULL);
    if (GetModuleFileNameW(hModule, path, MAX_PATH) > 0)
    {
        PathCchRemoveFileSpec(path, MAX_PATH);
        SetCurrentDirectoryW(path);
    }

    Application::create(hInstance);
    {
        std::shared_ptr<RenderCubeExample> demo = std::make_shared<RenderCubeExample>(L"D3DX12 Render Cube Training", 1280, 720);
        retCode = Application::get().run(demo);
    }
    Application::destroy();

    atexit(&ReportLiveObjects);

    return retCode;
}
