#pragma once

#include "DescriptorHeap.h"

#include <shared_mutex>

namespace dx12
{
    // Class that manages a collection of resources with associated descriptors in a DirectX 12 application.
    class ResourceTable
    {
    public:
        ResourceTable() = default;
        // Copy constructor.
        ResourceTable(const ResourceTable& other);
        // Move constructor.
        ResourceTable(ResourceTable&& other) noexcept;
        // Destructor.
        ~ResourceTable();

        // Copy assignment operator.
        ResourceTable& operator=(const ResourceTable& other);
        // Move assignment operator.
        ResourceTable& operator=(ResourceTable&& other) noexcept;

        // Initializes the resource table with a specified number of descriptors.
        // Optionally, the descriptors can be shader-visible.
        void Init(std::uint32_t numDescriptors, bool shaderVisible = false);

        // Resets the resource table, clearing any existing data.
        void Reset();

        // Copies a descriptor from another resource table to the current, specifying the resource type (e.g., RTV, DSV).
        // Returns index of the placed resource in the descriptor heap
        std::uint32_t CopyDescriptor(Resource* resource, ResourceViewType viewType, ResourceTable& srcTable);
        // Copies a descriptor handle, specifying the resource type (e.g., RTV, DSV).
        // Returns index of the placed resource in the descriptor heap
        std::uint32_t CopyDescriptor(Resource* resource, ResourceViewType viewType, D3D12_CPU_DESCRIPTOR_HANDLE handle);
        // Places a resource into the table, associating it with a descriptor based on the view type (e.g., RTV, DSV).
        bool PlaceResource(Resource* resource, ResourceViewType viewType);
        // Places a resource into the table if view is not present in the table.
        bool PlaceResourceIfNotExist(Resource* resource, ResourceViewType viewType);

        // Retrieves the CPU descriptor handle for a resource, based on its name and view type.
        D3D12_CPU_DESCRIPTOR_HANDLE GetResourceCPUHandle(Resource* resource, ResourceViewType viewType);
        // Retrieves the CPU descriptor handle for a resource by its name and view type.
        D3D12_CPU_DESCRIPTOR_HANDLE GetResourceCPUHandle(const std::string& resourceName, ResourceViewType viewType);

        // Retrieves the GPU descriptor handle for a resource by its name and view type.
        D3D12_GPU_DESCRIPTOR_HANDLE GetResourceGPUHandle(const std::string& resourceName, ResourceViewType viewType);
        // Retrieves the GPU descriptor handle for a resource based on the resource object and view type.
        D3D12_GPU_DESCRIPTOR_HANDLE GetResourceGPUHandle(Resource* resource, ResourceViewType viewType);

        // Retrieves the index of the resource in the underlying descriptor heap, based on the resource pointer and view type.
        std::uint32_t GetResourceIndex(Resource* resource, ResourceViewType viewType);
        // Retrieves the index of a resource in  the underlying descriptor heap by its name and view type.
        std::uint32_t GetResourceIndex(const std::string& resourceName, ResourceViewType viewType);

        // Retrieves a pointer to a resource by its name and view type.
        Resource* GetResourceByName(const std::string& resourceName, ResourceViewType viewType);

        // Retrieves the descriptor heap associated with a specific view type (e.g., RTV, DSV).
        DescriptorHeap& GetDescriptorHeap(ResourceViewType viewType);
        // Retrieves the constant descriptor heap associated with a specific view type.
        const DescriptorHeap& GetDescriptorHeap(ResourceViewType viewType) const;

    private:
        // A structure representing a unique key for identifying a resource by its name and view type.
        struct ResourceKey
        {
            ResourceKey() = default;
            ResourceKey(std::string_view name, ResourceViewType type)
                : Name(name), ViewType(type)
            {   }
            ResourceKey(const ResourceKey& other)
            {
                Name = other.Name;
                ViewType = other.ViewType;
            }
            ResourceKey(ResourceKey&& other)
            {
                Name = std::move(other.Name);
                ViewType = other.ViewType;
            }
            ResourceKey& operator=(ResourceKey&& other)
            {
                Name = std::move(other.Name);
                ViewType = other.ViewType;

                return *this;
            }
            ResourceKey& operator=(const ResourceKey& other)
            {
                Name = other.Name;
                ViewType = other.ViewType;

                return *this;
            }

            std::string_view Name;      // Resource's name (e.g., texture name).
            ResourceViewType ViewType;  // The type of resource view (e.g., RTV, DSV).

            // Equality operator to compare resource keys.
            bool operator==(const ResourceKey& other) const
            {
                return (Name == other.Name) && (ViewType == other.ViewType);
            }
        };

        // Hash function for the ResourceKey, allowing it to be used as a key in unordered maps.
        struct HashResourceKey
        {
            std::size_t operator()(const ResourceKey& key) const
            {
                return std::hash<std::string_view>{}(key.Name);
            }
        };

        // Internal structure representing the descriptor and placement details of a resource in the table.
        struct InternalResourceDesc
        {
            using ResourceIndex = std::uint32_t;

            InternalResourceDesc() = default;
            InternalResourceDesc(Resource* resource, ResourceIndex index, ResourceViewType type)
                : PlacedResource(resource), HeapIndex(index), Type(type)
            {   }
            InternalResourceDesc(const InternalResourceDesc& other)
            {
                PlacedResource = other.PlacedResource;
                HeapIndex = other.HeapIndex;
                Type = other.Type;
            }
            InternalResourceDesc(InternalResourceDesc&& other)
            {
                PlacedResource = other.PlacedResource;
                HeapIndex = other.HeapIndex;
                Type = other.Type;
            }
            InternalResourceDesc& operator=(InternalResourceDesc&& other)
            {
                PlacedResource = other.PlacedResource;
                HeapIndex = other.HeapIndex;
                Type = other.Type;

                return *this;
            }
            InternalResourceDesc& operator=(const InternalResourceDesc& other)
            {
                PlacedResource = other.PlacedResource;
                HeapIndex = other.HeapIndex;
                Type = other.Type;

                return *this;
            }

            Resource* PlacedResource = nullptr;                         // Pointer to the resource in the table.
            ResourceIndex HeapIndex = static_cast<std::uint32_t>(-1);   // Index of the resource in the descriptor heap.
            ResourceViewType Type = ResourceViewType::Unknown;          // The type of resource view (RTV, DSV, etc.).
        };

        // A map that stores resources by their keys, allowing quick access to their descriptors and details.
        using ResourceMap = std::unordered_map<ResourceKey, InternalResourceDesc, HashResourceKey>;

        // Private method to retrieve the resource map associated with a specific view type (e.g., RTV, DSV).
        ResourceMap& _GetResourceMap(ResourceViewType viewType);
        auto GetResource(const ResourceKey& key);

        // Resource maps for different types of resources: RTV, DSV, and Buffers.
        ResourceMap _RTVResources;
        ResourceMap _DSVResources;
        ResourceMap _BufferResources;

        // Descriptor heaps for different types of resources: RTV, DSV, and Buffers.
        DescriptorHeap _RTVDescriptorHeap;
        DescriptorHeap _DSVDescriptorHeap;
        DescriptorHeap _BuffersDescriptorHeap;

        // The total number of descriptors allocated in the table.
        std::uint32_t _numDescriptors;

        mutable std::shared_mutex _mutex;
    };
} // namespace dx12
