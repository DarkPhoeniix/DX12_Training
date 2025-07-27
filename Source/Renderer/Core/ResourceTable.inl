#include "ResourceTable.h"
#pragma once

template<typename T> requires (dx12::ResourceViewConcept<T>)
DescriptorHandle ResourceTable::AddStaticResourceView(const T& view)
{
    if constexpr (std::same_as<T, dx12::RenderTargetView>)
    {
        DescriptorHandle handle = _descriptorHeapManager.AllocateStatic(DescriptorHeapType::RTV);
        dx12::Device::CreateRenderTargetView(view, handle.CpuHandle);
        _RTVs[view.Owner->GetID()] = handle;

        return handle;
    }
    else if constexpr (std::same_as<T, dx12::DepthStencilView>)
    {
        DescriptorHandle handle = _descriptorHeapManager.AllocateStatic(DescriptorHeapType::DSV);
        dx12::Device::CreateDepthStencilView(view, handle.CpuHandle);
        _DSVs[view.Owner->GetID()] = handle;

        return handle;
    }
    else if constexpr (std::same_as<T, dx12::ConstantBufferView>)
    {
        DescriptorHandle handle = _descriptorHeapManager.AllocateStatic(DescriptorHeapType::Static);
        dx12::Device::CreateConstantBufferView(view, handle.CpuHandle);
        _CBVs[view.Owner->GetID()] = handle;

        return handle;
    }
    else if constexpr (std::same_as<T, dx12::ShaderResourceView>)
    {
        DescriptorHandle handle = _descriptorHeapManager.AllocateStatic(DescriptorHeapType::Static);
        dx12::Device::CreateShaderResourceView(view, handle.CpuHandle);
        _SRVs[view.Owner->GetID()] = handle;

        return handle;
    }
    else if constexpr (std::same_as<T, dx12::UnorderedAccessView>)
    {
        DescriptorHandle handle = _descriptorHeapManager.AllocateStatic(DescriptorHeapType::Static);
        dx12::Device::CreateUnorderedAccessView(view, handle.CpuHandle);
        _UAVs[view.Owner->GetID()] = handle;

        return handle;
    }
}

template<typename T> requires (dx12::ResourceViewConcept<T>)
DescriptorHandle ResourceTable::AddTransientResourceView(const T& view)
{
    DescriptorHandle handle = _descriptorHeapManager.AllocateTransient();
    dx12::Device::CreateRenderTargetView(view, handle);
    // TODO: ResourceTable::AddTransientResourceView

    if constexpr (std::same_as<T, dx12::RenderTargetView>)
    {
        _RTVs[view.Owner->GetID()] = handle;
    }
    else if constexpr (std::same_as<T, dx12::DepthStencilView>)
    {
        _DSVs[view.Owner->GetID()] = handle;
    }
    else if constexpr (std::same_as<T, dx12::ConstantBufferView>)
    {
        _CBVs[view.Owner->GetID()] = handle;
    }
    else if constexpr (std::same_as<T, dx12::ShaderResourceView>)
    {
        _SRVs[view.Owner->GetID()] = handle;
    }
    else if constexpr (std::same_as<T, dx12::UnorderedAccessView>)
    {
        _UAVs[view.Owner->GetID()] = handle;
    }

    return handle;
}

template<typename T> requires (dx12::ResourceViewConcept<T>)
constexpr DescriptorHandle ResourceTable::GetResourceHandle(const T& desc) const
{
    if constexpr (std::same_as<T, dx12::RenderTargetView>)
    {
        auto it = _RTVs.find(desc.Owner->GetID());
        if (it != _RTVs.end())
        {
            return it->second;
        }
        LOG_ERROR("Render Target View not found in Resource Table.");
    }
    else if constexpr (std::same_as<T, dx12::DepthStencilView>)
    {
        auto it = _DSVs.find(desc.Owner->GetID());
        if (it != _DSVs.end())
        {
            return it->second;
        }
        LOG_ERROR("Depth Stencil View not found in Resource Table.");
    }
    else if constexpr (std::same_as<T, dx12::ConstantBufferView>)
    {
        auto it = _CBVs.find(desc.Owner->GetID());
        if (it != _CBVs.end())
        {
            return it->second;
        }
        LOG_ERROR("Constant Buffer View not found in Resource Table.");
    }
    else if constexpr (std::same_as<T, dx12::ShaderResourceView>)
    {
        auto it = _SRVs.find(desc.Owner->GetID());
        if (it != _SRVs.end())
        {
            return it->second;
        }
        LOG_ERROR("Shader Resource View not found in Resource Table.");
    }
    else if constexpr (std::same_as<T, dx12::UnorderedAccessView>)
    {
        auto it = _UAVs.find(desc.Owner->GetID());
        if (it != _UAVs.end())
        {
            return it->second;
        }
        LOG_ERROR("Unordered Access View not found in Resource Table.");
    }

    return DescriptorHandle();
}
