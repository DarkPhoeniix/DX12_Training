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
    TaskGPU(rhi::Device* device, rhi::CommandList* commandList);
    TaskGPU(const TaskGPU& other) = delete;
    TaskGPU(TaskGPU&& other) noexcept = default;
    ~TaskGPU() override;

    TaskGPU& operator=(const TaskGPU& other) = delete;
    TaskGPU& operator=(TaskGPU&& other) noexcept = default;

    void Reset(rhi::PipelineState* pipelineState = nullptr);

    rhi::CommandList* GetCommandList() override;

    void SetFence(rhi::Fence* fence);
    rhi::Fence* GetFence() const;

    void AddDependency(const std::string& taskName);
    std::vector<std::string> GetDependencies() const;

    rhi::CommandListType GetType() const;

    void SetName(const std::string& name) override;
    const std::string& GetName() const override;

    tracking::ICommandListCrashContext* GetCrashContext();

private:
    rhi::CommandList* _commandList;
    std::unique_ptr<tracking::ICommandListCrashContext> _commandListCrashContext;

    rhi::Fence* _fence = nullptr;
    std::vector<std::string> _dependencies;

    rhi::CommandListType _type;
    std::string _name;
};

