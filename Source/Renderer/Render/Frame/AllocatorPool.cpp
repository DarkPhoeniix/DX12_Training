#include "RendererPCH.h"

#include "AllocatorPool.h"

void AllocatorPool::Init(rhi::Device* device)
{
    // TODO: refactor
    Make(device, streams, 128, rhi::CommandListType::Graphics);
    Make(device, computes, 128, rhi::CommandListType::Compute);
    Make(device, copies, 4, rhi::CommandListType::Copy);
}

Executor* AllocatorPool::Obtain(rhi::CommandListType type)
{
    std::vector<Executor>* res;
    switch (type)
    {
    case rhi::CommandListType::Graphics:
        res = &streams;
        break;
    case rhi::CommandListType::Compute:
        res = &computes;
        break;
    case rhi::CommandListType::Copy:
        res = &copies;
        break;
    default:
        UNREACHABLE("Unsupported command list type.");
        res = &streams;
        break;
    }

    for (auto& exec : *res)
    {
        if (exec.IsFree())
        {
            return &exec;
        }
    }

    LOG_CRITICAL("No free executors available for the command list type.");
    return nullptr;
}

void AllocatorPool::Make(rhi::Device* device, std::vector<Executor>& vecExec, unsigned int size, rhi::CommandListType type)
{
    for (size_t i = 0; i < size; ++i)
    {
        vecExec.emplace_back(device);
    }

    for (auto& exec : vecExec)
    {
        exec.Allocate(type);
        exec.SetFree(true);
    }
}
