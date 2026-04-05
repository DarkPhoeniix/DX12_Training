#pragma once

namespace rg
{
    class IDescriptorProvider
    {
    public:
        virtual void CreateStaticResourceView(rhi::ResourceID resourceID, rhi::ResourceViewType viewType) = 0;

        virtual std::uint32_t GetBindlessIndex(rhi::ResourceID, rhi::ResourceViewType viewType) = 0;

        virtual rhi::CPUDescriptor GetDescriptor(rhi::ResourceID resourceID, rhi::ResourceViewType viewType) = 0;
    };

    class ITask
    {
    public:
        virtual void AddDependency(const std::string& dependency) = 0; // TODO: refactor this to manage dependencies better/more effectively

        virtual rhi::CommandList* GetCommandList() = 0;

        virtual const std::string& GetName() const = 0;
        virtual void SetName(const std::string& name) = 0;
    };

    class ITaskAllocator
    {
    public:
        virtual ITask* AllocateTask(rhi::CommandListType type) = 0;
    };
} // namespace rg
