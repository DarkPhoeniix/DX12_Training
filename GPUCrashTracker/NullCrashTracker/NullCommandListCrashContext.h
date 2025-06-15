#pragma once

#include "../ICommandListCrashContext.h"

namespace tracking
{
    class NullCommandListCrashContext : public ICommandListCrashContext
    {
    public:
        // Inherited via ICommandListCrashContext
        void Initialize(ID3D12GraphicsCommandList* commandList) override {};
        void SetMarker(const char* marker) override {};
    };
} // namespace tracking
