#pragma once

#include "ResourceDescription.h"

class ResourceFactory;

namespace dx12
{
    struct RenderTargetView;
    struct DepthStencilView;
    struct ConstantBufferView;
    struct ShaderResourceView;
    struct UnorderedAccessView;

    using ResourceID = std::uint64_t;
    static ResourceID InvalidResourceID = ResourceID(-1);

    // Resource class representing a general GPU resource (e.g., texture, buffer).
    class Resource : public std::enable_shared_from_this<Resource>
    {
    public:
        friend class ResourceFactory;

        // Copy constructor.
        Resource(const Resource& other);
        // Move constructor.
        Resource(Resource&& other) noexcept;
        // Virtual destructor for proper cleanup of derived classes.
        virtual ~Resource();

        // Copy assignment operator.
        Resource& operator=(const Resource& other);
        // Move assignment operator.
        Resource& operator=(Resource&& other) noexcept;

        // Getter for the raw DirectX 12 resource.
        [[nodiscard]] ComPtr<ID3D12Resource> GetDXResource() const;
        [[nodiscard]] ComPtr<ID3D12Resource>& GetDXResource();

        const ResourceID& GetID() const;

        // Sets the name for the resource for debugging and identification.
        void SetName(const std::string& name);
        // Getter for the name of the resource.
        const std::string& GetName() const;

        // Sets the resource description, detailing the resource's properties.
        void SetResourceDescription(const ResourceDescription& resourceDesc);
        // Getter for the resource description.
        [[nodiscard]] ResourceDescription GetResourceDescription() const;

        // Sets the current state of the resource (e.g., copy, render target).
        void SetCurrentState(D3D12_RESOURCE_STATES state);
        // Getter for the current state of the resource.
        D3D12_RESOURCE_STATES GetCurrentState() const;

        // Getter for resource allocation info (e.g., size, alignment).
        [[nodiscard]] D3D12_RESOURCE_ALLOCATION_INFO GetAllocationInfo() const;

        // Computes the GPU virtual address offset for the resource.
        D3D12_GPU_VIRTUAL_ADDRESS OffsetGPU(std::uint64_t offset = 0) const;

        // Maps the resource to a CPU accessible memory region for reading/writing.
        template<typename Type>
        Type* Map(std::uint32_t begin = 0, std::uint32_t end = 0); // Maps a specific range of the resource.
        // Unmaps the resource after mapping is complete.
        void Unmap();

        // Resets the resource to its initial state.
        void Reset();

        // Create a committed resource.
        ComPtr<ID3D12Resource> CreateCommitedResource(D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON);
        // Create a placed resource (resource placed in a specific memory heap).
        ComPtr<ID3D12Resource> CreatePlacedResource(ComPtr<ID3D12Heap> heap, std::uint64_t offset, D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON);

        // Get the resource as a Render Target View (RTV) for rendering operations.
        [[nodiscard]] RenderTargetView GetAsRTV();
        // Get the resource as a Depth Stencil View (DSV) for depth/stencil operations.
        [[nodiscard]] DepthStencilView GetAsDSV();
        // Get the resource as a Constant Buffer View (CBV) for constant buffer usage.
        [[nodiscard]] ConstantBufferView GetAsCBV();
        // Get the resource as a Shader Resource View (SRV) for shader access.
        [[nodiscard]] ShaderResourceView GetAsSRV();
        // Get the resource as an Unordered Access View (UAV) for unordered access operations.
        [[nodiscard]] UnorderedAccessView GetAsUAV();

    protected:
        // Default constructor for the Resource class.
        Resource(const std::string& name);
        // Constructor to initialize Resource with a given ResourceDescription.
        Resource(const std::string& name, const ResourceDescription& resourceDesc);
        // Initializes the Resource from an existing DirectX 12 resource.
        Resource(const std::string& name, ComPtr<ID3D12Resource> resource);

        // Unique identifier assigned by resource factory during construction.
        ResourceID _ID;

        // The DirectX 12 resource pointer (e.g., ID3D12Resource) representing the actual GPU resource.
        ComPtr<ID3D12Resource> _resource;

        // Name for the resource, used for debugging and identification.
        std::string _name;

        // The resource description that holds properties like dimension, format, etc.
        ResourceDescription _resourceDesc;

        // The initial state of the resource when it was created.
        D3D12_RESOURCE_STATES _initialState;
        // The current state of the resource during its lifecycle.
        D3D12_RESOURCE_STATES _currentState;

        // Allocation info (e.g., size, alignment) of the resource.
        D3D12_RESOURCE_ALLOCATION_INFO _allocationInfo;
        uint32_t _uavCounterOffset;
    };

    // Struct for defining a Render Target View (RTV) for a resource.
    struct RenderTargetView : public D3D12_RENDER_TARGET_VIEW_DESC
    {
        std::shared_ptr<Resource> Owner; // Pointer back to the resource that owns this RTV.
    };

    // Struct for defining a Depth Stencil View (DSV) for a resource.
    struct DepthStencilView : public D3D12_DEPTH_STENCIL_VIEW_DESC
    {
        std::shared_ptr<Resource> Owner; // Pointer back to the resource that owns this DSV.
    };

    // Struct for defining a Constant Buffer View (CBV) for a resource.
    struct ConstantBufferView : public D3D12_CONSTANT_BUFFER_VIEW_DESC
    {
        std::shared_ptr<Resource> Owner; // Pointer back to the resource that owns this CBV.
    };

    // Struct for defining a Shader Resource View (SRV) for a resource.
    struct ShaderResourceView : public D3D12_SHADER_RESOURCE_VIEW_DESC
    {
        std::shared_ptr<Resource> Owner; // Pointer back to the resource that owns this SRV.
    };

    // Struct for defining an Unordered Access View (UAV) for a resource.
    struct UnorderedAccessView : public D3D12_UNORDERED_ACCESS_VIEW_DESC
    {
        std::shared_ptr<Resource> Owner; // Pointer back to the resource that owns this UAV.
    };

    template<typename T>
    concept ResourceViewConcept =
        std::same_as<T, dx12::RenderTargetView> ||
        std::same_as<T, dx12::DepthStencilView> ||
        std::same_as<T, dx12::ConstantBufferView> ||
        std::same_as<T, dx12::ShaderResourceView> ||
        std::same_as<T, dx12::UnorderedAccessView>;

    template<typename Type>
    Type* Resource::Map(std::uint32_t begin, std::uint32_t end)
    {
        void* data = nullptr;

        D3D12_RANGE range;
        range.Begin = begin;
        range.End = end;
        _resource->Map(0, &range, &data);

        return (Type*)data;
    }
} // namespace dx12
