#pragma once

#include "NsightAftermathHelpers.h"
#include "../ICommandListCrashContext.h"

namespace tracking
{
    class NsightAftermathCommandListContext : public ICommandListCrashContext
    {
    public:
        NsightAftermathCommandListContext();
        ~NsightAftermathCommandListContext() = default;

        // Inherited via ICommandListCrashContext
        void Initialize(ID3D12GraphicsCommandList* commandList) override;
        void SetMarker(const char* marker) override;

    private:
        ID3D12GraphicsCommandList* _commandList;
        GFSDK_Aftermath_ContextHandle _commandListCrashContext;
    };
} // namespace tracking
