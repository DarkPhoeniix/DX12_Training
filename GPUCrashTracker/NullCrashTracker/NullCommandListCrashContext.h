#pragma once

#include "../ICommandListCrashContext.h"

namespace tracking
{
    class NullCommandListCrashContext final : public ICommandListCrashContext
    {
    public:
        NullCommandListCrashContext(std::shared_ptr<IGPUCrashTracker> crashTracker)
            : ICommandListCrashContext(crashTracker)
        {   }

        // Inherited via ICommandListCrashContext
        void Initialize(ID3D12GraphicsCommandList*) override
        {   }
        void SetMarker(const char*) override
        {   }
    };
} // namespace tracking
