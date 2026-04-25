#include "RenderGraphPCH.h"

#include "RenderContext.h"

#include "RenderPassBuilder.h"

#include <format>

namespace rg
{
    RenderContext::RenderContext(rhi::Device* device, IDescriptorProvider* descriptorProvider)
        : _descriptorProvider(descriptorProvider)
        , _device(device)
        , _gpuProfiler(nullptr)
    {
    }

    void RenderContext::SetFrameBuffer(rhi::Buffer* buffer)
    {
        _frameBuffer = buffer;
    }

    std::shared_ptr<rhi::Buffer> RenderContext::GetBuffer(RGBufferId id) const
    {
        auto resourceIt = _mapIdToBuffer.find(id);
        if (resourceIt == _mapIdToBuffer.end())
        {
            LOG_WARNING("Resource with id {} not found in render context.", id.ID);
            return nullptr;
        }

        return resourceIt->second;
    }

    std::shared_ptr<rhi::Texture> RenderContext::GetTexture(RGTextureId id) const
    {
        auto resourceIt = _mapIdToTexture.find(id);
        if (resourceIt == _mapIdToTexture.end())
        {
            LOG_WARNING("Resource with id {} not found in render context.", id.ID);
            return nullptr;
        }

        return resourceIt->second;
    }

    std::uint32_t RenderContext::GetBindlessIndex(RGBufferId id, rhi::ResourceViewType viewType) const
    {
        return _descriptorProvider->GetBindlessIndex(rhi::ResourceID(id.ID), viewType);
    }

    std::uint32_t RenderContext::GetBindlessIndex(RGTextureId id, rhi::ResourceViewType viewType) const
    {
        return _descriptorProvider->GetBindlessIndex(rhi::ResourceID(id.ID), viewType);
    }

    rhi::CPUDescriptor RenderContext::GetDescriptor(RGBufferId id, rhi::ResourceViewType viewType) const
    {
        return _descriptorProvider->GetDescriptor(rhi::ResourceID(id.ID), viewType);
    }

    rhi::CPUDescriptor RenderContext::GetDescriptor(RGTextureId id, rhi::ResourceViewType viewType) const
    {
        return _descriptorProvider->GetDescriptor(rhi::ResourceID(id.ID), viewType);
    }

    rhi::Buffer* RenderContext::GetFrameBuffer() const
    {
        return _frameBuffer;
    }

    void RenderContext::SetProfiler(Profiler* gpuProfiler)
    {
        _gpuProfiler = gpuProfiler;
    }

    Profiler* RenderContext::GetProfiler() const
    {
        return _gpuProfiler;
    }

    RGBufferId RenderContext::DeclareBuffer(const std::string& name, const rhi::BufferDescription& desc, void* data, size_t dataSize)
    {
        ASSERT(!name.empty(), "Buffer name cannot be empty.");

        std::shared_ptr<rhi::Buffer> buffer = _device->CreateBuffer(desc, rhi::ResourceState::Common, name);

        FillBuffer(buffer, data, dataSize);

        _mapIdToBuffer[buffer->GetID()] = buffer;
        _mapNameToBufferId[name] = buffer->GetID();

        if (HasFlag(desc.Flags, rhi::ResourceFlags::AllowUnorderedAccess))
        {
            _descriptorProvider->CreateStaticResourceView(buffer, rhi::ResourceViewType::UAV);
        }
        _descriptorProvider->CreateStaticResourceView(buffer, rhi::ResourceViewType::SRV);

        return buffer->GetID();
    }

    RGTextureId RenderContext::DeclareTexture(const std::string& name, const rhi::TextureDescription& desc, void* data, size_t dataSize)
    {
        ASSERT(!name.empty(), "Texture name cannot be empty.");

        std::shared_ptr<rhi::Texture> texture = _device->CreateTexture(desc, rhi::ResourceState::Common, name);

        FillTexture(texture, data, dataSize);

        _mapIdToTexture[texture->GetID()] = texture;
        _mapNameToTextureId[name] = texture->GetID();

        if (HasFlag(desc.Flags, rhi::ResourceFlags::AllowRenderTarget))
        {
            _descriptorProvider->CreateStaticResourceView(texture, rhi::ResourceViewType::RTV);
        }
        if (HasFlag(desc.Flags, rhi::ResourceFlags::AllowDepthStencil))
        {
            _descriptorProvider->CreateStaticResourceView(texture, rhi::ResourceViewType::DSV);
        }
        if (HasFlag(desc.Flags, rhi::ResourceFlags::AllowUnorderedAccess))
        {
            _descriptorProvider->CreateStaticResourceView(texture, rhi::ResourceViewType::UAV);
        }
        _descriptorProvider->CreateStaticResourceView(texture, rhi::ResourceViewType::SRV);

        return texture->GetID();
    }

    RGBufferReadId RenderContext::ReadBuffer(const std::string& name)
    {
        auto it = _mapNameToBufferId.find(name);
        if (it == _mapNameToBufferId.end())
        {
            LOG_CRITICAL("Buffer is not exist in render graph context: {}", name);
            return RGBufferId::InvalidID;
        }
        return RGBufferReadId(it->second);
    }

    RGBufferWriteId RenderContext::WriteBuffer(const std::string& name)
    {
        auto it = _mapNameToBufferId.find(name);
        if (it == _mapNameToBufferId.end())
        {
            LOG_CRITICAL("Buffer is not exist in render graph context: {}", name);
            return RGBufferId::InvalidID;
        }
        return RGBufferWriteId(it->second);
    }

    RGBufferUploadId RenderContext::UploadBuffer(const std::string& name)
    {
        auto it = _mapNameToBufferId.find(name);
        if (it == _mapNameToBufferId.end())
        {
            LOG_CRITICAL("Buffer is not exist in render graph context: {}", name);
            return RGBufferId::InvalidID;
        }
        return RGBufferUploadId(it->second);
    }

    RGBufferCopySrcId RenderContext::CopySrcBuffer(const std::string& name)
    {
        auto it = _mapNameToBufferId.find(name);
        if (it == _mapNameToBufferId.end())
        {
            LOG_CRITICAL("Buffer is not exist in render graph context: {}", name);
            return RGBufferId::InvalidID;
        }
        return RGBufferCopySrcId(it->second);
    }
    
    RGBufferCopyDstId RenderContext::CopyDstBuffer(const std::string& name)
    {
        auto it = _mapNameToBufferId.find(name);
        if (it == _mapNameToBufferId.end())
        {
            LOG_CRITICAL("Buffer is not exist in render graph context: {}", name);
            return RGBufferId::InvalidID;
        }
        return RGBufferCopyDstId(it->second);
    }
    
    RGBufferIndirectArgsId RenderContext::IndirectArgBuffer(const std::string& name)
    {
        auto it = _mapNameToBufferId.find(name);
        if (it == _mapNameToBufferId.end())
        {
            LOG_CRITICAL("Buffer is not exist in render graph context: {}", name);
            return RGBufferId::InvalidID;
        }
        return RGBufferIndirectArgsId(it->second);
    }

    RGTextureReadId RenderContext::ReadTexture(const std::string& name)
    {
        auto it = _mapNameToTextureId.find(name);
        if (it == _mapNameToTextureId.end())
        {
            LOG_CRITICAL("Texture is not exist in render graph context: {}", name);
            return RGTextureId::InvalidID;
        }

        RGTextureId id = it->second;
        if (_descriptorProvider->GetBindlessIndex(id.ID, rhi::ResourceViewType::SRV) == std::uint32_t(-1))
        {
            _descriptorProvider->CreateStaticResourceView(_mapIdToTexture[id.ID], rhi::ResourceViewType::SRV);
        }

        return RGTextureReadId(id);
    }
    
    RGTextureWriteId RenderContext::WriteTexture(const std::string& name)
    {
        auto it = _mapNameToTextureId.find(name);
        if (it == _mapNameToTextureId.end())
        {
            LOG_CRITICAL("Texture is not exist in render graph context: {}", name);
            return RGTextureId::InvalidID;
        }
        return RGTextureWriteId(it->second);
    }
    
    RGTextureCopySrcId RenderContext::CopySrcTexture(const std::string& name)
    {
        auto it = _mapNameToTextureId.find(name);
        if (it == _mapNameToTextureId.end())
        {
            LOG_CRITICAL("Texture is not exist in render graph context: {}", name);
            return RGTextureId::InvalidID;
        }
        return RGTextureCopySrcId(it->second);
    }
    
    RGTextureCopyDstId RenderContext::CopyDstTexture(const std::string& name)
    {
        auto it = _mapNameToTextureId.find(name);
        if (it == _mapNameToTextureId.end())
        {
            LOG_CRITICAL("Texture is not exist in render graph context: {}", name);
            return RGTextureId::InvalidID;
        }
        return RGTextureCopyDstId(it->second);
    }
    
    RGTextureRenderTargetId RenderContext::RenderTarget(const std::string& name)
    {
        auto it = _mapNameToTextureId.find(name);
        if (it == _mapNameToTextureId.end())
        {
            LOG_CRITICAL("Texture is not exist in render graph context: {}", name);
            return RGTextureId::InvalidID;
        }
        return RGTextureRenderTargetId(it->second);
    }
    
    RGTextureDepthStencilReadId RenderContext::DepthStencilRead(const std::string& name)
    {
        auto it = _mapNameToTextureId.find(name);
        if (it == _mapNameToTextureId.end())
        {
            LOG_CRITICAL("Texture is not exist in render graph context: {}", name);
            return RGTextureId::InvalidID;
        }
        return RGTextureDepthStencilReadId(it->second);
    }

    RGTextureDepthStencilWriteId RenderContext::DepthStencilWrite(const std::string& name)
    {
        auto it = _mapNameToTextureId.find(name);
        if (it == _mapNameToTextureId.end())
        {
            LOG_CRITICAL("Texture is not exist in render graph context: {}", name);
            return RGTextureId::InvalidID;
        }
        return RGTextureDepthStencilWriteId(it->second);
    }

    RGVirtualResourceReadId RenderContext::ReadVirtualResource(const std::string& name)
    {
        auto it = _mapNameToBufferId.find(name);
        if (it == _mapNameToBufferId.end())
        {
            LOG_CRITICAL("Resource is not exist in render graph context: {}", name);
            return RGBufferId::InvalidID;
        }
        return RGVirtualResourceReadId(it->second.ID);
    }

    RGVirtualResourceWriteId RenderContext::WriteVirtualResource(const std::string& name)
    {
        auto it = _mapNameToBufferId.find(name);
        if (it == _mapNameToBufferId.end())
        {
            LOG_CRITICAL("Resource is not exist in render graph context: {}", name);
            return RGBufferId::InvalidID;
        }
        return RGVirtualResourceWriteId(it->second.ID);
    }

    void RenderContext::FillBuffer(std::shared_ptr<rhi::Buffer> buffer, void* data, size_t dataSize)
    {
        if (!data)
        {
            return;
        }

        void* mappedData = buffer->Map<void>();
        memcpy(mappedData, data, dataSize);

        buffer->Unmap();
    }

    void RenderContext::FillTexture(std::shared_ptr<rhi::Texture> texture, void* data, size_t dataSize)
    {
        if (!data)
        {
            return;
        }

        NOT_IMPLEMENTED();
        // TODO: implement RenderContext::FillTexture
    }
} // namespace rg
