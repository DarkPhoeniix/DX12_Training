#include "RenderGraphPCH.h"

#include "RenderContext.h"

#include "ResourceTable.h"

#include <format>

namespace
{
    constexpr std::uint32_t DESCRIPTOR_TABLE_SIZE = 4096u;
}

namespace rg
{
    RenderContext::RenderContext(ResourceTable& resourceTable, TextureManager& textureManager)
        : _frame(nullptr)
        , _resourceTableNew(resourceTable)
		, _textureManager(textureManager)
    {
        for (dx12::ResourceTable& table : _resourceTable)
        {
            table.Init(DESCRIPTOR_TABLE_SIZE, true);
        }

        // Initialize cache heap
        for (size_t i = 0 ; i < dx12::BACK_BUFFER_COUNT; ++i)
        {
            CacheGPU& cache = _cache[i];

            dx12::ResourceDescription desc = {};
            desc.SetSize({ _16MB, 1 });
            desc.SetStride(256);
            desc.SetFormat(DXGI_FORMAT_UNKNOWN);
            desc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);

            std::shared_ptr<dx12::Resource> memoryBlock = ResourceFactory::Create(std::format("Frame cache {}", i), desc);
            memoryBlock->CreateCommitedResource();

            cache.SetResource(memoryBlock);
        }
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

    std::shared_ptr<dx12::Resource> RenderContext::GetResource(ResourceId id)
    {
        auto resourceIt = _resources.find(id);
        if (resourceIt == _resources.end())
        {
            LOG_WARNING("Resource with id {} not found in render context.", id);
            return nullptr;
        }

        return resourceIt->second;
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

    D3D12_CPU_DESCRIPTOR_HANDLE RenderContext::GetCPUHandle(dx12::RenderTargetView rtv)
    {
        dx12::ResourceTable& table = _resourceTable[_frame->Index];

        table.PlaceResourceIfNotExist(rtv.Owner, dx12::ResourceViewType::RTV);

        return table.GetResourceCPUHandle(rtv.Owner, dx12::ResourceViewType::RTV);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE RenderContext::GetCPUHandle(dx12::DepthStencilView dsv)
    {
        dx12::ResourceTable& table = _resourceTable[_frame->Index];

        table.PlaceResourceIfNotExist(dsv.Owner, dx12::ResourceViewType::DSV);

        return table.GetResourceCPUHandle(dsv.Owner, dx12::ResourceViewType::DSV);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE RenderContext::GetCPUHandle(dx12::ShaderResourceView srv)
    {
        dx12::ResourceTable& table = _resourceTable[_frame->Index];

        table.PlaceResourceIfNotExist(srv.Owner, dx12::ResourceViewType::SRV);

        return table.GetResourceCPUHandle(srv.Owner, dx12::ResourceViewType::SRV);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE RenderContext::GetCPUHandle(dx12::UnorderedAccessView uav)
    {
        dx12::ResourceTable& table = _resourceTable[_frame->Index];

        table.PlaceResourceIfNotExist(uav.Owner, dx12::ResourceViewType::UAV);

        return table.GetResourceCPUHandle(uav.Owner, dx12::ResourceViewType::UAV);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE RenderContext::GetCPUHandle(dx12::ConstantBufferView cbv)
    {
        dx12::ResourceTable& table = _resourceTable[_frame->Index];

        table.PlaceResourceIfNotExist(cbv.Owner, dx12::ResourceViewType::CBV);

        return table.GetResourceCPUHandle(cbv.Owner, dx12::ResourceViewType::CBV);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE RenderContext::GetGPUHandle(dx12::RenderTargetView rtv)
    {
        dx12::ResourceTable& table = _resourceTable[_frame->Index];

        table.PlaceResourceIfNotExist(rtv.Owner, dx12::ResourceViewType::RTV);

        return table.GetResourceGPUHandle(rtv.Owner, dx12::ResourceViewType::RTV);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE RenderContext::GetGPUHandle(dx12::DepthStencilView dsv)
    {
        dx12::ResourceTable& table = _resourceTable[_frame->Index];

        table.PlaceResourceIfNotExist(dsv.Owner, dx12::ResourceViewType::DSV);

        return table.GetResourceGPUHandle(dsv.Owner, dx12::ResourceViewType::DSV);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE RenderContext::GetGPUHandle(dx12::ShaderResourceView srv)
    {
        dx12::ResourceTable& table = _resourceTable[_frame->Index];

        table.PlaceResourceIfNotExist(srv.Owner, dx12::ResourceViewType::SRV);

        return table.GetResourceGPUHandle(srv.Owner, dx12::ResourceViewType::SRV);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE RenderContext::GetGPUHandle(dx12::UnorderedAccessView uav)
    {
        dx12::ResourceTable& table = _resourceTable[_frame->Index];

        table.PlaceResourceIfNotExist(uav.Owner, dx12::ResourceViewType::UAV);

        return table.GetResourceGPUHandle(uav.Owner, dx12::ResourceViewType::UAV);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE RenderContext::GetGPUHandle(dx12::ConstantBufferView cbv)
    {
        dx12::ResourceTable& table = _resourceTable[_frame->Index];

        table.PlaceResourceIfNotExist(cbv.Owner, dx12::ResourceViewType::CBV);

        return table.GetResourceGPUHandle(cbv.Owner, dx12::ResourceViewType::CBV);
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

    ResourceId RenderContext::CreateResource(const std::string& name, dx12::ResourceDescription desc)
    {
        ASSERT(!name.empty(), "Resource name cannot be empty.");

        std::shared_ptr<dx12::Resource> resource = ResourceFactory::Create(name, desc);
        resource->CreateCommitedResource();

        ResourceId id = _mapNameToId.size();
        _mapNameToId[name] = id;
        _resources[id] = resource;

        for (dx12::ResourceTable& table : _resourceTable)
        {
            D3D12_RESOURCE_FLAGS flags = desc.GetFlags();
            if (flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
            {
                table.PlaceResource(resource, dx12::ResourceViewType::RTV);
            }
            if (flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
            {
                table.PlaceResource(resource, dx12::ResourceViewType::DSV);
            }
            if (flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
            {
                table.PlaceResource(resource, dx12::ResourceViewType::UAV);
            }
            table.PlaceResource(resource, dx12::ResourceViewType::SRV);
        }

        return id;
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
