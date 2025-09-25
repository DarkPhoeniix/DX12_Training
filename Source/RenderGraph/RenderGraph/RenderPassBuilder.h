#pragma once

#include "RenderGraphResourceId.h"
#include "RenderGraph.h"

namespace rg
{
    class IRenderPass;

    class RenderPassBuilder
    {
    public:
        RenderPassBuilder(RenderGraph& renderGraph, IRenderPass* renderPass);
        RenderPassBuilder(const RenderPassBuilder&) = default;
        RenderPassBuilder(RenderPassBuilder&&) = default;
        virtual ~RenderPassBuilder() = default;

        RenderPassBuilder& operator=(const RenderPassBuilder&) = default;
        RenderPassBuilder& operator=(RenderPassBuilder&&) = default;

        RGResourceId CreateResourceVirtual(const std::string& name);
        RGResourceId CreateResource(const std::string& name, dx12::ResourceDescription desc, void* data = nullptr, size_t dataSize = 0);
        RGResourceId ReadResource(const std::string& name);
        RGResourceId WriteResource(const std::string& name);

        RGResourceId DeclareBuffer(const std::string& name, const dx12::ResourceDescription& desc, void* data = nullptr, size_t dataSize = 0);
        RGResourceId DeclareTexture(const std::string& name, const dx12::ResourceDescription& desc, void* data = nullptr, size_t dataSize = 0);
        RGResourceId DeclareVirtualResource(const std::string& name);

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

    private:
        IRenderPass* _renderPass;
        RenderGraph& _renderGraph;
    };
} // namespace rg

