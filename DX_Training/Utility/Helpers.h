#pragma once

namespace Helper
{
    inline void throwIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw std::exception();
        }
    }
}
