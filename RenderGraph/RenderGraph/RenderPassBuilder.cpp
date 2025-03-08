#include "RenderGraphPCH.h"

#include "RenderPassBuilder.h"

namespace rg
{
    RenderPassBuilder::RenderPassBuilder(RenderGraph& renderGraph, IRenderPass* renderPass)
        : _renderGraph(renderGraph), _renderPass(renderPass)
    {   }

    ResourceId RenderPassBuilder::CreateResource(std::string name, dx12::ResourceDescription desc)
    {
        ResourceId resourceId = _renderGraph._context.CreateResource(name, desc);

        _renderPass->_creates.push_back(resourceId);
        _renderPass->_reads.push_back(resourceId);
        _renderPass->_writes.push_back(resourceId);

        return resourceId;
    }

    ResourceId RenderPassBuilder::ReadResource(std::string name)
    {
        ResourceId resourceId = _renderGraph._context.ReadResource(name);

        _renderPass->_reads.push_back(resourceId);

        return resourceId;
    }

    ResourceId RenderPassBuilder::WriteResource(std::string name)
    {
        ResourceId resourceId = _renderGraph._context.ReadResource(name);

        _renderPass->_reads.push_back(resourceId);
        _renderPass->_writes.push_back(resourceId);

        return resourceId;
    }
}
