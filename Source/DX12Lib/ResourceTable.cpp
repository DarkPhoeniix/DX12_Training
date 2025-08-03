#include "ResourceTable.h"
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

    ResourceTable::ResourceTable(const ResourceTable& other)
        : _RTVResources(other._RTVResources)
        , _DSVResources(other._DSVResources)
        , _BufferResources(other._BufferResources)
        , _RTVDescriptorHeap(other._RTVDescriptorHeap)
        , _DSVDescriptorHeap(other._DSVDescriptorHeap)
        , _BuffersDescriptorHeap(other._BuffersDescriptorHeap)
        , _numDescriptors(other._numDescriptors)
    {
    }

    ResourceTable::ResourceTable(ResourceTable&& other) noexcept
        : _RTVResources(std::move(other._RTVResources))
        , _DSVResources(std::move(other._DSVResources))
        , _BufferResources(std::move(other._BufferResources))
        , _RTVDescriptorHeap(std::move(other._RTVDescriptorHeap))
        , _DSVDescriptorHeap(std::move(other._DSVDescriptorHeap))
        , _BuffersDescriptorHeap(std::move(other._BuffersDescriptorHeap))
        , _numDescriptors(other._numDescriptors)
    {
    }

    ResourceTable::~ResourceTable()
    {
    }

    ResourceTable& ResourceTable::operator=(const ResourceTable& other)
    {
        if (this != &other)
        {
            _RTVResources = other._RTVResources;
            _DSVResources = other._DSVResources;
            _BufferResources = other._BufferResources;

            _RTVDescriptorHeap = other._RTVDescriptorHeap;
            _DSVDescriptorHeap = other._DSVDescriptorHeap;
            _BuffersDescriptorHeap = other._BuffersDescriptorHeap;

            _numDescriptors = other._numDescriptors;
        }

        return *this;
    }

    ResourceTable& ResourceTable::operator=(ResourceTable&& other) noexcept
    {
        if (this != &other)
        {
            _RTVResources = std::move(other._RTVResources);
            _DSVResources = std::move(other._DSVResources);
            _BufferResources = std::move(other._BufferResources);

            _RTVDescriptorHeap = std::move(other._RTVDescriptorHeap);
            _DSVDescriptorHeap = std::move(other._DSVDescriptorHeap);
            _BuffersDescriptorHeap = std::move(other._BuffersDescriptorHeap);

            _numDescriptors = other._numDescriptors;
        }

        return *this;
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

    std::uint32_t ResourceTable::CopyDescriptor(std::shared_ptr<Resource> resource, ResourceViewType viewType, ResourceTable& srcTable)
    {
        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);
        DescriptorHeap& descriptorHeap = GetDescriptorHeap(viewType);

        ResourceKey key = { resource->GetName().c_str(), viewType};
        auto it = GetResource(key);
        if (it != resources.end())
        {
            return resources[key].HeapIndex;
        }

        {
            std::unique_lock<std::shared_mutex> writeLock(_mutex);

            InternalResourceDesc value = { resource, descriptorHeap.GetCurrentOffset(), viewType };
            resources.insert(std::make_pair(key, value));

            D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = srcTable.GetResourceCPUHandle(resource, viewType);
            descriptorHeap.CopyResourceDescriptor(srcHandle);

            return value.HeapIndex;
        }
    }

    std::uint32_t ResourceTable::CopyDescriptor(std::shared_ptr<Resource> resource, ResourceViewType viewType, D3D12_CPU_DESCRIPTOR_HANDLE handle)
    {
        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);
        DescriptorHeap& descriptorHeap = GetDescriptorHeap(viewType);

        ResourceKey key = { resource->GetName().c_str(), viewType };
        auto it = GetResource(key);
        if (it != resources.end())
        {
            return resources[key].HeapIndex;
        }

        {
            std::unique_lock<std::shared_mutex> writeLock(_mutex);

            InternalResourceDesc value = { resource, descriptorHeap.GetCurrentOffset(), viewType };
            resources.insert(std::make_pair(key, value));

            descriptorHeap.CopyResourceDescriptor(handle);

            return value.HeapIndex;
        }
    }

    bool ResourceTable::PlaceResource(std::shared_ptr<Resource> resource, ResourceViewType viewType)
    {
        ASSERT(resource, "Trying to add a nullptr resource to resource table");

        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);
        DescriptorHeap& descriptorHeap = GetDescriptorHeap(viewType);

        ASSERT((resources.size() < _numDescriptors), "Resource table is full");
        ASSERT((descriptorHeap.GetCurrentOffset() < _numDescriptors), "Resource table is full");

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
                LOG_ERROR("Failed to create resource view");
                break;
            };
        }

        return true;
    }

    bool ResourceTable::PlaceResourceIfNotExist(std::shared_ptr<Resource> resource, ResourceViewType viewType)
    {
        ASSERT(resource, "Trying to add a nullptr resource to resource table");

        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);
        
        ResourceKey key = { resource->GetName().c_str(), viewType };
        auto it = GetResource(key);

        if (it == resources.end())
        {
            return PlaceResource(resource, viewType);
        }

        return false;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE ResourceTable::GetResourceCPUHandle(std::shared_ptr<Resource> resource, ResourceViewType viewType)
    {
        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);
        DescriptorHeap& descriptorHeap = GetDescriptorHeap(viewType);

        ResourceKey key = { resource->GetName().c_str(), viewType };
        auto it = GetResource(key);
        FAIL(it != resources.end(), "Failed to get resource handle");

        return descriptorHeap.GetCPUHandleWithOffset(it->second.HeapIndex);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE ResourceTable::GetResourceGPUHandle(std::shared_ptr<Resource> resource, ResourceViewType viewType)
    {
        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);
        DescriptorHeap& descriptorHeap = GetDescriptorHeap(viewType);

        ResourceKey key = { resource->GetName().c_str(), viewType };
        auto it = GetResource(key);
        FAIL(it != resources.end(), "Failed to get resource handle");

        return descriptorHeap.GetGPUHandleWithOffset(it->second.HeapIndex);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE ResourceTable::GetResourceCPUHandle(const std::string& resourceName, ResourceViewType viewType)
    {
        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);
        DescriptorHeap& descriptorHeap = GetDescriptorHeap(viewType);

        ResourceKey key = { resourceName.c_str(), viewType };
        auto it = GetResource(key);
        FAIL(it != resources.end(), "Failed to get resource handle");

        return descriptorHeap.GetCPUHandleWithOffset(it->second.HeapIndex);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE ResourceTable::GetResourceGPUHandle(const std::string& resourceName, ResourceViewType viewType)
    {
        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);
        DescriptorHeap& descriptorHeap = GetDescriptorHeap(viewType);

        ResourceKey key = { resourceName.c_str(), viewType };
        auto it = GetResource(key);
        FAIL(it != resources.end(), "Failed to get resource handle");

        return descriptorHeap.GetGPUHandleWithOffset(it->second.HeapIndex);
    }

    std::uint32_t ResourceTable::GetResourceIndex(std::shared_ptr<Resource> resource, ResourceViewType viewType)
    {
        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);

        ResourceKey key = { resource->GetName().c_str(), viewType};
        auto it = GetResource(key);
        FAIL(it != resources.end(), "Failed to get resource handle");

        return it->second.HeapIndex;
    }

    std::uint32_t ResourceTable::GetResourceIndex(const std::string& resourceName, ResourceViewType viewType)
    {
        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);

        ResourceKey key = { resourceName.c_str(), viewType };
        auto it = GetResource(key);
        FAIL(it != resources.end(), "Failed to get resource handle");

        return it->second.HeapIndex;
    }

    std::shared_ptr<Resource> ResourceTable::GetResourceByName(const std::string& resourceName, ResourceViewType viewType)
    {
        ResourceTable::ResourceMap& resources = _GetResourceMap(viewType);

        ResourceKey key = { resourceName.c_str(), viewType };
        auto it = GetResource(key);
        FAIL(it != resources.end(), "Failed to get resource handle");

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
