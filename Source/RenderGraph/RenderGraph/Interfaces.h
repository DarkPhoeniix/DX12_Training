#pragma once

namespace rg
{
    class IDescriptorProvider
    {
    public:
        virtual void CreateResourceView(rhi::ResourceID resourceID, rhi::ResourceViewType viewType) = 0;

        virtual rhi::CPUDescriptor GetCPUDescriptor(rhi::ResourceID resourceID, rhi::ResourceViewType viewType) = 0;
        virtual rhi::GPUDescriptor GetGPUDescriptor(rhi::ResourceID resourceID, rhi::ResourceViewType viewType) = 0;
    };

    class ITask
    {
    public:
        virtual void AddDependency(const std::string& dependency) = 0; // TODO: refactor this to manage dependencies better/more effectively

        virtual rhi::CommandList* GetCommandList() = 0;

        virtual std::string GetName() const = 0;
        virtual void SetName(const std::string& name) = 0;
    };

    class ITaskAllocator
    {
    public:
        virtual ITask* AllocateTask(rhi::CommandListType type) = 0;
    };
} // namespace rg
