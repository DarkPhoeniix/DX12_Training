#pragma once

namespace rg
{
    class IDescriptorProvider
    {
    public:
        virtual ~IDescriptorProvider() = default;

        virtual void CreateStaticResourceView(std::shared_ptr<rhi::Buffer> buffer, rhi::ResourceViewType viewType) = 0;
        virtual void CreateStaticResourceView(std::shared_ptr<rhi::Texture> texture, rhi::ResourceViewType viewType) = 0;

        virtual std::uint32_t GetBindlessIndex(rhi::ResourceID, rhi::ResourceViewType viewType) = 0;

        virtual rhi::CPUDescriptor GetDescriptor(rhi::ResourceID resourceID, rhi::ResourceViewType viewType) = 0;
    };

    class ITask
    {
    public:
        virtual ~ITask() = default;

        virtual void AddDependency(const std::string& dependency) = 0; // TODO: refactor this to manage dependencies better/more effectively

        virtual rhi::CommandList* GetCommandList() = 0;

        virtual const std::string& GetName() const = 0;
        virtual void SetName(const std::string& name) = 0;
    };

    class ITaskAllocator
    {
    public:
        virtual ~ITaskAllocator() = default;

        virtual ITask* AllocateTask(rhi::CommandListType type, rhi::PipelineState* rootSignature = nullptr) = 0;
    };
} // namespace rg
