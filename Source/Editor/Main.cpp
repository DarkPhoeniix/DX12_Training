
#include "EditorPCH.h"

#include "Application.h"
#include "Core/Renderer.h"
#include "Window/Win32Window.h"

#include <pathcch.h>

void SetWorkingPath();

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ PWSTR lpCmdLine, _In_ int nCmdShow)
{
    int retCode = 0;

    SetWorkingPath();

    {
        Application::Init(hInstance);

        std::wstring temp(lpCmdLine);
        WindowParams params = { 1280, 720, L"Equinox Engine", false };

        retCode = Application::Instance()->Run(params, std::string(temp.cbegin(), temp.cend()));

        Application::Quit();
    }

    return retCode;
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
        LOG_CRITICAL("Failed to retrieve the path of the executable file");
    }
}
