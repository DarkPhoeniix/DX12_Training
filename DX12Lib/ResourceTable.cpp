#include "DX12LibPCH.h"

#include "ResourceTable.h"

namespace dx12
{
    auto ResourceTable::GetResource(const ResourceKey& key)
    {
        std::shared_lock<std::shared_mutex> readLock(_mutex);

        ResourceTable::ResourceMap& resources = _GetResourceMap(key.ViewType);

        return resources.find(key);
    }

    void ResourceTable::Init(std::uint32_t numDescriptors, bool shaderVisible)
    {
        _numDescriptors = numDescriptors;

        dx12::DescriptorHeapDescription descriptorHeapDesc;
        descriptorHeapDesc.SetNumDescriptors(numDescriptors);
        descriptorHeapDesc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

        descriptorHeapDesc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        _RTVDescriptorHeap.Create(descriptorHeapDesc);
        _RTVDescriptorHeap.SetName("RTV Descriptor heap of resource table");

        descriptorHeapDesc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        _DSVDescriptorHeap.Create(descriptorHeapDesc);
        _DSVDescriptorHeap.SetName("DSV Descriptor heap of resource table");

        descriptorHeapDesc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        if (shaderVisible)
        {
            descriptorHeapDesc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
        }
        _BuffersDescriptorHeap.Create(descriptorHeapDesc);
        _BuffersDescriptorHeap.SetName("Buffers Descriptor heap of resource table");
    }

    void ResourceTable::Reset()
    {
        _RTVResources.clear();
        _DSVResources.clear();
        _BufferResources.clear();

        _RTVDescriptorHeap.Reset();
        _DSVDescriptorHeap.Reset();
        _BuffersDescriptorHeap.Reset();
    }

    bool ResourceTable::CopyDescriptor(Resource* resource, ResourceViewType viewType, ResourceTable& srcTable)
    {
        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);
        DescriptorHeap& descriptorHeap = GetDescriptorHeap(viewType);

        ResourceKey key = { resource->GetName().c_str(), viewType};
        auto it = GetResource(key);
        if (it != resources.end())
        {
            return false;
        }

        {
            std::unique_lock<std::shared_mutex> writeLock(_mutex);

            InternalResourceDesc value = { resource, descriptorHeap.GetCurrentOffset(), viewType };
            resources.insert(std::make_pair(key, value));

            D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = srcTable.GetResourceCPUHandle(resource, viewType);
            descriptorHeap.CopyResourceDescriptor(srcHandle);
        }

        return true;
    }

    bool ResourceTable::CopyDescriptor(Resource* resource, ResourceViewType viewType, D3D12_CPU_DESCRIPTOR_HANDLE handle)
    {
        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);
        DescriptorHeap& descriptorHeap = GetDescriptorHeap(viewType);

        {
            std::unique_lock<std::shared_mutex> writeLock(_mutex);

            ResourceKey key = { resource->GetName().c_str(), viewType };
            InternalResourceDesc value = { resource, descriptorHeap.GetCurrentOffset(), viewType };
            resources.insert(std::make_pair(key, value));

            descriptorHeap.CopyResourceDescriptor(handle);
        }

        return true;
    }

    bool ResourceTable::PlaceResource(Resource* resource, ResourceViewType viewType)
    {
        if (ASSERT(resource, "Trying to add a nullptr resource to resource table"))
        {
            return false;
        }

        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);
        DescriptorHeap& descriptorHeap = GetDescriptorHeap(viewType);

        ASSERT((resources.size() < _numDescriptors), "Resource table is full");

        ResourceKey key = { resource->GetName().c_str(), viewType};
        InternalResourceDesc value = { resource, descriptorHeap.GetCurrentOffset(), viewType};

        {
            std::unique_lock<std::shared_mutex> writeLock(_mutex);

            resources.insert_or_assign(key, value);
            switch (viewType)
            {
            case ResourceViewType::RTV:
                dx12::Device::CreateRenderTargetView(resource->GetAsRTV(), descriptorHeap);
                break;
            case ResourceViewType::DSV:
                dx12::Device::CreateDepthStencilView(resource->GetAsDSV(), descriptorHeap);
                break;
            case ResourceViewType::CBV:
                dx12::Device::CreateConstantBufferView(resource->GetAsCBV(), descriptorHeap);
                break;
            case ResourceViewType::SRV:
                dx12::Device::CreateShaderResourceView(resource->GetAsSRV(), descriptorHeap);
                break;
            case ResourceViewType::UAV:
                dx12::Device::CreateUnorderedAccessView(resource->GetAsUAV(), descriptorHeap,
                    (resource->GetResourceDescription().GetUAVCounterOffset() != -1) ? resource : nullptr);
                break;
            default:
                ASSERT(false, "TODO");
                break;
            };
        }

        return true;
    }

    bool ResourceTable::PlaceResourceIfNotExist(Resource* resource, ResourceViewType viewType)
    {
        if (ASSERT(resource, "Trying to add a nullptr resource to resource table"))
        {
            return false;
        }

        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);
        DescriptorHeap& descriptorHeap = GetDescriptorHeap(viewType);
        
        ResourceKey key = { resource->GetName().c_str(), viewType };
        auto it = GetResource(key);

        if (it == resources.end())
        {
            return PlaceResource(resource, viewType);
        }

        return false;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE ResourceTable::GetResourceCPUHandle(Resource* resource, ResourceViewType viewType)
    {
        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);
        DescriptorHeap& descriptorHeap = GetDescriptorHeap(viewType);

        ResourceKey key = { resource->GetName().c_str(), viewType };
        auto it = GetResource(key);
        if (it == resources.end())
        {
            FAIL("Failed to get resource handle");
        }

        return descriptorHeap.GetCPUHandleWithOffset(it->second.HeapIndex);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE ResourceTable::GetResourceGPUHandle(Resource* resource, ResourceViewType viewType)
    {
        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);
        DescriptorHeap& descriptorHeap = GetDescriptorHeap(viewType);

        ResourceKey key = { resource->GetName().c_str(), viewType };
        auto it = GetResource(key);
        if (it == resources.end())
        {
            FAIL("Failed to get resource handle");
        }

        return descriptorHeap.GetGPUHandleWithOffset(it->second.HeapIndex);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE ResourceTable::GetResourceCPUHandle(const std::string& resourceName, ResourceViewType viewType)
    {
        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);
        DescriptorHeap& descriptorHeap = GetDescriptorHeap(viewType);

        ResourceKey key = { resourceName.c_str(), viewType };
        auto it = GetResource(key);
        if (it == resources.end())
        {
            FAIL("Failed to get resource handle");
        }

        return descriptorHeap.GetCPUHandleWithOffset(it->second.HeapIndex);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE ResourceTable::GetResourceGPUHandle(const std::string& resourceName, ResourceViewType viewType)
    {
        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);
        DescriptorHeap& descriptorHeap = GetDescriptorHeap(viewType);

        ResourceKey key = { resourceName.c_str(), viewType };
        auto it = GetResource(key);
        if (it == resources.end())
        {
            FAIL("Failed to get resource handle");
        }

        return descriptorHeap.GetGPUHandleWithOffset(it->second.HeapIndex);
    }

    std::uint32_t ResourceTable::GetResourceIndex(Resource* resource, ResourceViewType viewType)
    {
        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);

        ResourceKey key = { resource->GetName().c_str(), viewType};
        auto it = GetResource(key);
        if (it == resources.end())
        {
            FAIL("Failed to get resource handle");
            return -1;
        }

        return it->second.HeapIndex;
    }

    std::uint32_t ResourceTable::GetResourceIndex(const std::string& resourceName, ResourceViewType viewType)
    {
        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);

        ResourceKey key = { resourceName.c_str(), viewType };
        auto it = GetResource(key);
        if (it == resources.end())
        {
            FAIL("Failed to get resource handle");
            return -1;
        }

        return it->second.HeapIndex;
    }

    Resource* ResourceTable::GetResourceByName(const std::string& resourceName, ResourceViewType viewType)
    {
        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);

        ResourceKey key = { resourceName.c_str(), viewType };
        auto it = GetResource(key);
        if (it == resources.end())
        {
            FAIL("Failed to get resource handle");
        }

        return it->second.PlacedResource;
    }

    DescriptorHeap& ResourceTable::GetDescriptorHeap(ResourceViewType viewType)
    {
        switch (viewType)
        {
        case ResourceViewType::RTV:
            return _RTVDescriptorHeap;
        case ResourceViewType::DSV:
            return _DSVDescriptorHeap;
        default: // CBV / SRV / UAV
            return _BuffersDescriptorHeap;
        }
    }

    const DescriptorHeap& ResourceTable::GetDescriptorHeap(ResourceViewType viewType) const
    {
        switch (viewType)
        {
        case ResourceViewType::RTV:
            return _RTVDescriptorHeap;
        case ResourceViewType::DSV:
            return _DSVDescriptorHeap;
        default: // CBV / SRV / UAV
            return _BuffersDescriptorHeap;
        }
    }

    ResourceTable::ResourceMap& ResourceTable::_GetResourceMap(ResourceViewType viewType)
    {
        switch (viewType)
        {
        case ResourceViewType::RTV:
            return _RTVResources;
        case ResourceViewType::DSV:
            return _DSVResources;
        default: // CBV / SRV / UAV
            return _BufferResources;
        }
    }
} // namespace dx12
