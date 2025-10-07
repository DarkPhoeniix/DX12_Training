#include "RenderGraphPCH.h"

#include "RenderPassBuilder.h"

namespace rg
{
    RenderPassBuilder::RenderPassBuilder(RenderGraph& renderGraph, IRenderPass* renderPass)
        : _renderGraph(renderGraph), _renderPass(renderPass)
    {
    }

    void RenderPassBuilder::DeclareBuffer(const std::string& name, const dx12::ResourceDescription& desc, void* data, size_t dataSize)
    {
        RGResourceId id = _renderGraph._context.DeclareBuffer(name, desc, data, dataSize);

        _renderPass->_creates.push_back(id);
        _renderPass->_reads.push_back(id);
        _renderPass->_writes.push_back(id);
    }
    
    void RenderPassBuilder::DeclareTexture(const std::string& name, const dx12::ResourceDescription& desc, void* data, size_t dataSize)
    {
        RGResourceId id = _renderGraph._context.DeclareTexture(name, desc, data, dataSize);

        _renderPass->_creates.push_back(id);
        _renderPass->_reads.push_back(id);
        _renderPass->_writes.push_back(id);
    }

    RGBufferReadId RenderPassBuilder::ReadBuffer(const std::string& name)
    {
        RGBufferReadId id = _renderGraph._context.ReadBuffer(name);

        _renderPass->_resourceStateMap[id] = (_renderPass->GetType() == RenderPassType::Compute) ? 
            dx12::ResourceState::NonPixelShaderResource : dx12::ResourceState::PixelShaderResource;

        _renderPass->_reads.push_back(id);

        return id;
    }

    RGBufferWriteId RenderPassBuilder::WriteBuffer(const std::string& name)
    {
        RGBufferWriteId id = _renderGraph._context.WriteBuffer(name);
        _renderPass->_resourceStateMap[id] = dx12::ResourceState::UnorderedAccess;
        _renderPass->_reads.push_back(id);
        _renderPass->_writes.push_back(id);
        return id;
    }

    RGBufferUploadId RenderPassBuilder::UploadBuffer(const std::string& name)
    {
        RGBufferUploadId id = _renderGraph._context.UploadBuffer(name);
        _renderPass->_resourceStateMap[id] = dx12::ResourceState::GenericRead;
        _renderPass->_reads.push_back(id);
        return id;
    }

    RGBufferCopySrcId RenderPassBuilder::CopySrcBuffer(const std::string& name)
    {
        RGBufferCopySrcId id = _renderGraph._context.CopySrcBuffer(name);
        _renderPass->_resourceStateMap[id] = dx12::ResourceState::CopySource;
        _renderPass->_reads.push_back(id);
        return id;
    }

    RGBufferCopyDstId RenderPassBuilder::CopyDstBuffer(const std::string& name)
    {
        RGBufferCopyDstId id = _renderGraph._context.CopyDstBuffer(name);
        _renderPass->_resourceStateMap[id] = dx12::ResourceState::CopyDest;
        _renderPass->_reads.push_back(id);
        _renderPass->_writes.push_back(id);
        return id;
    }

    RGBufferIndirectArgsId RenderPassBuilder::IndirectArgBuffer(const std::string& name)
    {
        RGBufferIndirectArgsId id = _renderGraph._context.IndirectArgBuffer(name);
        _renderPass->_resourceStateMap[id] = dx12::ResourceState::IndirectArgument;
        _renderPass->_reads.push_back(id);
        return id;
    }

    RGTextureReadId RenderPassBuilder::ReadTexture(const std::string& name)
    {
        RGTextureReadId id = _renderGraph._context.ReadTexture(name);
        _renderPass->_resourceStateMap[id] = (_renderPass->GetType() == RenderPassType::Compute) ? 
            dx12::ResourceState::NonPixelShaderResource : dx12::ResourceState::PixelShaderResource;
        _renderPass->_reads.push_back(id);
        return id;
    }

    RGTextureWriteId RenderPassBuilder::WriteTexture(const std::string& name)
    {
        RGTextureWriteId id = _renderGraph._context.WriteTexture(name);
        _renderPass->_resourceStateMap[id] = dx12::ResourceState::UnorderedAccess;
        _renderPass->_reads.push_back(id);
        _renderPass->_writes.push_back(id);
        return id;
    }

    RGTextureCopySrcId RenderPassBuilder::CopySrcTexture(const std::string& name)
    {
        RGTextureCopySrcId id = _renderGraph._context.CopySrcTexture(name);
        _renderPass->_resourceStateMap[id] = dx12::ResourceState::CopySource;
        _renderPass->_reads.push_back(id);
        return id;
    }

    RGTextureCopyDstId RenderPassBuilder::CopyDstTexture(const std::string& name)
    {
        RGTextureCopyDstId id = _renderGraph._context.CopyDstTexture(name);
        _renderPass->_resourceStateMap[id] = dx12::ResourceState::CopyDest;
        _renderPass->_reads.push_back(id);
        _renderPass->_writes.push_back(id);
        return id;
    }

    RGTextureRenderTargetId RenderPassBuilder::RenderTarget(const std::string& name)
    {
        RGTextureRenderTargetId id = _renderGraph._context.RenderTarget(name);
        _renderPass->_resourceStateMap[id] = dx12::ResourceState::RenderTarget;
        _renderPass->_reads.push_back(id);
        _renderPass->_writes.push_back(id);
        return id;
    }

    RGTextureDepthStencilReadId RenderPassBuilder::DepthStencilRead(const std::string& name)
    {
        RGTextureDepthStencilReadId id = _renderGraph._context.DepthStencilRead(name);
        _renderPass->_resourceStateMap[id] = dx12::ResourceState::DepthRead;
        _renderPass->_reads.push_back(id);
        return id;
    }

    RGTextureDepthStencilWriteId RenderPassBuilder::DepthStencilWrite(const std::string& name)
    {
        RGTextureDepthStencilWriteId id = _renderGraph._context.DepthStencilWrite(name);
        _renderPass->_resourceStateMap[id] = dx12::ResourceState::DepthWrite;
        _renderPass->_reads.push_back(id);
        _renderPass->_writes.push_back(id);
        return id;
    }

    RGVirtualResourceReadId RenderPassBuilder::ReadVirtualResource(const std::string& name)
    {
        RGVirtualResourceReadId id = _renderGraph._context.ReadVirtualResource(name);
        _renderPass->_reads.push_back(id);
        return id;
    }

    RGVirtualResourceWriteId RenderPassBuilder::WriteVirtualResource(const std::string& name)
    {
        RGVirtualResourceWriteId id = _renderGraph._context.WriteVirtualResource(name);
        _renderPass->_reads.push_back(id);
        _renderPass->_writes.push_back(id);
        return id;
    }
} // namespace rg
