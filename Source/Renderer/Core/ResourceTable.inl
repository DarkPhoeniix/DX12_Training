#include "ResourceTable.h"
#pragma once
//
//template<typename T> requires (rhi::ResourceViewConcept<T>)
//DescriptorHandle ResourceTable::AddStaticResourceView(const T& view)
//{
//    if constexpr (std::same_as<T, rhi::RenderTargetView>)
//    {
//        DescriptorHandle handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::RTV);
//        rhi::Device::CreateRenderTargetView(view, handle.CpuHandle);
//        _staticRTVs[view.Owner->GetID()] = handle;
//
//        return handle;
//    }
//    else if constexpr (std::same_as<T, rhi::DepthStencilView>)
//    {
//        DescriptorHandle handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::DSV);
//        rhi::Device::CreateDepthStencilView(view, handle.CpuHandle);
//        _staticDSVs[view.Owner->GetID()] = handle;
//
//        return handle;
//    }
//    else if constexpr (std::same_as<T, rhi::ConstantBufferView>)
//    {
//        DescriptorHandle handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::Static);
//        rhi::Device::CreateConstantBufferView(view, handle.CpuHandle);
//        _staticCBVs[view.Owner->GetID()] = handle;
//
//        return handle;
//    }
//    else if constexpr (std::same_as<T, rhi::ShaderResourceView>)
//    {
//        DescriptorHandle handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::Static);
//        rhi::Device::CreateShaderResourceView(view, handle.CpuHandle);
//        _staticSRVs[view.Owner->GetID()] = handle;
//
//        return handle;
//    }
//    else if constexpr (std::same_as<T, rhi::UnorderedAccessView>)
//    {
//        DescriptorHandle handle = DescriptorHeapManager::Get().AllocateStatic(DescriptorHeapType::Static);
//        rhi::Device::CreateUnorderedAccessView(view, handle.CpuHandle);
//        _staticUAVs[view.Owner->GetID()] = handle;
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
//template<typename T> requires (rhi::ResourceViewConcept<T>)
//DescriptorHandle ResourceTable::AddTransientResourceView(const T& view)
//{
//    if constexpr (std::same_as<T, rhi::RenderTargetView>)
//    {
//        DescriptorHandle handle = DescriptorHeapManager::Get().AllocateTransient(DescriptorHeapType::RTV);
//        rhi::Device::CreateRenderTargetView(view, handle.CpuHandle);
//        _transientRTVs[view.Owner->GetID()] = handle;
//
//        return handle;
//    }
//    else if constexpr (std::same_as<T, rhi::DepthStencilView>)
//    {
//        DescriptorHandle handle = DescriptorHeapManager::Get().AllocateTransient(DescriptorHeapType::DSV);
//        rhi::Device::CreateDepthStencilView(view, handle.CpuHandle);
//        _transientDSVs[view.Owner->GetID()] = handle;
//
//        return handle;
//    }
//    else if constexpr (std::same_as<T, rhi::ConstantBufferView>)
//    {
//        DescriptorHandle handle = DescriptorHeapManager::Get().AllocateTransient(DescriptorHeapType::Static);
//        rhi::Device::CreateConstantBufferView(view, handle.CpuHandle);
//        _transientCBVs[view.Owner->GetID()] = handle;
//
//        return handle;
//    }
//    else if constexpr (std::same_as<T, rhi::ShaderResourceView>)
//    {
//        DescriptorHandle handle = DescriptorHeapManager::Get().AllocateTransient(DescriptorHeapType::Static);
//        rhi::Device::CreateShaderResourceView(view, handle.CpuHandle);
//        _transientSRVs[view.Owner->GetID()] = handle;
//
//        return handle;
//    }
//    else if constexpr (std::same_as<T, rhi::UnorderedAccessView>)
//    {
//        DescriptorHandle handle = DescriptorHeapManager::Get().AllocateTransient(DescriptorHeapType::Static);
//        rhi::Device::CreateUnorderedAccessView(view, handle.CpuHandle);
//        _transientUAVs[view.Owner->GetID()] = handle;
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
//template<typename T> requires (rhi::ResourceViewConcept<T>)
//constexpr DescriptorHandle ResourceTable::GetStaticResourceHandle(const T& desc)
//{
//    if constexpr (std::same_as<T, rhi::RenderTargetView>)
//    {
//        auto it = _staticRTVs.find(desc.Owner->GetID());
//        if (it != _staticRTVs.end())
//        {
//            return it->second;
//        }
//        LOG_WARNING("Render Target View not found in Resource Table.");
//    }
//    else if constexpr (std::same_as<T, rhi::DepthStencilView>)
//    {
//        auto it = _staticDSVs.find(desc.Owner->GetID());
//        if (it != _staticDSVs.end())
//        {
//            return it->second;
//        }
//        LOG_WARNING("Depth Stencil View not found in Resource Table.");
//    }
//    else if constexpr (std::same_as<T, rhi::ConstantBufferView>)
//    {
//        auto it = _staticCBVs.find(desc.Owner->GetID());
//        if (it != _staticCBVs.end())
//        {
//            return it->second;
//        }
//        LOG_WARNING("Constant Buffer View not found in Resource Table.");
//    }
//    else if constexpr (std::same_as<T, rhi::ShaderResourceView>)
//    {
//        auto it = _staticSRVs.find(desc.Owner->GetID());
//        if (it != _staticSRVs.end())
//        {
//            return it->second;
//        }
//        LOG_WARNING("Shader Resource View not found in Resource Table.");
//    }
//    else if constexpr (std::same_as<T, rhi::UnorderedAccessView>)
//    {
//        auto it = _staticUAVs.find(desc.Owner->GetID());
//        if (it != _staticUAVs.end())
//        {
//            return it->second;
//        }
//        LOG_WARNING("Unordered Access View not found in Resource Table.");
//    }
//
//    return AddStaticResourceView(desc);
//}
//
//template<typename T> requires (rhi::ResourceViewConcept<T>)
//constexpr DescriptorHandle ResourceTable::GetTransientResourceHandle(const T& desc)
//{
//    if constexpr (std::same_as<T, rhi::RenderTargetView>)
//    {
//        auto it = _transientRTVs.find(desc.Owner->GetID());
//        if (it != _transientRTVs.end())
//        {
//            return it->second;
//        }
//        LOG_WARNING("Render Target View not found in Resource Table.");
//    }
//    else if constexpr (std::same_as<T, rhi::DepthStencilView>)
//    {
//        auto it = _transientDSVs.find(desc.Owner->GetID());
//        if (it != _transientDSVs.end())
//        {
//            return it->second;
//        }
//        LOG_WARNING("Depth Stencil View not found in Resource Table.");
//    }
//    else if constexpr (std::same_as<T, rhi::ConstantBufferView>)
//    {
//        auto it = _transientCBVs.find(desc.Owner->GetID());
//        if (it != _transientCBVs.end())
//        {
//            return it->second;
//        }
//        LOG_WARNING("Constant Buffer View not found in Resource Table.");
//    }
//    else if constexpr (std::same_as<T, rhi::ShaderResourceView>)
//    {
//        auto it = _transientSRVs.find(desc.Owner->GetID());
//        if (it != _transientSRVs.end())
//        {
//            return it->second;
//        }
//        LOG_WARNING("Shader Resource View not found in Resource Table.");
//    }
//    else if constexpr (std::same_as<T, rhi::UnorderedAccessView>)
//    {
//        auto it = _transientUAVs.find(desc.Owner->GetID());
//        if (it != _transientUAVs.end())
//        {
//            return it->second;
//        }
//        LOG_WARNING("Unordered Access View not found in Resource Table.");
//    }
//
//    return AddTransientResourceView(desc);
//}
