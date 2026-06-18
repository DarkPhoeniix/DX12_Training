#pragma once

#include "ResourceCommon.h"

namespace D3D12MA
{
    class Allocator;
    class Allocation;
} // namespace D3D12MA

namespace rhi::d3d12
{
    class D3D12Resource : std::enable_shared_from_this<D3D12Resource>
    {
    public:
        D3D12Resource(rhi::Device* device, D3D12MA::Allocator* allocator, const BufferDescription& description, ResourceState initialState = ResourceState::Common, const std::string& name = "");
        D3D12Resource(rhi::Device* device, D3D12MA::Allocator* allocator, const TextureDescription& description, ResourceState initialState = ResourceState::Common, const std::string& name = "");
        D3D12Resource(rhi::Device* device, const BufferDescription& description, rhi::Heap* heap, std::uint64_t offset, ResourceState initialState = ResourceState::Common, const std::string& name = "");
        D3D12Resource(rhi::Device* device, const TextureDescription& description, rhi::Heap* heap, std::uint64_t offset, ResourceState initialState = ResourceState::Common, const std::string& name = "");
        D3D12Resource(rhi::Device* device, ID3D12Resource* resource, const std::string& name = "");
        D3D12Resource(const D3D12Resource& other) = delete;
        D3D12Resource(D3D12Resource&& other) noexcept;
        virtual ~D3D12Resource();

        D3D12Resource& operator=(const D3D12Resource& other) = delete;
        D3D12Resource& operator=(D3D12Resource&& other) noexcept;

        const ResourceID& GetID() const;

        void* Map(std::uint32_t begin, std::uint32_t end);
        void Unmap();

        std::uint64_t GetVirtualAddress() const;

        ResourceState GetInitialState() const;
        ResourceState GetCurrentState() const;
        void SetCurrentState(ResourceState state);

        std::uint32_t GetUAVCounterOffset() const;

        void* GetNative() const;

    protected:
        void CreateCommitedResource(D3D12MA::Allocator* allocator, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_HEAP_PROPERTIES& heapProperties, D3D12_CLEAR_VALUE* clearValue);
        void CreatePlacedResource(const D3D12_RESOURCE_DESC& resourceDesc, rhi::Heap* heap, std::uint64_t offset, D3D12_CLEAR_VALUE* clearValue);

        ResourceID _ID;

        ResourceState _initialState;
        ResourceState _currentState;

        D3D12_RESOURCE_DESC _description;
        std::uint32_t _stride;
        std::uint32_t _uavCounterOffset;

        ComPtr<ID3D12Resource> _resource;
        ComPtr<D3D12MA::Allocation> _allocation;

        rhi::Device* _device;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::d3d12
