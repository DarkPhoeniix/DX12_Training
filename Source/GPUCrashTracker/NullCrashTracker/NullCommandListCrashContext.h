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

        void Initialize([[maybe_unused]] rhi::CommandList* commandList) override
        {   }

        void SetMarker(const char*) override
        {   }
    };
} // namespace tracking
