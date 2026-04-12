#pragma once

#include "ResourceCommon.h"

#include <atomic>

namespace rhi
{
    // ResourceIdGenerator is a utility class responsible for generating unique resource IDs for GPU resources.
    // It uses an atomic counter to ensure thread-safe generation of resource IDs, allowing multiple threads to create resources concurrently without conflicts. 
    // The GenerateID method returns a new unique ResourceID each time it is called, which can be used to identify and manage GPU resources within the rendering system
    class ResourceIdGenerator
    {
    public:
        // Generates a unique ResourceID by incrementing the atomic counter, ensuring thread safety and uniqueness across multiple threads
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
