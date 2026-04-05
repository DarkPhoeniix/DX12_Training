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
//
//rhi::DescriptorHeap* ResourceTable::GetShaderResourcesDescriptorHeap() const
//{
//    return DescriptorHeapManager::Get().GetShaderResourcesDescriptorHeap();
//}
//
//DescriptorHandle ResourceTable::AddStaticBufferView(std::shared_ptr<rhi::Buffer> buffer, rhi::ResourceViewType viewType)
//{
//    if (viewType == rhi::ResourceViewType::CBV)
//    {
//        DescriptorHandle handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::Static);
//        _device->CreateBufferCBV(buffer, handle.CpuHandle);
//        _staticBufferCBVs[buffer->GetID()] = handle;
//
//        return handle;
//    }
//    else if (viewType == rhi::ResourceViewType::SRV)
//    {
//        DescriptorHandle handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::Static);
//        _device->CreateBufferSRV(buffer, handle.CpuHandle);
//        _staticBufferSRVs[buffer->GetID()] = handle;
//
//        return handle;
//    }
//    else if (viewType == rhi::ResourceViewType::UAV)
//    {
//        DescriptorHandle handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::Static);
//        _device->CreateBufferUAV(buffer, handle.CpuHandle);
//        _staticBufferUAVs[buffer->GetID()] = handle;
//
//        return handle;
//    }
//    else
//    {
//        LOG_CRITICAL("Unsupported resource view type.");
//        return {};
//    }
//}
//
//DescriptorHandle ResourceTable::AddTransientTextureView(std::shared_ptr<rhi::Texture> texture, rhi::ResourceViewType viewType)
//{
//    if (viewType == rhi::ResourceViewType::RTV)
//    {
//        DescriptorHandle handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::Static);
//        _device->CreateTextureRTV(texture, handle.CpuHandle);
//        _staticTextureRTVs[texture->GetID()] = handle;
//
//        return handle;
//    }
//    else if (viewType == rhi::ResourceViewType::DSV)
//    {
//        DescriptorHandle handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::Static);
//        _device->CreateTextureDSV(texture, handle.CpuHandle);
//        _staticTextureDSVs[texture->GetID()] = handle;
//
//        return handle;
//    }
//    else if (viewType == rhi::ResourceViewType::SRV)
//    {
//        DescriptorHandle handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::Static);
//        _device->CreateTextureSRV(texture, handle.CpuHandle);
//        _staticTextureSRVs[texture->GetID()] = handle;
//
//        return handle;
//    }
//    else if (viewType == rhi::ResourceViewType::UAV)
//    {
//        DescriptorHandle handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::Static);
//        _device->CreateTextureUAV(texture, handle.CpuHandle);
//        _staticTextureUAVs[texture->GetID()] = handle;
//
//        return handle;
//    }
//    else
//    {
//        LOG_CRITICAL("Unsupported resource view type.");
//        return {};
//    }
//}
//
//DescriptorHandle ResourceTable::GetStaticBufferHandle(std::shared_ptr<rhi::Buffer> buffer, rhi::ResourceViewType viewType)
//{
//    if (viewType == rhi::ResourceViewType::CBV)
//    {
//        auto it = _staticBufferCBVs.find(buffer->GetID());
//        if (it != _staticBufferCBVs.end())
//        {
//            return it->second;
//        }
//        LOG_WARNING("Render Target View not found in Resource Table.");
//    }
//    else if (viewType == rhi::ResourceViewType::SRV)
//    {
//        auto it = _staticBufferSRVs.find(buffer->GetID());
//        if (it != _staticBufferSRVs.end())
//        {
//            return it->second;
//        }
//        LOG_WARNING("Depth Stencil View not found in Resource Table.");
//    }
//    else if (viewType == rhi::ResourceViewType::UAV)
//    {
//        auto it = _staticBufferUAVs.find(buffer->GetID());
//        if (it != _staticBufferUAVs.end())
//        {
//            return it->second;
//        }
//        LOG_WARNING("Constant Buffer View not found in Resource Table.");
//    }
//
//    return AddStaticBufferView(buffer, viewType);
//}
//
//constexpr DescriptorHandle ResourceTable::GetTransientTextureHandle(std::shared_ptr<rhi::Texture> texture, rhi::ResourceViewType viewType)
//{
//    if (viewType == rhi::ResourceViewType::RTV)
//    {
//        auto it = _staticTextureRTVs.find(texture->GetID());
//        if (it != _staticTextureRTVs.end())
//        {
//            return it->second;
//        }
//        LOG_WARNING("Render Target View not found in Resource Table.");
//    }
//    else if (viewType == rhi::ResourceViewType::DSV)
//    {
//        auto it = _staticTextureDSVs.find(texture->GetID());
//        if (it != _staticTextureDSVs.end())
//        {
//            return it->second;
//        }
//        LOG_WARNING("Depth Stencil View not found in Resource Table.");
//    }
//    else if (viewType == rhi::ResourceViewType::SRV)
//    {
//        auto it = _staticTextureSRVs.find(texture->GetID());
//        if (it != _staticTextureSRVs.end())
//        {
//            return it->second;
//        }
//        LOG_WARNING("Depth Stencil View not found in Resource Table.");
//    }
//    else if (viewType == rhi::ResourceViewType::UAV)
//    {
//        auto it = _staticTextureUAVs.find(texture->GetID());
//        if (it != _staticTextureUAVs.end())
//        {
//            return it->second;
//        }
//        LOG_WARNING("Constant Buffer View not found in Resource Table.");
//    }
//
//    return AddStaticTextureView(texture, viewType);
//}

ResourceTable::ResourceTable(rhi::Device* device)
    : _device(device)
{
}

void ResourceTable::CreateStaticResourceView(rhi::ResourceID resourceID, rhi::ResourceViewType viewType)
{
    NOT_IMPLEMENTED();
}

void ResourceTable::CreateTransientResourceView(rhi::ResourceID resourceID, rhi::ResourceViewType viewType)
{
    NOT_IMPLEMENTED();
}

std::uint32_t ResourceTable::GetBindlessIndex(rhi::ResourceID resourceID, rhi::ResourceViewType viewType)
{
    NOT_IMPLEMENTED();

    return std::uint32_t();
}

rhi::CPUDescriptor ResourceTable::GetDescriptor(rhi::ResourceID resourceID, rhi::ResourceViewType viewType)
{
    NOT_IMPLEMENTED();
    return rhi::CPUDescriptor();
}

void ResourceTable::Reset()
{
    DescriptorHeapManager::Get().Reset();

    _staticBufferCBVs.clear();
    _staticBufferSRVs.clear();
    _staticBufferUAVs.clear();

    _staticTextureRTVs.clear();
    _staticTextureDSVs.clear();
    _staticTextureSRVs.clear();
    _staticTextureUAVs.clear();

    ResetTransientResources();
}

void ResourceTable::ResetTransientResources()
{
    DescriptorHeapManager::Get().ResetTransient();

    _transientBufferCBVs.clear();
    _transientBufferSRVs.clear();
    _transientBufferUAVs.clear();

    _transientTextureRTVs.clear();
    _transientTextureDSVs.clear();
    _transientTextureSRVs.clear();
    _transientTextureUAVs.clear();

}
