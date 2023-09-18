#pragma once

#include "stdafx.h"

inline void throwIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw std::exception();
    }
}

