#include "RenderGraphPCH.h"

#include "RenderContext.h"

#include <format>

namespace rg
{
    RenderContext::RenderContext(ResourceTable& resourceTable, TextureManager& textureManager)
        : _frame(nullptr)
        , _resourceTableNew(resourceTable)
		, _textureManager(textureManager)
    {
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
        return _resourceTableNew;
    }

    TextureManager& RenderContext::GetTextureManager()
    {
		return _textureManager;
    }

    void RenderContext::BindBindlessTable(dx12::CommandList& commandList) const
    {
        commandList.SetDescriptorHeaps({ _resourceTableNew.GetShaderResourcesDescriptorHeap().GetDXDescriptorHeap().Get() });
    }

    std::shared_ptr<dx12::Resource> RenderContext::GetResourceNew(ResourceId id)
    {
        auto resourceIt = _resourcesNew.find(id);
        if (resourceIt == _resourcesNew.end())
        {
            LOG_WARNING("Resource with id {} not found in render context.", id);
            return nullptr;
        }

        return resourceIt->second;
    }

    const DescriptorHandle& RenderContext::GetStaticResourceHandle(dx12::RenderTargetView rtv) const
    {
        return _resourceTableNew.GetStaticResourceHandle(rtv);
    }

    const DescriptorHandle& RenderContext::GetStaticResourceHandle(dx12::DepthStencilView dsv) const
    {
        return _resourceTableNew.GetStaticResourceHandle(dsv);
    }

    const DescriptorHandle& RenderContext::GetStaticResourceHandle(dx12::ShaderResourceView srv) const
    {
        return _resourceTableNew.GetStaticResourceHandle(srv);
    }

    const DescriptorHandle& RenderContext::GetStaticResourceHandle(dx12::UnorderedAccessView uav) const
    {
        return _resourceTableNew.GetStaticResourceHandle(uav);
    }

    const DescriptorHandle& RenderContext::GetStaticResourceHandle(dx12::ConstantBufferView cbv) const
    {
        return _resourceTableNew.GetStaticResourceHandle(cbv);
    }

    ResourceId RenderContext::CreateResourceVirtual(const std::string& name)
    {
        ASSERT(!name.empty(), "Resource name cannot be empty.");

        std::shared_ptr<dx12::Resource> resource = ResourceFactory::Create(name);
        _mapNameToIdNew[name] = resource->GetID();
        return resource->GetID();
    }

    ResourceId RenderContext::CreateResourceNew(const std::string& name, dx12::ResourceDescription desc, void* data /*= nullptr*/, size_t dataSize /*= 0*/)
    {
        ASSERT(!name.empty(), "Resource name cannot be empty.");

        std::shared_ptr<dx12::Resource> resource = ResourceFactory::Create(name, desc);
        resource->CreateCommitedResource();

        FillResource(resource, data, dataSize);

        _resourcesNew[resource->GetID()] = resource;
        {
            if (desc.GetFlags() & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
            {
                _resourceTableNew.AddStaticResourceView(resource->GetAsRTV());
            }
            if (desc.GetFlags() & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
            {
                _resourceTableNew.AddStaticResourceView(resource->GetAsDSV());
            }
            if (desc.GetFlags() & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
            {
                _resourceTableNew.AddStaticResourceView(resource->GetAsUAV());
            }
            _resourceTableNew.AddStaticResourceView(resource->GetAsSRV());
        }
        _mapNameToIdNew[name] = resource->GetID();

        return resource->GetID();
    }

    ResourceId RenderContext::ReadResourceNew(const std::string& name)
    {
        auto IdIt = _mapNameToIdNew.find(name);
        if (IdIt == _mapNameToIdNew.end())
        {
            LOG_CRITICAL("Texture is not exist in render graph context: {}", name);
            return ResourceId(-1);
        }

        return IdIt->second;
    }

    ResourceId RenderContext::WriteResourceNew(const std::string& name)
    {
        auto IdIt = _mapNameToIdNew.find(name);
        if (IdIt == _mapNameToIdNew.end())
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
