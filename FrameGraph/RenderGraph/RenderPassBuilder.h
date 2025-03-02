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

        ResourceId CreateResource(std::string name, dx12::ResourceDescription desc);
        ResourceId ReadResource(std::string name);
        ResourceId WriteResource(std::string name);

    private:
        IRenderPass* _renderPass;
        RenderGraph& _renderGraph;
    };
}

