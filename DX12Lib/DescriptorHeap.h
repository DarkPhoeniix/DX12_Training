#pragma once

#include "DescriptorHeapDescription.h"

namespace dx12
{
    class Resource;

    enum class ResourceViewType
    {
        Unknown,
        RTV,
        DSV,
        CBV,
        SRV,
        UAV
    };

    enum class DescriptorHeapType
    {
        RTV,
        DSV,
        CBV_SRV_UAV
    };

    class DescriptorHeap
    {
    public:
        DescriptorHeap();
        DescriptorHeap(const DescriptorHeapDescription& description);
        ~DescriptorHeap();

        void Create();
        void Create(const DescriptorHeapDescription& description);
        void Reset();

        void PlaceResource(Resource* resource, ResourceViewType viewType);
        void CopyResourceDescriptor(Resource* resource, ResourceViewType viewType, D3D12_CPU_DESCRIPTOR_HANDLE descriptor);

        D3D12_CPU_DESCRIPTOR_HANDLE GetHeapStartCPUHandle();
        D3D12_GPU_DESCRIPTOR_HANDLE GetHeapStartGPUHandle();

        D3D12_CPU_DESCRIPTOR_HANDLE GetResourceCPUHandle(Resource* resource, ResourceViewType viewType);
        D3D12_GPU_DESCRIPTOR_HANDLE GetResourceGPUHandle(Resource* resource, ResourceViewType viewType);

        D3D12_CPU_DESCRIPTOR_HANDLE GetResourceCPUHandle(const std::string& resourceName, ResourceViewType viewType);
        D3D12_GPU_DESCRIPTOR_HANDLE GetResourceGPUHandle(const std::string& resourceName, ResourceViewType viewType);

        UINT GetResourceIndex(Resource* resource, ResourceViewType viewType);
        UINT GetResourceIndex(const std::string& resourceName, ResourceViewType viewType);

        Resource* GetResourceByName(const std::string& name, ResourceViewType viewType);

        void SetDescription(const DescriptorHeapDescription& description);
        const DescriptorHeapDescription& GetDescription() const;

        void SetName(const std::string& name);
        const std::string& GetName() const;

        ComPtr<ID3D12DescriptorHeap> GetDXDescriptorHeap() const;

    private:
        struct InternalResourceDesc
        {
            using ResourceIndex = std::uint32_t;

            Resource* Res;
            ResourceIndex HeapIndex = -1;
            ResourceViewType Type = ResourceViewType::Unknown;
        };

        ComPtr<ID3D12DescriptorHeap> _descriptorHeap;
        DescriptorHeapDescription _description;
        UINT _heapIncrementSize;

        std::unordered_multimap<std::string, InternalResourceDesc> _resources;

        std::string _name;
    };
} // namespace dx12
