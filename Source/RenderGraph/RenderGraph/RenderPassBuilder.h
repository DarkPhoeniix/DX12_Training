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

        ResourceId CreateResource(const std::string& name, dx12::ResourceDescription desc);
        ResourceId ReadResource(const std::string& name);
        ResourceId WriteResource(const std::string& name);

        ResourceId CreateResourceNew(const std::string& name, dx12::ResourceDescription desc, void* data = nullptr, size_t dataSize = 0);
        ResourceId ReadResourceNew(const std::string& name);
        ResourceId WriteResourceNew(const std::string& name);

    private:
        IRenderPass* _renderPass;
        RenderGraph& _renderGraph;
    };
} // namespace rg

