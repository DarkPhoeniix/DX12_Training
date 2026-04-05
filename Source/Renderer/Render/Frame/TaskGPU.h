#pragma once

#include "GPUCrashTracker/ICommandListCrashContext.h"
#include "RenderGraph/Interfaces.h"

namespace rhi
{
    class CommandList;
    class Fence;
} // namespace rhi

class TaskGPU : public rg::ITask
{
public:
    TaskGPU(rhi::Device* device);
    ~TaskGPU();

    void AddCommandList(rhi::CommandList* commandList);
    rhi::CommandList* GetCommandList() override;

    void SetFence(rhi::Fence* fence);
    rhi::Fence* GetFence() const;
    UINT64 GetFenceValue() const;

    void AddDependency(const std::string& taskName);
    std::vector<std::string> GetDependencies() const;

    rhi::CommandListType GetType() const;

    void SetName(const std::string& name) override;
    const std::string& GetName() const override;

    std::shared_ptr<tracking::ICommandListCrashContext> GetCrashContext();

private:
    std::vector<rhi::CommandList*> _commandLists;
    std::shared_ptr<tracking::ICommandListCrashContext> _commandListCrashContext;

    rhi::Fence* _fence = nullptr;
    std::vector<std::string> _dependencies;

    std::string _name;
};

