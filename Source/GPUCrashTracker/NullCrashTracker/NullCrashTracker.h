#pragma once

#include "../IGPUCrashTracker.h"

namespace tracking
{
    class NullCrashTracker final : public IGPUCrashTracker
    {
    public:
        void Enable() override
        {   }

        void Initialize([[maybe_unused]] rhi::Device* device) override
        {   }

        void WaitUntilCrashDumpFinished() override
        {   }
    };
} // namespace tracking
