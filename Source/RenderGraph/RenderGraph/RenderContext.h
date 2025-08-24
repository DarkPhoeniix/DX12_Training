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
        RenderContext(ResourceTable& resourceTable, TextureManager& textureManager);

        const Frame* GetFrame() const;
        std::uint32_t GetFrameIndex() const;

        ResourceTable& GetResourceTable();
		TextureManager& GetTextureManager();

        void BindBindlessTable(dx12::CommandList& commandList) const;

        std::shared_ptr<dx12::Resource> GetResourceNew(ResourceId id);

        const DescriptorHandle& GetStaticResourceHandle(dx12::RenderTargetView rtv) const;
        const DescriptorHandle& GetStaticResourceHandle(dx12::DepthStencilView dsv) const;
        const DescriptorHandle& GetStaticResourceHandle(dx12::ShaderResourceView srv) const;
        const DescriptorHandle& GetStaticResourceHandle(dx12::UnorderedAccessView uav) const;
        const DescriptorHandle& GetStaticResourceHandle(dx12::ConstantBufferView cbv) const;

        //DescriptorHandle GetTransientResourceHandle(dx12::RenderTargetView rtv);
        //DescriptorHandle GetTransientResourceHandle(dx12::DepthStencilView dsv);
        //DescriptorHandle GetTransientResourceHandle(dx12::ShaderResourceView srv);
        //DescriptorHandle GetTransientResourceHandle(dx12::UnorderedAccessView uav);
        //DescriptorHandle GetTransientResourceHandle(dx12::ConstantBufferView cbv);

    private:
        friend class RenderGraph;
        friend class RenderPassBuilder;

        ResourceId CreateResourceVirtual(const std::string& name);
        ResourceId CreateResourceNew(const std::string& name, dx12::ResourceDescription desc, void* data = nullptr, size_t dataSize = 0);
        ResourceId ReadResourceNew(const std::string& name);
        ResourceId WriteResourceNew(const std::string& name);

        void FillResource(std::shared_ptr<dx12::Resource> resource, void* data, size_t dataSize = 0);

        std::unordered_map<std::string, ResourceId> _mapNameToIdNew;
        std::unordered_map<ResourceId, std::shared_ptr<dx12::Resource>> _resourcesNew;

        Frame* _frame;

        ResourceTable& _resourceTableNew;
		TextureManager& _textureManager;
    };
} // namespace rg
