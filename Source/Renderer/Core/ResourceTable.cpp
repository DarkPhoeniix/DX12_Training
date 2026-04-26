#include "RendererPCH.h"

#include "ResourceTable.h"

std::unique_ptr<ResourceTable> ResourceTable::_instance = nullptr;

void ResourceTable::Create(rhi::Device* device)
{
    if (_instance)
    {
        LOG_WARNING("ResourceTable instance already exists. Creation skipped.");
        return;
    }
    _instance = std::unique_ptr<ResourceTable>(new ResourceTable(device));
}

void ResourceTable::Destroy()
{
    if (_instance)
    {
        _instance.reset();
    }
    else
    {
        LOG_WARNING("ResourceTable instance does not exist. Destruction skipped.");
    }
}

ResourceTable& ResourceTable::Get()
{
    ASSERT(_instance, "ResourceTable instance is not created. Call Create() first.");
    return *_instance;
}

ResourceTable::ResourceTable(rhi::Device* device)
    : _device(device)
{
}

void ResourceTable::CreateStaticResourceView(const rhi::BufferView& view)
{
    DescriptorHandle handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::Static);

    _device->CreateBufferView(view, handle.CpuHandle);

    std::unordered_map<rhi::ResourceID, DescriptorHandle>& bufferMap = GetStaticResourceMap(view.GetType());
    bufferMap[view.GetBuffer()->GetID()] = handle;
}

void ResourceTable::CreateStaticResourceView(const rhi::TextureView& view)
{
    DescriptorHandle handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::Static);

    _device->CreateTextureView(view, handle.CpuHandle);

    std::unordered_map<rhi::ResourceID, DescriptorHandle>& textureMap = GetStaticResourceMap(view.GetType());
    textureMap[view.GetTexture()->GetID()] = handle;
}

void ResourceTable::CreateStaticResourceView(std::shared_ptr<rhi::Buffer> buffer, rhi::ResourceViewType viewType)
{
    if (FindHandle(buffer->GetID(), viewType).Index != InvalidHeapIndex)
    {
        return;
    }

    DescriptorHandle handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::Static);

    switch (viewType)
    {
    case rhi::ResourceViewType::CBV:
        _device->CreateBufferCBV(buffer, handle.CpuHandle);
        break;
    case rhi::ResourceViewType::SRV:
        _device->CreateBufferSRV(buffer, handle.CpuHandle);
        break;
    case rhi::ResourceViewType::UAV:
        _device->CreateBufferUAV(buffer, handle.CpuHandle);
        break;
    default:
        UNREACHABLE("Unsupported buffer view type.");
        break;
    }

    std::unordered_map<rhi::ResourceID, DescriptorHandle>& bufferMap = GetStaticResourceMap(viewType);
    bufferMap[buffer->GetID()] = handle;
}

void ResourceTable::CreateStaticResourceView(std::shared_ptr<rhi::Texture> texture, rhi::ResourceViewType viewType)
{
    if (FindHandle(texture->GetID(), viewType).Index != InvalidHeapIndex)
    {
        return;
    }

    DescriptorHandle handle;

    switch (viewType)
    {
    case rhi::ResourceViewType::RTV:
        handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::RTV);
        _device->CreateTextureRTV(texture, handle.CpuHandle);
        break;
    case rhi::ResourceViewType::DSV:
        handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::DSV);
        _device->CreateTextureDSV(texture, handle.CpuHandle);
        break;
    case rhi::ResourceViewType::SRV:
        handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::Static);
        _device->CreateTextureSRV(texture, handle.CpuHandle);
        break;
    case rhi::ResourceViewType::UAV:
        handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::Static);
        _device->CreateTextureUAV(texture, handle.CpuHandle);
        break;
    default:
        UNREACHABLE("Unsupported texture view type.");
        break;
    }

    std::unordered_map<rhi::ResourceID, DescriptorHandle>& textureMap = GetStaticResourceMap(viewType);
    textureMap[texture->GetID()] = handle;
}

void ResourceTable::CreateTransientResourceView(std::shared_ptr<rhi::Buffer> buffer, rhi::ResourceViewType viewType)
{
    DescriptorHandle handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::Dynamic);

    switch (viewType)
    {
    case rhi::ResourceViewType::CBV:
        _device->CreateBufferCBV(buffer, handle.CpuHandle);
        break;
    case rhi::ResourceViewType::SRV:
        _device->CreateBufferSRV(buffer, handle.CpuHandle);
        break;
    case rhi::ResourceViewType::UAV:
        _device->CreateBufferUAV(buffer, handle.CpuHandle);
        break;
    default:
        UNREACHABLE("Unsupported buffer view type.");
        break;
    }

    std::unordered_map<rhi::ResourceID, DescriptorHandle>& bufferMap = GetTransientResourceMap(viewType);
    bufferMap[buffer->GetID()] = handle;
}

void ResourceTable::CreateTransientResourceView(std::shared_ptr<rhi::Texture> texture, rhi::ResourceViewType viewType)
{
    DescriptorHandle handle;

    switch (viewType)
    {
    case rhi::ResourceViewType::RTV:
        handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::RTV);
        _device->CreateTextureRTV(texture, handle.CpuHandle);
        break;
    case rhi::ResourceViewType::DSV:
        handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::DSV);
        _device->CreateTextureDSV(texture, handle.CpuHandle);
        break;
    case rhi::ResourceViewType::SRV:
        handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::Dynamic);
        _device->CreateTextureSRV(texture, handle.CpuHandle);
        break;
    case rhi::ResourceViewType::UAV:
        handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::Dynamic);
        _device->CreateTextureUAV(texture, handle.CpuHandle);
        break;
    default:
        UNREACHABLE("Unsupported texture view type.");
        break;
    }

    std::unordered_map<rhi::ResourceID, DescriptorHandle>& textureMap = GetTransientResourceMap(viewType);
    textureMap[texture->GetID()] = handle;
}

std::uint32_t ResourceTable::GetBindlessIndex(rhi::ResourceID resourceID, rhi::ResourceViewType viewType)
{
    std::uint32_t index = FindHandle(resourceID, viewType).Index;
    return index;
}

rhi::CPUDescriptor ResourceTable::GetDescriptor(rhi::ResourceID resourceID, rhi::ResourceViewType viewType)
{
    rhi::CPUDescriptor descriptor = FindHandle(resourceID, viewType).CpuHandle;
    return descriptor;
}

void ResourceTable::Reset()
{
    DescriptorHeapManager::Get().Reset();

    _staticResourceRTVs.clear();
    _staticResourceDSVs.clear();
    _staticResourceCBVs.clear();
    _staticResourceSRVs.clear();
    _staticResourceUAVs.clear();

    ResetTransientResources();
}

void ResourceTable::ResetTransientResources()
{
    DescriptorHeapManager::Get().ResetTransient();

    _transientResourceRTVs.clear();
    _transientResourceDSVs.clear();
    _transientResourceCBVs.clear();
    _transientResourceSRVs.clear();
    _transientResourceUAVs.clear();
}

std::unordered_map<rhi::ResourceID, DescriptorHandle>& ResourceTable::GetStaticResourceMap(rhi::ResourceViewType viewType)
{
    switch (viewType)
    {
    case rhi::ResourceViewType::RTV:
        return _staticResourceRTVs;
    case rhi::ResourceViewType::DSV:
        return _staticResourceDSVs;
    case rhi::ResourceViewType::CBV:
        return _staticResourceCBVs;
    case rhi::ResourceViewType::SRV:
        return _staticResourceSRVs;
    case rhi::ResourceViewType::UAV:
        return _staticResourceUAVs;
    default:
        UNREACHABLE("Unsupported resource view type.");
        return _staticResourceRTVs;
    }
}

std::unordered_map<rhi::ResourceID, DescriptorHandle>& ResourceTable::GetTransientResourceMap(rhi::ResourceViewType viewType)
{
    switch (viewType)
    {
    case rhi::ResourceViewType::RTV:
        return _transientResourceRTVs;
    case rhi::ResourceViewType::DSV:
        return _transientResourceDSVs;
    case rhi::ResourceViewType::CBV:
        return _transientResourceCBVs;
    case rhi::ResourceViewType::SRV:
        return _transientResourceSRVs;
    case rhi::ResourceViewType::UAV:
        return _transientResourceUAVs;
    default:
        UNREACHABLE("Unsupported resource view type.");
        return _transientResourceRTVs;
    }
}

DescriptorHandle ResourceTable::FindHandle(rhi::ResourceID resourceID, rhi::ResourceViewType viewType)
{
    std::unordered_map<rhi::ResourceID, DescriptorHandle>& transientMap = GetTransientResourceMap(viewType);
    if (auto it = transientMap.find(resourceID); it != transientMap.end())
    {
        return it->second;
    }

    std::unordered_map<rhi::ResourceID, DescriptorHandle>& staticMap = GetStaticResourceMap(viewType);
    if (auto it = staticMap.find(resourceID); it != staticMap.end())
    {
        return it->second;
    }

    LOG_ERROR("Failed to find resource handle in the ResourceTable.");
    return DescriptorHandle{};
}
