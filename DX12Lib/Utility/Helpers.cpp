
#include "stdafx.h"

#include "Helpers.h"

#include <filesystem>

namespace Helper
{
    std::string HrToString(HRESULT hr)
    {
        char s_str[64] = {};
        sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
        return std::string(s_str);
    }

    void throwIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            Logger::Log(LogType::Error, "Raised an exception with code " + hr);
            throw HrException(hr);
        }
    }

    Json::Value ParseJson(const std::string& filepath)
    {
        if (!std::filesystem::exists(filepath))
        {
            Logger::Log(LogType::Error, "Failed to load " + filepath);

            return {};
        }

        std::ifstream file(filepath, std::ios_base::binary);
		file.open(filepath, std::ios_base::binary);
        Json::Value root;

        file >> root;

        return root;
    }
}
