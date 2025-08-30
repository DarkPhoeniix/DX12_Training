#pragma once

#include "Core/ResourceTable.h"
#include "Core/TextureManager.h"
#include "Render/Frame/CacheGPU.h"
#include "Render/Frame/Frame.h"

namespace rg
{
    class RenderGraph;
    class RenderPassBuilder;

    using ResourceId = std::uint64_t;

    class RenderContext
    {
    public:
        RenderContext();

        void Init(ResourceTable& resourceTable, TextureManager& textureManager);

        const Frame* GetFrame() const;
        std::uint32_t GetFrameIndex() const;

        ResourceTable& GetResourceTable();
		TextureManager& GetTextureManager();

        void BindBindlessTable(dx12::CommandList& commandList) const;

        std::shared_ptr<dx12::Resource> GetResource(ResourceId id);

        DescriptorHandle GetStaticResourceHandle(const dx12::RenderTargetView& rtv) const;
        DescriptorHandle GetStaticResourceHandle(const dx12::DepthStencilView& dsv) const;
        DescriptorHandle GetStaticResourceHandle(const dx12::ShaderResourceView& srv) const;
        DescriptorHandle GetStaticResourceHandle(const dx12::UnorderedAccessView& uav) const;
        DescriptorHandle GetStaticResourceHandle(const dx12::ConstantBufferView& cbv) const;

        DescriptorHandle GetTransientResourceHandle(const dx12::RenderTargetView& rtv) const;
        DescriptorHandle GetTransientResourceHandle(const dx12::DepthStencilView& dsv) const;
        DescriptorHandle GetTransientResourceHandle(const dx12::ShaderResourceView& srv) const;
        DescriptorHandle GetTransientResourceHandle(const dx12::UnorderedAccessView& uav) const;
        DescriptorHandle GetTransientResourceHandle(const dx12::ConstantBufferView& cbv) const;

    private:
        friend class RenderGraph;
        friend class RenderPassBuilder;

        ResourceId CreateResourceVirtual(const std::string& name);
        ResourceId CreateResource(const std::string& name, dx12::ResourceDescription desc, void* data = nullptr, size_t dataSize = 0);
        ResourceId ReadResource(const std::string& name);
        ResourceId WriteResource(const std::string& name);

        void FillResource(std::shared_ptr<dx12::Resource> resource, void* data, size_t dataSize = 0);

        std::unordered_map<std::string, ResourceId> _mapNameToId;
        std::unordered_map<ResourceId, std::shared_ptr<dx12::Resource>> _mapIdToResource;

        Frame* _frame;

        ResourceTable* _resourceTable;
		TextureManager* _textureManager;
    };
} // namespace rg
