#pragma once

#include "DescriptorHeap.h"

namespace dx12
{
    class ResourceTable
    {
    public:
        void Init(int numDescriptors, bool shaderVisible = false);
        void Reset();

        bool CopyDescriptor(Resource* resource, ResourceViewType viewType, ResourceTable& srcTable);
        bool CopyDescriptor(Resource* resource, ResourceViewType viewType, D3D12_CPU_DESCRIPTOR_HANDLE handle);
        bool PlaceResource(Resource* resource, ResourceViewType viewType);

        D3D12_CPU_DESCRIPTOR_HANDLE GetResourceCPUHandle(Resource* resource, ResourceViewType viewType);
        D3D12_CPU_DESCRIPTOR_HANDLE GetResourceCPUHandle(const std::string& resourceName, ResourceViewType viewType);

        D3D12_GPU_DESCRIPTOR_HANDLE GetResourceGPUHandle(const std::string& resourceName, ResourceViewType viewType);
        D3D12_GPU_DESCRIPTOR_HANDLE GetResourceGPUHandle(Resource* resource, ResourceViewType viewType);

        UINT GetResourceIndex(Resource* resource, ResourceViewType viewType);
        UINT GetResourceIndex(const std::string& resourceName, ResourceViewType viewType);

        Resource* GetResourceByName(const std::string& resourceName, ResourceViewType viewType);

        DescriptorHeap& GetDescriptorHeap(ResourceViewType viewType);
        const DescriptorHeap& GetDescriptorHeap(ResourceViewType viewType) const;

    private:
        struct ResourceKey
        {
            std::string_view Name;
            ResourceViewType ViewType;

            bool operator==(const ResourceKey& other) const
            {
                return (Name == other.Name) && (ViewType == other.ViewType);
            }
        };

        struct HashResourceKey
        {
            std::size_t operator()(const ResourceKey& key) const
            {
                return std::hash<std::string_view>{}(key.Name);
            }
        };

        struct InternalResourceDesc
        {
            using ResourceIndex = std::uint32_t;

            Resource* PlacedResource;
            ResourceIndex HeapIndex = -1;
            ResourceViewType Type = ResourceViewType::Unknown;
        };

        using ResourceMap = std::unordered_map<ResourceKey, InternalResourceDesc, HashResourceKey>;

        ResourceMap& _GetResourceMap(ResourceViewType viewType);

        ResourceMap _RTVResources;
        ResourceMap _DSVResources;
        ResourceMap _BufferResources;

        DescriptorHeap _RTVDescriptorHeap;
        DescriptorHeap _DSVDescriptorHeap;
        DescriptorHeap _BuffersDescriptorHeap;

        int _numDescriptors;
    };
} // namespace dx12
