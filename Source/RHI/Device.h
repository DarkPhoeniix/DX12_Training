#pragma once

#include "CommandList.h"
#include "Descriptor.h"
#include "DescriptorHeap.h"
#include "ResourceCommon.h"

namespace tracking
{
    class IGPUCrashTracker;
} // namespace tracking

namespace rhi
{
    class CommandListPool;
    class CommandQueue;
    class CommandSignature;
    class PipelineState;
    class Fence;
    class Buffer;
    class BufferView;
    class Texture;
    class TextureView;
    class DescriptorHeap;
    class QueryHeap;
    class SwapChain;
    class StatisticsQuery;
    class TimestampQuery;
    struct BufferDescription;
    struct TextureDescription;
    struct IndirectArgumentDescription;
    struct DescriptorHeapDescription;
    struct QueryHeapDescription;

    // BackendAPI represents the graphics API that the device is using
    enum class BackendAPI
    {
        D3D12,
        Vulkan
    };

    // DeviceOptionalFeatures encapsulates optional features that a graphics device may support, allowing the application to query and utilize these features 
    // for enhanced performance and capabilities.
    struct DeviceOptionalFeatures
    {
        // Indicates whether the device supports enhanced barriers, which can optimize resource state transitions
        bool EnhancedBarriers = false;
    };

    struct VideoMemoryInfo
    {
        // The total video memory budget available to the application
        std::uint64_t Budget = 0;
        // The current video memory usage by the application, in bytes
        std::uint64_t CurrentUsage = 0;
        // The amount of video memory that the application has available for reservation
        std::uint64_t AvailableForReservation = 0;
        // The amount of video memory that the application has currently reserved
        std::uint64_t CurrentReservation = 0;
    };

    struct AdapterInfo
    {
        std::string Name;
        std::string Vendor;
        std::string DeviceId;

        VideoMemoryInfo VideoMemory = {};
    };

    // AllocatorStats reports the internal suballocation accounting of the device's resource allocator.
    // Unlike VideoMemoryInfo (OS-reported, whole-process), these numbers cover only memory managed by the allocator.
    struct AllocatorStats
    {
        // Bytes actually occupied by live allocations
        std::uint64_t AllocationBytes = 0;
        // Bytes reserved in memory blocks/heaps; always >= AllocationBytes, the difference being free suballocation headroom
        std::uint64_t BlockBytes = 0;
        // Number of live allocations
        std::uint32_t AllocationCount = 0;
        // Number of memory blocks/heaps backing the allocations
        std::uint32_t BlockCount = 0;
    };

    // Device is an abstract interface representing a graphics device, responsible for managing GPU resources, command queues, and swap chains. It provides 
    // methods for creating buffers, textures, command lists, descriptor heaps, fences, query heaps, heaps, statistics queries, timestamp queries, command 
    // signatures, swap chains, and pipeline states. The device also allows querying for optional features and binding a swap chain for presentation.
    class Device
    {
    public:
        Device() = default;
        Device(const Device&) = delete;
        Device(Device&&) noexcept = default;
        virtual ~Device() = default;

        Device& operator=(const Device&) = delete;
        Device& operator=(Device&&) noexcept = default;

        virtual BackendAPI GetBackend() const = 0;

        // Checks if the device supports enhanced barriers, which can optimize resource state transitions
        virtual bool IsEnhancedBarriersSupported() = 0;

        // Binds the swap chain to the device, allowing it to manage the presentation of rendered frames
        virtual void BindSwapChain(SwapChain* swapChain) = 0;

        // Retrieves the command queue for the specified command list type
        virtual CommandQueue* GetQueue(rhi::CommandListType type) = 0;
        // Convenience methods for retrieving specific command queues
        // Get the graphics command queue, used for rendering operations
        virtual CommandQueue* GetGraphicsQueue() = 0;
        // Get the compute command queue, used for general-purpose GPU computations
        virtual CommandQueue* GetComputeQueue() = 0;
        // Get the copy command queue, used for resource copying and data transfer operations
        virtual CommandQueue* GetCopyQueue() = 0;

        // Handles resizing of the swap chain's back buffer, allowing the device to adjust its resources and state accordingly
        virtual void OnResize(std::uint32_t width, std::uint32_t height) = 0;
        // Retrieves the current back buffer texture, which is the render target for the next frame to be presented
        virtual std::shared_ptr<Texture> GetBackBuffer() = 0;

        // Presents the rendered frame to the screen, typically by swapping the back buffer with the front buffer in the swap chain
        virtual void Present() = 0;

        // Resource creation methods for buffers and textures, allowing the device to manage GPU resources

        // Creates a buffer resource with the specified description, initial state, and optional name for debugging purposes
        virtual std::shared_ptr<Buffer> CreateBuffer(const BufferDescription& description, ResourceState initialState = ResourceState::Common, const std::string& name = "") = 0;
        // Creates a buffer resource that wraps an existing native buffer pointer, with an optional name for debugging purposes
        virtual std::shared_ptr<Buffer> CreateBuffer(void* nativePtr, const std::string& name = "") = 0;
        // Creates a texture resource with the specified description, initial state, and optional name for debugging purposes
        virtual std::shared_ptr<Texture> CreateTexture(const TextureDescription& description, ResourceState initialState = ResourceState::Common, const std::string& name = "") = 0;
        // Creates a texture resource that wraps an existing native texture pointer, with an optional name for debugging purposes
        virtual std::shared_ptr<Texture> CreateTexture(void* nativePtr, const std::string& name = "") = 0;

        // Creation methods for various GPU objects

        // Creates a command list of the specified type with an optional name for debugging purposes, allowing the application to record GPU commands for execution
        virtual std::unique_ptr<CommandList> CreateCommandList(CommandListType type, const std::string& name = "") = 0;
        // Creates a command list pool, which can be used to manage and reuse command lists for efficient GPU command recording and execution
        virtual std::unique_ptr<CommandListPool> CreateCommandListPool() = 0;
        // Creates a descriptor heap based on the provided description and an optional name for debugging purposes, allowing the application to manage GPU 
        // resource descriptors for shader binding and rendering operations
        virtual std::unique_ptr<DescriptorHeap> CreateDescriptorHeap(const DescriptorHeapDescription& description, const std::string& name = "") = 0;
        // Creates a query heap based on the provided description and an optional name for debugging purposes, allowing the application to manage GPU queries 
        // for performance measurements and other data collection
        virtual std::unique_ptr<QueryHeap> CreateQueryHeap(const QueryHeapDescription& description, const std::string& name = "") = 0;
        // Creates a fence with the specified initial value, allowing the application to synchronize GPU and CPU operations by signaling and waiting on the fence
        virtual std::unique_ptr<Fence> CreateFence(std::uint64_t initialValue) = 0;
        // Creates a statistics query, which can be used to gather performance data and other metrics from the GPU, with an optional name for debugging purposes
        virtual std::unique_ptr<StatisticsQuery> CreateStatisticsQuery(const std::string& name = "") = 0;
        // Creates a timestamp query, which can be used to measure GPU execution time for specific operations, with an optional name for debugging purposes
        virtual std::unique_ptr<TimestampQuery> CreateTimestampQuery(std::uint32_t timestampsCount, const std::string& name = "") = 0;
        // Creates a command signature based on the provided indirect argument descriptions and pipeline state, allowing the application to define the layout 
        // of indirect command buffers for GPU execution, with an optional name for debugging purposes
        virtual std::unique_ptr<CommandSignature> CreateCommandSignature(const std::vector<IndirectArgumentDescription>& arguments, PipelineState* pipelineState, const std::string& name = "") = 0;
        // Creates a swap chain for the specified window handle, dimensions, and vertical sync setting, allowing the application to manage the presentation of 
        // rendered frames to the screen, with an optional name for debugging purposes
        virtual std::unique_ptr<SwapChain> CreateSwapChain(void* windowHandle, std::uint32_t width, std::uint32_t height, bool vSync) = 0;
        virtual std::unique_ptr<PipelineState> CreatePipelineState(const std::string& filepath) = 0;

        // Resource view creation methods for buffers and textures, allowing the device to create views that can be used for shader resource binding and rendering

        // Creates a buffer view based on the provided buffer view description and CPU descriptor, allowing the application to define how a buffer resource is accessed and
        // interpreted in shaders, with the view being stored in the specified CPU descriptor for later binding to the pipeline
        virtual void CreateBufferView(const BufferView& view, CPUDescriptor& descriptor) = 0;
        // Creates a default shader resource view (SRV) for a buffer resource, allowing it to be accessed as a read-only resource in shaders
        virtual void CreateBufferSRV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor) = 0;
        // Creates a default constant buffer view (CBV) for a buffer resource, allowing it to be accessed as a constant buffer in shaders
        virtual void CreateBufferCBV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor) = 0;
        // Creates a default unordered access view (UAV) for a buffer resource, allowing it to be accessed as a read-write resource in shaders, with an optional 
        // counter resource for atomic operations
        virtual void CreateBufferUAV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor, std::shared_ptr<Buffer> counterResource = nullptr) = 0;

        // Creates a texture view based on the provided texture view description and CPU descriptor, allowing the application to define how a texture resource is accessed and
        // interpreted in shaders, with the view being stored in the specified CPU descriptor for later binding to the pipeline
        virtual void CreateTextureView(const TextureView& view, CPUDescriptor& descriptor) = 0;
        // Creates a default render target view (RTV) for a texture resource, allowing it to be used as a render target for rendering operations
        virtual void CreateTextureRTV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor) = 0;
        // Creates a default depth stencil view (DSV) for a texture resource, allowing it to be used as a depth stencil target for rendering operations
        virtual void CreateTextureDSV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor) = 0;
        // Creates a default shader resource view (SRV) for a texture resource, allowing it to be accessed as a read-only resource in shaders
        virtual void CreateTextureSRV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor) = 0;
        // Creates a default unordered access view (UAV) for a texture resource, allowing it to be accessed as a read-write resource in shaders, with an optional 
        // mip slice for array textures
        virtual void CreateTextureUAV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor) = 0;

        // Retrieves the size of a single descriptor in the specified descriptor heap type, which is necessary for calculating offsets when copying descriptors or creating views
        virtual std::uint32_t GetDescriptorHandleIncrementSize(DescriptorHeapType type) const = 0;

        // Retrieves allocation information for buffers based on their descriptions, which can be used to determine the required size and alignment for resource creation
        virtual AllocationInfo GetAllocationInfo(const BufferDescription& description) const = 0;
        // Retrieves allocation information for textures based on their descriptions, which can be used to determine the required size and alignment for resource creation
        virtual AllocationInfo GetAllocationInfo(const TextureDescription& description) const = 0;

        // Queries the adapter information, including details about the GPU such as its name, vendor, device ID, driver version, and video memory statistics
        virtual const AdapterInfo& QueryAdapterInfo() = 0;

        // Queries the resource allocator's suballocation statistics, for profiling and debugging
        virtual AllocatorStats QueryAllocatorStats() const = 0;

        // Retrieves the GPU crash tracker interface, which can be used to track and analyze GPU crashes for debugging purposes
        virtual tracking::IGPUCrashTracker* GetCrashTracker() = 0;

        // Retrieves a pointer to the native device object, which can be used for advanced operations or interoperability with other APIs that require direct 
        // access to the underlying graphics device
        virtual void* GetNative() const = 0;
    };

    // Factory function for creating a device instance based on the specified backend API, allowing the application to choose between different graphics APIs at runtime
    std::unique_ptr<Device> CreateDevice(BackendAPI backend);
} // namespace rhi
