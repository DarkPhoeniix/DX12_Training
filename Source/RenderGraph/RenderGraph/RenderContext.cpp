#include "RenderGraphPCH.h"

#include "RenderContext.h"

#include <format>

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

    ResourceTable& RenderContext::GetResourceTable()
    {
        return *_resourceTable;
    }

    TextureManager& RenderContext::GetTextureManager()
    {
		return *_textureManager;
    }

    void RenderContext::BindBindlessTable(dx12::CommandList& commandList) const
    {
        commandList.SetDescriptorHeaps({ _resourceTable->GetShaderResourcesDescriptorHeap().GetDXDescriptorHeap().Get() });
    }

    std::shared_ptr<dx12::Resource> RenderContext::GetResource(ResourceId id)
    {
        auto resourceIt = _mapIdToResource.find(id);
        if (resourceIt == _mapIdToResource.end())
        {
            LOG_WARNING("Resource with id {} not found in render context.", id);
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

    ResourceId RenderContext::CreateResourceVirtual(const std::string& name)
    {
        ASSERT(!name.empty(), "Resource name cannot be empty.");

        std::shared_ptr<dx12::Resource> resource = ResourceFactory::Create(name);
        _mapNameToId[name] = resource->GetID();
        return resource->GetID();
    }

    ResourceId RenderContext::CreateResource(const std::string& name, dx12::ResourceDescription desc, void* data /*= nullptr*/, size_t dataSize /*= 0*/)
    {
        ASSERT(!name.empty(), "Resource name cannot be empty.");

        std::shared_ptr<dx12::Resource> resource = ResourceFactory::Create(name, desc);
        resource->CreateCommitedResource();

        FillResource(resource, data, dataSize);

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

    ResourceId RenderContext::ReadResource(const std::string& name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (IdIt == _mapNameToId.end())
        {
            LOG_CRITICAL("Texture is not exist in render graph context: {}", name);
            return ResourceId(-1);
        }

        return IdIt->second;
    }

    ResourceId RenderContext::WriteResource(const std::string& name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (IdIt == _mapNameToId.end())
        {
            LOG_CRITICAL("Texture is not exist in render graph context: {}", name);
            return ResourceId(-1);
        }

        return IdIt->second;
    }

    void RenderContext::FillResource(std::shared_ptr<dx12::Resource> resource, void* data, size_t dataSize)
    {
        if (!data)
        {
            return;
        }

		void* mappedData = resource->Map<void>();
		memcpy(mappedData, data, dataSize);

        resource->Unmap();
    }
} // namespace rg
