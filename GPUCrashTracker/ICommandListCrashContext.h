#pragma once

#include <string>

struct ID3D12GraphicsCommandList;

namespace tracking
{
    class ICommandListCrashContext
    {
    public:
        virtual ~ICommandListCrashContext() = default;

        virtual void Initialize(ID3D12GraphicsCommandList* commandList) = 0;
        virtual void SetMarker(const char* marker) = 0;
        virtual void SetMarker(const std::string& marker) final { SetMarker(marker.c_str()); }
    };
} // namespace tracking
