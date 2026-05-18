
#include "EditorPCH.h"

#include "Application.h"
#include "Core/Renderer.h"
#include "Window/Win32Window.h"

#include <pathcch.h>
#include <cwctype>

void SetWorkingPath();
void ParseCommandLine(ApplicationConfig& config, PWSTR lpCmdLine);

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ PWSTR lpCmdLine, _In_ int nCmdShow)
{
    int retCode = 0;

    SetWorkingPath();

    {
        Application::Init(hInstance);

        ApplicationConfig config = 
        { 
            .WindowWidth = 1280, 
            .WindowHeight = 720, 
            .Name = L"Equinox Engine", 
            .VSync = false 
        };
        ParseCommandLine(config, lpCmdLine);

        retCode = Application::Instance()->Run(config);

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

void ParseCommandLine(ApplicationConfig& config, PWSTR lpCmdLine)
{
    auto parseToken = [](const std::wstring& token) -> std::pair<std::wstring, std::wstring>
        {
            size_t equalPos = token.find(L'=');
            if (equalPos != std::wstring::npos)
            {
                std::wstring key = token.substr(0, equalPos);
                std::wstring value = token.substr(equalPos + 1);
                return { key, value };
            }
            else
            {
                return { token, L"" };
            }
        };

    std::wstring temp(lpCmdLine);
    std::wstringstream stream(temp);

    while (stream)
    {
        std::wstring arg;
        stream >> arg;
        
        auto [key, value] = parseToken(arg);

        if (key == L"-width" && !value.empty())
        {
            config.WindowWidth = std::stoul(value);
        }
        else if (key == L"-height" && !value.empty())
        {
            config.WindowHeight = std::stoul(value);
        }
        else if (key == L"-scene" && !value.empty())
        {
            config.ScenePath = value;
        }
        else if (key == L"-vsync")
        {
            if (key == L"false")
            {
                config.VSync = false;
            }
            else
            {
                config.VSync = true;
            }
        }
    }
}
