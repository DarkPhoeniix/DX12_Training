#include "D3D12CommandListPool.h"
#include "D3D12CommandListPool.h"

#include "RHI_PCH.h"

#include "D3D12CommandListPool.h"

namespace rhi::d3d12
{
    D3D12CommandListPool::D3D12CommandListPool(Device* device)
        : _device(device)
    {
    }

    CommandList* D3D12CommandListPool::AllocateCommandList(CommandListType type)
    {
        std::deque<Executor>* executors = nullptr;

        switch (type)
        {
        case CommandListType::Graphics:
            executors = &_graphicsExecutors;
            break;
        case CommandListType::Compute:
            executors = &_computeExecutors;
            break;
        case CommandListType::Copy:
            executors = &_copyExecutors;
            break;
        default:
            UNREACHABLE("Unsupported command list type.");
            return nullptr;
        }

        Executor* executor = nullptr;

        for (auto& e : *executors)
        {
            if (e.IsFree)
            {
                executor = &e;

                executor->CommandList = e.CommandList;
                executor->IsFree = false; // Mark the command list as in use

                break;
            }
        }

        if (!executor)
        {
            // No available command list, create a new one
            const auto& newCommandList = _commandLists.emplace_back(_device->CreateCommandList(type));
            // Add the new command list to the pool and mark it as in use
            auto& newExecutor = executors->emplace_back(newCommandList.get(), false);

            executor = &newExecutor;

            LOG_DEBUG("Created new command list of type {}. Total command lists in pool: {}", static_cast<int>(type), executors->size());
        }

        return executor->CommandList;
    }

    void D3D12CommandListPool::FreeCommandList(CommandList* commandList)
    {
        std::deque<Executor>* executors = nullptr;

        switch (commandList->GetCommandListType())
        {
        case CommandListType::Graphics:
            executors = &_graphicsExecutors;
            break;
        case CommandListType::Compute:
            executors = &_computeExecutors;
            break;
        case CommandListType::Copy:
            executors = &_copyExecutors;
            break;
        default:
            UNREACHABLE("Unsupported command list type.");
            return;
        }

        for (auto& e : *executors)
        {
            if (e.CommandList == commandList)
            {
                e.IsFree = true; // Mark the command list as free
                return;
            }
        }

        LOG_ERROR("Command list not found in the pool.");
    }
} // namespace rhi::d3d12
