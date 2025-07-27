#pragma once

#include "Resource.h"
#include "Core/DescriptorHeapManager.h"

class ResourceTable
{
public:
    ResourceTable(DescriptorHeapManager& descriptorHeapManager);
    ~ResourceTable() = default;

    template<typename T> requires (dx12::ResourceViewConcept<T>)
        DescriptorHandle AddStaticResourceView(const T& view);
    template<typename T> requires (dx12::ResourceViewConcept<T>)
        DescriptorHandle AddTransientResourceView(const T& view);

    template<typename T> requires (dx12::ResourceViewConcept<T>)
        constexpr DescriptorHandle GetResourceHandle(const T& desc) const;

    const dx12::DescriptorHeap& GetShaderResourcesDescriptorHeap() const;

private:
    DescriptorHeapManager& _descriptorHeapManager;

    std::unordered_map<dx12::ResourceID, DescriptorHandle> _RTVs;
    std::unordered_map<dx12::ResourceID, DescriptorHandle> _DSVs;
    std::unordered_map<dx12::ResourceID, DescriptorHandle> _CBVs;
    std::unordered_map<dx12::ResourceID, DescriptorHandle> _SRVs;
    std::unordered_map<dx12::ResourceID, DescriptorHandle> _UAVs;
};

#include "ResourceTable.inl"
