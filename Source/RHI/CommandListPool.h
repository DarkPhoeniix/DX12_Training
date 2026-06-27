#pragma once

#include "CommandList.h"

namespace rhi
{
    // Container for managing and reusing command lists. Supports auto-expansion of the pool when no free command lists are available. 
    // It provides methods for allocating and freeing command lists, allowing efficient management of GPU command resources.
    class CommandListPool
    {
    public:
        virtual ~CommandListPool() = default;

        // Allocates a command list of the specified type from the pool. If no free command lists are available, the pool will automatically expand to accommodate the request.
        [[nodiscard]] virtual CommandList* AllocateCommandList(CommandListType type) = 0;
        // Frees a command list back to the pool, marking it as available for future allocations. This allows for efficient reuse of command lists and helps to minimize resource creation overhead.
        virtual void FreeCommandList(CommandList* commandList) = 0;
    };
} // namespace rhi
