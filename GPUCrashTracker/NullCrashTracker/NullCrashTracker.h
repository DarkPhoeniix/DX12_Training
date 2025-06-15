#pragma once

#include "../IGPUCrashTracker.h"

namespace tracking
{
    class NullCrashTracker : public IGPUCrashTracker
    {
    public:
        // Inherited via IGPUCrashTracker
        void Enable() override {}
        void Initialize(ID3D12Device2*) override {}
        void WaitUntilCrashDumpFinished() override {};
    };
} // namespace tracking
