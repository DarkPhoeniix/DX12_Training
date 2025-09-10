#pragma once

#include "Resource.h"
#include "Core/DescriptorHeapManager.h"

class ResourceTable
{
public:
    ResourceTable(const ResourceTable&) = delete;
    ResourceTable(ResourceTable&&) noexcept = default;
    ~ResourceTable() = default;

    ResourceTable& operator=(const ResourceTable&) = delete;
    ResourceTable& operator=(ResourceTable&&) noexcept = default;

    static void Create();
    static void Destroy();

    static ResourceTable& Get();

    void Reset();
    void ResetTransientResources();

    const dx12::DescriptorHeap& GetShaderResourcesDescriptorHeap() const;

    template<typename T> requires (dx12::ResourceViewConcept<T>)
        DescriptorHandle AddStaticResourceView(const T& view);
    template<typename T> requires (dx12::ResourceViewConcept<T>)
        DescriptorHandle AddTransientResourceView(const T& view);

    template<typename T> requires (dx12::ResourceViewConcept<T>)
        constexpr DescriptorHandle GetStaticResourceHandle(const T& desc);
    template<typename T> requires (dx12::ResourceViewConcept<T>)
        constexpr DescriptorHandle GetTransientResourceHandle(const T& desc);

private:
    ResourceTable() = default;

    std::unordered_map<dx12::ResourceID, DescriptorHandle> _staticRTVs;
    std::unordered_map<dx12::ResourceID, DescriptorHandle> _staticDSVs;
    std::unordered_map<dx12::ResourceID, DescriptorHandle> _staticCBVs;
    std::unordered_map<dx12::ResourceID, DescriptorHandle> _staticSRVs;
    std::unordered_map<dx12::ResourceID, DescriptorHandle> _staticUAVs;

    std::unordered_map<dx12::ResourceID, DescriptorHandle> _transientRTVs;
    std::unordered_map<dx12::ResourceID, DescriptorHandle> _transientDSVs;
    std::unordered_map<dx12::ResourceID, DescriptorHandle> _transientCBVs;
    std::unordered_map<dx12::ResourceID, DescriptorHandle> _transientSRVs;
    std::unordered_map<dx12::ResourceID, DescriptorHandle> _transientUAVs;

    static std::unique_ptr<ResourceTable> _instance;
};

#include "ResourceTable.inl"
