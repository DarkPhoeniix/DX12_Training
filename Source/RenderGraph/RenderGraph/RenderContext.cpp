#include "RenderGraphPCH.h"

#include "RenderContext.h"

#include <format>
#include "RenderPassBuilder.h"

namespace rg
{
    RenderContext::RenderContext()
        : _frame(nullptr)
        , _resourceTable(nullptr)
        , _textureManager(nullptr)
    {
    }

    void RenderContext::Init(ResourceTable& resourceTable, TextureManager& textureManager)
    {
        _resourceTable = &resourceTable;
        _textureManager = &textureManager;
    }

    const Frame* RenderContext::GetFrame() const
    {
        return _frame;
    }

    std::uint32_t RenderContext::GetFrameIndex() const
    {
        return _frame->Index;
    }

    TextureManager& RenderContext::GetTextureManager()
    {
        return *_textureManager;
    }

    void RenderContext::BindBindlessTable(dx12::CommandList& commandList) const
    {
        commandList.SetDescriptorHeaps({ _resourceTable->GetShaderResourcesDescriptorHeap().GetDXDescriptorHeap().Get() });
    }

    std::shared_ptr<dx12::Resource> RenderContext::GetResource(RGResourceId id)
    {
        auto resourceIt = _mapIdToResource.find(id);
        if (resourceIt == _mapIdToResource.end())
        {
            LOG_WARNING("Resource with id {} not found in render context.", id.ID);
            return nullptr;
        }

        return resourceIt->second;
    }

    DescriptorHandle RenderContext::GetStaticResourceHandle(const dx12::RenderTargetView& rtv) const
    {
        return _resourceTable->GetStaticResourceHandle(rtv);
    }

    DescriptorHandle RenderContext::GetStaticResourceHandle(const dx12::DepthStencilView& dsv) const
    {
        return _resourceTable->GetStaticResourceHandle(dsv);
    }

    DescriptorHandle RenderContext::GetStaticResourceHandle(const dx12::ShaderResourceView& srv) const
    {
        return _resourceTable->GetStaticResourceHandle(srv);
    }

    DescriptorHandle RenderContext::GetStaticResourceHandle(const dx12::UnorderedAccessView& uav) const
    {
        return _resourceTable->GetStaticResourceHandle(uav);
    }

    DescriptorHandle RenderContext::GetStaticResourceHandle(const dx12::ConstantBufferView& cbv) const
    {
        return _resourceTable->GetStaticResourceHandle(cbv);
    }

    DescriptorHandle RenderContext::GetTransientResourceHandle(const dx12::RenderTargetView& rtv) const
    {
        return _resourceTable->GetTransientResourceHandle(rtv);
    }

    DescriptorHandle RenderContext::GetTransientResourceHandle(const dx12::DepthStencilView& dsv) const
    {
        return _resourceTable->GetTransientResourceHandle(dsv);
    }

    DescriptorHandle RenderContext::GetTransientResourceHandle(const dx12::ShaderResourceView& srv) const
    {
        return _resourceTable->GetTransientResourceHandle(srv);
    }

    DescriptorHandle RenderContext::GetTransientResourceHandle(const dx12::UnorderedAccessView& uav) const
    {
        return _resourceTable->GetTransientResourceHandle(uav);
    }

    DescriptorHandle RenderContext::GetTransientResourceHandle(const dx12::ConstantBufferView& cbv) const
    {
        return _resourceTable->GetTransientResourceHandle(cbv);
    }

    RGResourceId RenderContext::CreateResourceVirtual(const std::string& name)
    {
        ASSERT(!name.empty(), "Resource name cannot be empty.");

        std::shared_ptr<dx12::Resource> resource = ResourceFactory::Create(name);
        _mapNameToId[name] = resource->GetID();
        return resource->GetID();
    }

    RGResourceId RenderContext::CreateResource(const std::string& name, dx12::ResourceDescription desc, void* data /*= nullptr*/, size_t dataSize /*= 0*/)
    {
        ASSERT(!name.empty(), "Resource name cannot be empty.");

        std::shared_ptr<dx12::Resource> resource = ResourceFactory::Create(name, desc);
        resource->CreateCommitedResource();

        FillBuffer(resource, data, dataSize);

        _mapIdToResource[resource->GetID()] = resource;
        {
            if (desc.GetFlags() & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
            {
                _resourceTable->AddStaticResourceView(resource->GetAsRTV());
            }
            if (desc.GetFlags() & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
            {
                _resourceTable->AddStaticResourceView(resource->GetAsDSV());
            }
            if (desc.GetFlags() & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
            {
                _resourceTable->AddStaticResourceView(resource->GetAsUAV());
            }
            _resourceTable->AddStaticResourceView(resource->GetAsSRV());
        }
        _mapNameToId[name] = resource->GetID();

        return resource->GetID();
    }

    RGResourceId RenderContext::ReadResource(const std::string& name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (IdIt == _mapNameToId.end())
        {
            LOG_CRITICAL("Texture is not exist in render graph context: {}", name);
            return RGResourceId(-1);
        }

        return IdIt->second;
    }

    RGResourceId RenderContext::WriteResource(const std::string& name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (IdIt == _mapNameToId.end())
        {
            LOG_CRITICAL("Texture is not exist in render graph context: {}", name);
            return RGResourceId(-1);
        }

        return IdIt->second;
    }

    RGResourceId RenderContext::DeclareBuffer(const std::string& name, const dx12::ResourceDescription& desc, void* data, size_t dataSize)
    {
        ASSERT(!name.empty(), "Buffer name cannot be empty.");

        std::shared_ptr<dx12::Resource> resource = ResourceFactory::Create(name, desc);
        resource->CreateCommitedResource();

        FillBuffer(resource, data, dataSize);

        _mapIdToResource[resource->GetID()] = resource;
        _mapNameToId[name] = resource->GetID();

        if (desc.GetFlags() & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
        {
            _resourceTable->AddStaticResourceView(resource->GetAsUAV());
        }
        _resourceTable->AddStaticResourceView(resource->GetAsSRV());

        return resource->GetID();
    }

    RGResourceId RenderContext::DeclareTexture(const std::string& name, const dx12::ResourceDescription& desc, void* data, size_t dataSize)
    {
        ASSERT(!name.empty(), "Texture name cannot be empty.");

        std::shared_ptr<dx12::Resource> resource = ResourceFactory::Create(name, desc);
        resource->CreateCommitedResource();

        FillTexture(resource, data, dataSize);

        _mapIdToResource[resource->GetID()] = resource;
        _mapNameToId[name] = resource->GetID();

        if (desc.GetFlags() & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
        {
            _resourceTable->AddStaticResourceView(resource->GetAsRTV());
        }
        if (desc.GetFlags() & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
        {
            _resourceTable->AddStaticResourceView(resource->GetAsDSV());
        }
        if (desc.GetFlags() & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
        {
            _resourceTable->AddStaticResourceView(resource->GetAsUAV());
        }
        _resourceTable->AddStaticResourceView(resource->GetAsSRV());

        return resource->GetID();
    }

    RGResourceId RenderContext::DeclareVirtualResource(const std::string& name)
    {
        ASSERT(!name.empty(), "Resource name cannot be empty.");

        std::shared_ptr<dx12::Resource> resource = ResourceFactory::Create(name);
        _mapNameToId[name] = resource->GetID();

        return resource->GetID();
    }

    RGBufferReadId RenderContext::ReadBuffer(const std::string& name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (IdIt == _mapNameToId.end())
        {
            LOG_CRITICAL("Buffer is not exist in render graph context: {}", name);
            return RGResourceId::InvalidID;
        }
        return RGBufferReadId(IdIt->second);
    }

    RGBufferWriteId RenderContext::WriteBuffer(const std::string& name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (IdIt == _mapNameToId.end())
        {
            LOG_CRITICAL("Buffer is not exist in render graph context: {}", name);
            return RGResourceId::InvalidID;
        }
        return RGBufferWriteId(IdIt->second);
    }

    RGBufferUploadId RenderContext::UploadBuffer(const std::string& name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (IdIt == _mapNameToId.end())
        {
            LOG_CRITICAL("Buffer is not exist in render graph context: {}", name);
            return RGResourceId::InvalidID;
        }
        return RGBufferUploadId(IdIt->second);
    }

    RGBufferCopySrcId RenderContext::CopySrcBuffer(const std::string& name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (IdIt == _mapNameToId.end())
        {
            LOG_CRITICAL("Buffer is not exist in render graph context: {}", name);
            return RGResourceId::InvalidID;
        }
        return RGBufferCopySrcId(IdIt->second);
    }
    
    RGBufferCopyDstId RenderContext::CopyDstBuffer(const std::string& name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (IdIt == _mapNameToId.end())
        {
            LOG_CRITICAL("Buffer is not exist in render graph context: {}", name);
            return RGResourceId::InvalidID;
        }
        return RGBufferCopyDstId(IdIt->second);
    }
    
    RGBufferIndirectArgsId RenderContext::IndirectArgBuffer(const std::string& name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (IdIt == _mapNameToId.end())
        {
            LOG_CRITICAL("Buffer is not exist in render graph context: {}", name);
            return RGResourceId::InvalidID;
        }
        return RGBufferIndirectArgsId(IdIt->second);
    }

    RGTextureReadId RenderContext::ReadTexture(const std::string& name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (IdIt == _mapNameToId.end())
        {
            LOG_CRITICAL("Texture is not exist in render graph context: {}", name);
            return RGResourceId::InvalidID;
        }
        return RGTextureReadId(IdIt->second);
    }
    
    RGTextureWriteId RenderContext::WriteTexture(const std::string& name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (IdIt == _mapNameToId.end())
        {
            LOG_CRITICAL("Texture is not exist in render graph context: {}", name);
            return RGResourceId::InvalidID;
        }
        return RGTextureWriteId(IdIt->second);
    }
    
    RGTextureCopySrcId RenderContext::CopySrcTexture(const std::string& name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (IdIt == _mapNameToId.end())
        {
            LOG_CRITICAL("Texture is not exist in render graph context: {}", name);
            return RGResourceId::InvalidID;
        }
        return RGTextureCopySrcId(IdIt->second);
    }
    
    RGTextureCopyDstId RenderContext::CopyDstTexture(const std::string& name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (IdIt == _mapNameToId.end())
        {
            LOG_CRITICAL("Texture is not exist in render graph context: {}", name);
            return RGResourceId::InvalidID;
        }
        return RGTextureCopyDstId(IdIt->second);
    }
    
    RGTextureRenderTargetId RenderContext::RenderTarget(const std::string& name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (IdIt == _mapNameToId.end())
        {
            LOG_CRITICAL("Texture is not exist in render graph context: {}", name);
            return RGResourceId::InvalidID;
        }
        return RGTextureRenderTargetId(IdIt->second);
    }
    
    RGTextureDepthStencilReadId RenderContext::DepthStencilRead(const std::string& name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (IdIt == _mapNameToId.end())
        {
            LOG_CRITICAL("Texture is not exist in render graph context: {}", name);
            return RGResourceId::InvalidID;
        }
        return RGTextureDepthStencilReadId(IdIt->second);
    }

    RGTextureDepthStencilWriteId RenderContext::DepthStencilWrite(const std::string& name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (IdIt == _mapNameToId.end())
        {
            LOG_CRITICAL("Texture is not exist in render graph context: {}", name);
            return RGResourceId::InvalidID;
        }
        return RGTextureDepthStencilWriteId(IdIt->second);
    }

    RGVirtualResourceReadId RenderContext::ReadVirtualResource(const std::string& name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (IdIt == _mapNameToId.end())
        {
            LOG_CRITICAL("Resource is not exist in render graph context: {}", name);
            return RGResourceId::InvalidID;
        }
        return RGVirtualResourceReadId(IdIt->second);
    }

    RGVirtualResourceWriteId RenderContext::WriteVirtualResource(const std::string& name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (IdIt == _mapNameToId.end())
        {
            LOG_CRITICAL("Resource is not exist in render graph context: {}", name);
            return RGResourceId::InvalidID;
        }
        return RGVirtualResourceWriteId(IdIt->second);
    }

    void RenderContext::FillBuffer(std::shared_ptr<dx12::Resource> resource, void* data, size_t dataSize)
    {
        if (!data)
        {
            return;
        }

        void* mappedData = resource->Map<void>();
        memcpy(mappedData, data, dataSize);

        resource->Unmap();
    }

    void RenderContext::FillTexture(std::shared_ptr<dx12::Resource> resource, void* data, size_t dataSize)
    {
        if (!data)
        {
            return;
        }

        // TODO: implement RenderContext::FillTexture
    }
} // namespace rg
