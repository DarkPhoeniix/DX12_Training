#pragma once

#include "Buffer.h"
#include "Texture.h"

namespace rhi
{
    enum class HeapType
    {
        Default,
        Upload,
        GPUUpload,
        Readback,
        Custom
    };

    enum class CPUPageProperty
    {
        Unknown,
        NotAvailable,
        WriteCombine,
        Writeback
    };

    enum class MemoryPool
    {
        Unknown,
        L0,
        L1
    };

    struct HeapProperties
    {
        HeapType Type = HeapType::Default;
        CPUPageProperty CPUPageProperty = CPUPageProperty::Unknown;
        MemoryPool MemoryPoolPreference = MemoryPool::Unknown;
        std::uint32_t CreationNodeMask = 0;
        std::uint32_t VisibleNodeMask = 0;
    };

    struct HeapDescription
    {
        std::uint64_t SizeInBytes = 0;
        HeapProperties Properties = {};
        std::uint64_t Alignment = 0;
        std::uint32_t Flags = 0;
    };

    class Heap
    {
    public:
        Heap() = default;
        Heap(const Heap&) = delete;
        Heap(Heap&&) = default;
        virtual ~Heap() = default;

        Heap& operator=(const Heap&) = delete;
        Heap& operator=(Heap&&) = default;

        virtual std::shared_ptr<Buffer> PlaceResource(const rhi::BufferDescription& bufferDesc, ResourceState state = ResourceState::Common, std::uint64_t offset = (std::uint64_t)-1) = 0;
        virtual std::shared_ptr<Texture> PlaceResource(const rhi::TextureDescription& textureDesc, ResourceState state = ResourceState::Common, std::uint64_t offset = (std::uint64_t)-1) = 0;

        virtual void Reset() = 0;

        virtual void* GetNative() const = 0;
    };
} // namespace rhi
