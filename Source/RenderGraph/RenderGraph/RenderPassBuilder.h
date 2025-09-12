#pragma once

#include "RenderGraph.h"

namespace rg
{
    class IRenderPass;
    class RenderGraph;

    class RenderPassBuilder
    {
    public:
        RenderPassBuilder(RenderGraph& renderGraph, IRenderPass* renderPass);
        RenderPassBuilder(const RenderPassBuilder&) = default;
        RenderPassBuilder(RenderPassBuilder&&) = default;
        virtual ~RenderPassBuilder() = default;

        RenderPassBuilder& operator=(const RenderPassBuilder&) = default;
        RenderPassBuilder& operator=(RenderPassBuilder&&) = default;

        ResourceId CreateResourceVirtual(const std::string& name);
        ResourceId CreateResource(const std::string& name, dx12::ResourceDescription desc, void* data = nullptr, size_t dataSize = 0);
        ResourceId ReadResource(const std::string& name);
        ResourceId WriteResource(const std::string& name);

        void DeclareBuffer(const std::string& name, const dx12::ResourceDescription& desc, void* data = nullptr, size_t dataSize = 0);
        void DeclareTexture(const std::string& name, const dx12::ResourceDescription& desc, void* data = nullptr, size_t dataSize = 0);
        void DeclareVirtualResource(const std::string& name);

        [[nodiscard]] ResourceId ReadBuffer(const std::string& name);
        [[nodiscard]] ResourceId WriteBuffer(const std::string& name);
        [[nodiscard]] ResourceId CopySrcBuffer(const std::string& name);
        [[nodiscard]] ResourceId CopyDstBuffer(const std::string& name);
        [[nodiscard]] ResourceId IndirectArgBuffer(const std::string& name);

        [[nodiscard]] ResourceId ReadTexture(const std::string& name);
        [[nodiscard]] ResourceId WriteTexture(const std::string& name);
        [[nodiscard]] ResourceId CopySrcTexture(const std::string& name);
        [[nodiscard]] ResourceId CopyDstTexture(const std::string& name);
        [[nodiscard]] ResourceId RenderTarget(const std::string& name);
        [[nodiscard]] ResourceId DepthStencilRead(const std::string& name);
        [[nodiscard]] ResourceId DepthStencilWrite(const std::string& name);

    private:
        IRenderPass* _renderPass;
        RenderGraph& _renderGraph;
    };
} // namespace rg

