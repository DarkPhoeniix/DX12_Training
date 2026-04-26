#pragma once

#include "Buffer.h"
#include "Texture.h"

namespace rhi
{
    // HeapType represents the type of heap, which determines the intended usage and memory properties of the heap. 
    // It is used to specify how the heap should be allocated and accessed by the GPU and CPU
    enum class HeapType
    {
        Default,    // Resources only used by the GPU
        Upload,     // Resources that are written to by the CPU and read by the GPU (CPU access optimized, limited bandwidth for the GPU)
        GPUUpload,
        Readback,   // Resources that are read by the CPU and written to by the GPU
        Custom      // Resources with custom properties specified by the application
    };

    // CPUPageProperty represents the properties of CPU pages for a heap, which can affect the performance and behavior of CPU access to the heap
    enum class CPUPageProperty
    {
        Unknown,
        NotAvailable,
        WriteCombine,
        Writeback
    };

    // MemoryPool represents the memory pool preference for a heap, which can influence the allocation and management of memory for the heap based on the underlying hardware architecture
    enum class MemoryPool
    {
        Unknown,
        L0,
        L1
    };

    // HeapProperties encapsulates the properties and configuration of a heap
    struct HeapProperties
    {
        HeapType Type = HeapType::Default;
        CPUPageProperty CPUPageProperty = CPUPageProperty::Unknown;
        MemoryPool MemoryPoolPreference = MemoryPool::Unknown;
        std::uint32_t CreationNodeMask = 0;
        std::uint32_t VisibleNodeMask = 0;
    };

    // HeapDescription encapsulates the properties and configuration of a heap. It is used to specify how the heap should be allocated and managed by the GPU and CPU
    struct HeapDescription
    {
        std::uint64_t SizeInBytes = 0;
        HeapProperties Properties = {};
        std::uint64_t Alignment = 0;
        std::uint32_t Flags = 0;
    };

    // Heap is an abstract interface representing a heap, which is a memory allocation that can be used to place GPU resources such as buffers and textures
    class Heap
    {
    public:
        Heap() = default;
        Heap(const Heap&) = delete;
        Heap(Heap&&) = default;
        virtual ~Heap() = default;

        Heap& operator=(const Heap&) = delete;
        Heap& operator=(Heap&&) = default;

        // Places a buffer in the heap based on the provided description and returns a shared pointer to the created buffer. 
        // The state parameter specifies the initial resource state for the placed buffer, and the offset parameter allows specifying an optional offset within the heap for placing the buffer, 
        // otherwise the buffer will be placed at the next available offset in the heap
        virtual std::shared_ptr<Buffer> PlaceResource(const rhi::BufferDescription& bufferDesc, ResourceState state = ResourceState::Common, std::uint64_t offset = (std::uint64_t)-1) = 0;
        // Places a texture in the heap based on the provided description and returns a shared pointer to the created texture. 
        // The state parameter specifies the initial resource state for the placed texture, and the offset parameter allows specifying an optional offset within the heap for placing the texture, 
        // otherwise the texture will be placed at the next available offset in the heap
        virtual std::shared_ptr<Texture> PlaceResource(const rhi::TextureDescription& textureDesc, ResourceState state = ResourceState::Common, std::uint64_t offset = (std::uint64_t)-1) = 0;

        // Resets the heap, freeing all resources placed in the heap and allowing the heap to be reused for new resource placements
        virtual void Reset() = 0;

        // Retrieves the native heap pointer
        virtual void* GetNative() const = 0;
    };
} // namespace rhi
