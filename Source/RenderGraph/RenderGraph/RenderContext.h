#pragma once

#include "RenderGraphResourceId.h"

#include "Core/ResourceTable.h"
#include "Core/TextureManager.h"
#include "Render/Frame/Frame.h"

namespace rg
{
    class RenderGraph;
    class RenderPassBuilder;

    class RenderContext
    {
    public:
        RenderContext();

        void Init(ResourceTable& resourceTable, TextureManager& textureManager);

        const Frame* GetFrame() const;
        std::uint32_t GetFrameIndex() const;

		TextureManager& GetTextureManager();

        void BindBindlessTable(dx12::CommandList& commandList) const;

        std::shared_ptr<dx12::Resource> GetResource(RGResourceId id);

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

        RGResourceId CreateResourceVirtual(const std::string& name);
        RGResourceId CreateResource(const std::string& name, dx12::ResourceDescription desc, void* data = nullptr, size_t dataSize = 0);
        RGResourceId ReadResource(const std::string& name);
        RGResourceId WriteResource(const std::string& name);

        RGResourceId DeclareBuffer(const std::string& name, const dx12::ResourceDescription& desc, void* data = nullptr, size_t dataSize = 0);
        RGResourceId DeclareTexture(const std::string& name, const dx12::ResourceDescription& desc, void* data = nullptr, size_t dataSize = 0);

        [[nodiscard]] RGBufferReadId ReadBuffer(const std::string& name);
        [[nodiscard]] RGBufferWriteId WriteBuffer(const std::string& name);
        [[nodiscard]] RGBufferUploadId UploadBuffer(const std::string& name);
        [[nodiscard]] RGBufferCopySrcId CopySrcBuffer(const std::string& name);
        [[nodiscard]] RGBufferCopyDstId CopyDstBuffer(const std::string& name);
        [[nodiscard]] RGBufferIndirectArgsId IndirectArgBuffer(const std::string& name);

        [[nodiscard]] RGTextureReadId ReadTexture(const std::string& name);
        [[nodiscard]] RGTextureWriteId WriteTexture(const std::string& name);
        [[nodiscard]] RGTextureCopySrcId CopySrcTexture(const std::string& name);
        [[nodiscard]] RGTextureCopyDstId CopyDstTexture(const std::string& name);
        [[nodiscard]] RGTextureRenderTargetId RenderTarget(const std::string& name);
        [[nodiscard]] RGTextureDepthStencilReadId DepthStencilRead(const std::string& name);
        [[nodiscard]] RGTextureDepthStencilWriteId DepthStencilWrite(const std::string& name);

        [[nodiscard]] RGVirtualResourceReadId ReadVirtualResource(const std::string& name);
        [[nodiscard]] RGVirtualResourceWriteId WriteVirtualResource(const std::string& name);

        void FillBuffer(std::shared_ptr<dx12::Resource> resource, void* data, size_t dataSize = 0);
        void FillTexture(std::shared_ptr<dx12::Resource> resource, void* data, size_t dataSize = 0);

        std::unordered_map<std::string, RGResourceId> _mapNameToId;
        std::unordered_map<RGResourceId, std::shared_ptr<dx12::Resource>> _mapIdToResource;

        Frame* _frame;

        ResourceTable* _resourceTable;
		TextureManager* _textureManager;
    };
} // namespace rg
