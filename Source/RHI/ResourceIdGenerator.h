#pragma once

#include "ResourceCommon.h"

#include <atomic>

namespace rhi
{
    class ResourceIdGenerator
    {
    public:
        static [[nodiscard]] ResourceID GenerateID()
        {
            ResourceID id = _currentID.fetch_add(1, std::memory_order_relaxed);
            return id;
        }

    private:
        static std::atomic<ResourceID> _currentID;
    };

    std::atomic<ResourceID> ResourceIdGenerator::_currentID = 0;
} // namespace rhi::helpers
