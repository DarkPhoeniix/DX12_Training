#include "RenderGraphPCH.h"

#include "RenderPassBuilder.h"

namespace rg
{
    RenderPassBuilder::RenderPassBuilder(RenderGraph& renderGraph, IRenderPass* renderPass)
        : _renderGraph(renderGraph), _renderPass(renderPass)
    {
    }

    ResourceId RenderPassBuilder::CreateResourceVirtual(const std::string& name)
    {
        ResourceId resourceId = _renderGraph._context.CreateResourceVirtual(name);

        _renderPass->_creates.push_back(resourceId);
        _renderPass->_reads.push_back(resourceId);
        _renderPass->_writes.push_back(resourceId);

        return resourceId;
    }

    ResourceId RenderPassBuilder::CreateResource(const std::string& name, dx12::ResourceDescription desc, void* data /*= nullptr*/, size_t dataSize /*= 0*/)
    {
        ResourceId resourceId = _renderGraph._context.CreateResource(name, desc, data, dataSize);

        _renderPass->_creates.push_back(resourceId);
        _renderPass->_reads.push_back(resourceId);
        _renderPass->_writes.push_back(resourceId);

        return resourceId;
    }

    ResourceId RenderPassBuilder::ReadResource(const std::string& name)
    {
        ResourceId resourceId = _renderGraph._context.ReadResource(name);

        _renderPass->_reads.push_back(resourceId);

        return resourceId;
    }

    ResourceId RenderPassBuilder::WriteResource(const std::string& name)
    {
        ResourceId resourceId = _renderGraph._context.WriteResource(name);

        _renderPass->_reads.push_back(resourceId);
        _renderPass->_writes.push_back(resourceId);

        return resourceId;
    }
} // namespace rg
