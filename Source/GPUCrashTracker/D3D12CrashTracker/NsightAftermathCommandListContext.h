#pragma once

#include "NsightAftermathHelpers.h"
#include "../ICommandListCrashContext.h"

namespace tracking
{
    class NsightAftermathCommandListContext final : public ICommandListCrashContext
    {
    public:
        NsightAftermathCommandListContext(IGPUCrashTracker* crashTracker);
        ~NsightAftermathCommandListContext() = default;

        // Inherited via ICommandListCrashContext
        void Initialize(rhi::CommandList* commandList) override;
        void SetMarker(const char* marker) override;

    private:
        ID3D12GraphicsCommandList* _commandList;
        GFSDK_Aftermath_ContextHandle _commandListCrashContext;

        IGPUCrashTracker* _crashTracker;
    };
} // namespace tracking
