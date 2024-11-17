
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
}
