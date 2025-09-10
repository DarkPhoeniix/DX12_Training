#include "RendererPCH.h"

#include "ResourceTable.h"

namespace internal
{
    struct ViewCreationVisitor
    {
        ViewCreationVisitor(DescriptorHeapManager& descriptorHeapManager)
            : _descriptorHeapManager(descriptorHeapManager)
        {
        }

        std::uint32_t operator()(const dx12::RenderTargetView& view)
        {
            DescriptorHandle descriptor = _descriptorHeapManager.AllocateStatic(DescriptorHeapType::RTV);

            dx12::Device::CreateRenderTargetView(view, descriptor.CpuHandle);

            return descriptor.Index;
        }

        std::uint32_t operator()(const dx12::DepthStencilView& view)
        {
            DescriptorHandle descriptor = _descriptorHeapManager.AllocateStatic(DescriptorHeapType::DSV);

            dx12::Device::CreateDepthStencilView(view, descriptor.CpuHandle);

            return descriptor.Index;
        }

        std::uint32_t operator()(const dx12::ConstantBufferView& view)
        {
            DescriptorHandle descriptor = _descriptorHeapManager.AllocateStatic(DescriptorHeapType::Static);

            dx12::Device::CreateConstantBufferView(view, descriptor.CpuHandle);

            return descriptor.Index;
        }

        std::uint32_t operator()(const dx12::ShaderResourceView& view)
        {
            DescriptorHandle descriptor = _descriptorHeapManager.AllocateStatic(DescriptorHeapType::Static);

            dx12::Device::CreateShaderResourceView(view, descriptor.CpuHandle);

            return descriptor.Index;
        }

        std::uint32_t operator()(const dx12::UnorderedAccessView& view)
        {
            DescriptorHandle descriptor = _descriptorHeapManager.AllocateStatic(DescriptorHeapType::Static);

            dx12::Device::CreateUnorderedAccessView(view, descriptor.CpuHandle);

            return descriptor.Index;
        }

    private:
        DescriptorHeapManager& _descriptorHeapManager;
    };
} // namespace internal

std::unique_ptr<ResourceTable> ResourceTable::_instance = nullptr;

void ResourceTable::Create()
{
    if (_instance)
    {
        LOG_WARNING("ResourceTable instance already exists. Creation skipped.");
        return;
    }
    _instance = std::unique_ptr<ResourceTable>(new ResourceTable());
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

const dx12::DescriptorHeap& ResourceTable::GetShaderResourcesDescriptorHeap() const
{
    return DescriptorHeapManager::Get().GetShaderResourcesDescriptorHeap();
}

void ResourceTable::Reset()
{
    DescriptorHeapManager::Get().Reset();

    _staticRTVs.clear();
    _staticDSVs.clear();
    _staticCBVs.clear();
    _staticSRVs.clear();
    _staticUAVs.clear();

    ResetTransientResources();
}

void ResourceTable::ResetTransientResources()
{
    DescriptorHeapManager::Get().ResetTransient();

    _transientRTVs.clear();
    _transientDSVs.clear();
    _transientCBVs.clear();
    _transientSRVs.clear();
    _transientUAVs.clear();
}
