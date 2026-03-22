#pragma once

#include "ResourceCommon.h"

namespace rhi::d3d12
{
    struct RenderTargetView;
    struct DepthStencilView;
    struct ConstantBufferView;
    struct ShaderResourceView;
    struct UnorderedAccessView;

    // D3D12Resource class representing a general GPU resource (e.g., texture, buffer).
    class D3D12Resource : std::enable_shared_from_this<D3D12Resource>
    {
    public:
        D3D12Resource(rhi::Device* device, const BufferDescription& description, const void* initialData = nullptr, const std::string& name = "");
        D3D12Resource(rhi::Device* device, const TextureDescription& description, const void* initialData = nullptr, const std::string& name = "");
        D3D12Resource(rhi::Device* device, ID3D12Resource* resource, const std::string& name = "");
        D3D12Resource(const D3D12Resource& other) = delete;
        D3D12Resource(D3D12Resource&& other) noexcept;
        virtual ~D3D12Resource() = default;

        D3D12Resource& operator=(const D3D12Resource& other) = delete;
        D3D12Resource& operator=(D3D12Resource&& other) noexcept;

        const ResourceID& GetID() const;

        void* Map(std::uint32_t begin, std::uint32_t end);
        void Unmap();

        std::uint64_t GetVirtualAddress() const;

        ResourceState GetInitialState() const;
        ResourceState GetCurrentState() const;

        const AllocationInfo& GetAllocationInfo() const;

        // Get the resource as a Render Target View (RTV) for rendering operations.
        [[nodiscard]] RenderTargetView GetAsRTV();
        // Get the resource as a Depth Stencil View (DSV) for depth/stencil operations.
        [[nodiscard]] DepthStencilView GetAsDSV();
        // Get the resource as a Constant Buffer View (CBV) for constant buffer usage.
        [[nodiscard]] ConstantBufferView GetAsCBV();
        // Get the resource as a Shader D3D12Resource View (SRV) for shader access.
        [[nodiscard]] ShaderResourceView GetAsSRV();
        // Get the resource as an Unordered Access View (UAV) for unordered access operations.
        [[nodiscard]] UnorderedAccessView GetAsUAV();

        void* GetNative() const;

    protected:
        void CreateResource(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_HEAP_PROPERTIES& heapProperties, const void* initialData);

        // Unique identifier assigned by resource factory during construction.
        ResourceID _ID;

        ResourceState _initialState;
        ResourceState _currentState;

        AllocationInfo _allocationInfo;

        D3D12_RESOURCE_DESC _description;
        std::uint32_t _stride;
        std::uint32_t _uavCounterOffset;

        ComPtr<ID3D12Resource> _resource;

        Device* _device;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };

    // Struct for defining a Render Target View (RTV) for a resource.
    struct RenderTargetView : public D3D12_RENDER_TARGET_VIEW_DESC
    {
        std::shared_ptr<D3D12Resource> Owner; // Pointer back to the resource that owns this RTV.
    };

    // Struct for defining a Depth Stencil View (DSV) for a resource.
    struct DepthStencilView : public D3D12_DEPTH_STENCIL_VIEW_DESC
    {
        std::shared_ptr<D3D12Resource> Owner; // Pointer back to the resource that owns this DSV.
    };

    // Struct for defining a Constant Buffer View (CBV) for a resource.
    struct ConstantBufferView : public D3D12_CONSTANT_BUFFER_VIEW_DESC
    {
        std::shared_ptr<D3D12Resource> Owner; // Pointer back to the resource that owns this CBV.
    };

    // Struct for defining a Shader D3D12Resource View (SRV) for a resource.
    struct ShaderResourceView : public D3D12_SHADER_RESOURCE_VIEW_DESC
    {
        std::shared_ptr<D3D12Resource> Owner; // Pointer back to the resource that owns this SRV.
    };

    // Struct for defining an Unordered Access View (UAV) for a resource.
    struct UnorderedAccessView : public D3D12_UNORDERED_ACCESS_VIEW_DESC
    {
        std::shared_ptr<D3D12Resource> Owner; // Pointer back to the resource that owns this UAV.
    };
} // namespace rhi::d3d12
