#include "RenderGraphPCH.h"

#include "RenderPassBuilder.h"

namespace rg
{
    RenderPassBuilder::RenderPassBuilder(RenderGraph& renderGraph, IRenderPass* renderPass)
        : _renderGraph(renderGraph), _renderPass(renderPass)
    {
    }

    void RenderPassBuilder::DeclareBuffer(const std::string& name, const rhi::BufferDescription& desc, void* data, size_t dataSize)
    {
        RGBufferId id = _renderGraph._context.DeclareBuffer(name, desc, data, dataSize);

        _renderPass->_bufferCreates.push_back(id);
        _renderPass->_bufferReads.push_back(id);
        _renderPass->_bufferWrites.push_back(id);
    }
    
    void RenderPassBuilder::DeclareTexture(const std::string& name, const rhi::TextureDescription& desc, void* data, size_t dataSize)
    {
        RGTextureId id = _renderGraph._context.DeclareTexture(name, desc, data, dataSize);

        _renderPass->_textureCreates.push_back(id);
        _renderPass->_textureReads.push_back(id);
        _renderPass->_textureWrites.push_back(id);
    }

    RGBufferReadId RenderPassBuilder::ReadBuffer(const std::string& name)
    {
        RGBufferReadId id = _renderGraph._context.ReadBuffer(name);

        _renderPass->_bufferStateMap[id] = rhi::ResourceState::AllShaderResource;
        _renderPass->_bufferReads.push_back(id);

        return id;
    }

    RGBufferWriteId RenderPassBuilder::WriteBuffer(const std::string& name)
    {
        RGBufferWriteId id = _renderGraph._context.WriteBuffer(name);

        _renderPass->_bufferStateMap[id] = rhi::ResourceState::UnorderedAccess;
        _renderPass->_bufferReads.push_back(id);
        _renderPass->_bufferWrites.push_back(id);

        return id;
    }

    RGBufferUploadId RenderPassBuilder::UploadBuffer(const std::string& name)
    {
        RGBufferUploadId id = _renderGraph._context.UploadBuffer(name);

        _renderPass->_bufferStateMap[id] = rhi::ResourceState::GenericRead;
        _renderPass->_bufferReads.push_back(id);

        return id;
    }

    RGBufferCopySrcId RenderPassBuilder::CopySrcBuffer(const std::string& name)
    {
        RGBufferCopySrcId id = _renderGraph._context.CopySrcBuffer(name);

        _renderPass->_bufferStateMap[id] = rhi::ResourceState::CopySource;
        _renderPass->_bufferReads.push_back(id);

        return id;
    }

    RGBufferCopyDstId RenderPassBuilder::CopyDstBuffer(const std::string& name)
    {
        RGBufferCopyDstId id = _renderGraph._context.CopyDstBuffer(name);

        _renderPass->_bufferStateMap[id] = rhi::ResourceState::CopyDest;
        _renderPass->_bufferReads.push_back(id);
        _renderPass->_bufferWrites.push_back(id);

        return id;
    }

    RGBufferIndirectArgsId RenderPassBuilder::IndirectArgBuffer(const std::string& name)
    {
        RGBufferIndirectArgsId id = _renderGraph._context.IndirectArgBuffer(name);

        _renderPass->_bufferStateMap[id] = rhi::ResourceState::IndirectArgument;
        _renderPass->_bufferReads.push_back(id);

        return id;
    }

    RGTextureReadId RenderPassBuilder::ReadTexture(const std::string& name)
    {
        RGTextureReadId id = _renderGraph._context.ReadTexture(name);

        _renderPass->_textureStateMap[id] = rhi::ResourceState::AllShaderResource;
        _renderPass->_textureReads.push_back(id);

        return id;
    }

    RGTextureWriteId RenderPassBuilder::WriteTexture(const std::string& name)
    {
        RGTextureWriteId id = _renderGraph._context.WriteTexture(name);

        _renderPass->_textureStateMap[id] = rhi::ResourceState::UnorderedAccess;
        _renderPass->_textureReads.push_back(id);
        _renderPass->_textureWrites.push_back(id);

        return id;
    }

    RGTextureCopySrcId RenderPassBuilder::CopySrcTexture(const std::string& name)
    {
        RGTextureCopySrcId id = _renderGraph._context.CopySrcTexture(name);

        _renderPass->_textureStateMap[id] = rhi::ResourceState::CopySource;
        _renderPass->_textureReads.push_back(id);

        return id;
    }

    RGTextureCopyDstId RenderPassBuilder::CopyDstTexture(const std::string& name)
    {
        RGTextureCopyDstId id = _renderGraph._context.CopyDstTexture(name);

        _renderPass->_textureStateMap[id] = rhi::ResourceState::CopyDest;
        _renderPass->_textureReads.push_back(id);
        _renderPass->_textureWrites.push_back(id);

        return id;
    }

    RGTextureRenderTargetId RenderPassBuilder::RenderTarget(const std::string& name)
    {
        RGTextureRenderTargetId id = _renderGraph._context.RenderTarget(name);

        _renderPass->_textureStateMap[id] = rhi::ResourceState::RenderTarget;
        _renderPass->_textureReads.push_back(id);
        _renderPass->_textureWrites.push_back(id);

        return id;
    }

    RGTextureDepthStencilReadId RenderPassBuilder::DepthStencilRead(const std::string& name)
    {
        RGTextureDepthStencilReadId id = _renderGraph._context.DepthStencilRead(name);

        _renderPass->_textureStateMap[id] = rhi::ResourceState::DepthRead;
        _renderPass->_textureReads.push_back(id);

        return id;
    }

    RGTextureDepthStencilWriteId RenderPassBuilder::DepthStencilWrite(const std::string& name)
    {
        RGTextureDepthStencilWriteId id = _renderGraph._context.DepthStencilWrite(name);

        _renderPass->_textureStateMap[id] = rhi::ResourceState::DepthWrite;
        _renderPass->_textureReads.push_back(id);
        _renderPass->_textureWrites.push_back(id);

        return id;
    }

    RGVirtualResourceReadId RenderPassBuilder::ReadVirtualResource(const std::string& name)
    {
        RGVirtualResourceReadId id = _renderGraph._context.ReadVirtualResource(name);

        _renderPass->_virtualReads.push_back(id);

        return id;
    }

    RGVirtualResourceWriteId RenderPassBuilder::WriteVirtualResource(const std::string& name)
    {
        RGVirtualResourceWriteId id = _renderGraph._context.WriteVirtualResource(name);

        _renderPass->_virtualReads.push_back(id);
        _renderPass->_virtualWrites.push_back(id);

        return id;
    }
} // namespace rg
