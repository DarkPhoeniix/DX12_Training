#pragma once

#include "Format.h"
#include "ResourceCommon.h"

namespace rhi
{
    struct BufferDescription
    {
        std::uint32_t Size      = 0;
        std::uint32_t Stride    = 0;
        Format Format           = Format::UNKNOWN;
        ResourceUsage Usage     = ResourceUsage::Default;
        ResourceFlags Flags     = ResourceFlags::None;
    };

    class Buffer
    {
    public:
        Buffer() = default;
        Buffer(const Buffer&) = delete;
        Buffer(Buffer&&) noexcept = default;
        virtual ~Buffer() = default;

        Buffer& operator=(const Buffer&) = delete;
        Buffer& operator=(Buffer&&) noexcept = default;

        template<typename Type>
        [[nodiscard]] Type* Map(std::uint32_t begin = 0, std::uint32_t end = 0);
        virtual void* Map(std::uint32_t begin = 0, std::uint32_t end = 0) = 0;
        virtual void Unmap() = 0;

        [[nodiscard]] virtual std::uint64_t GetVirtualAddress(std::uint64_t offset = 0) = 0;
        [[nodiscard]] virtual ResourceState GetInitialState() const = 0;
        [[nodiscard]] virtual ResourceState GetCurrentState() const = 0;
        virtual void SetCurrentState(ResourceState state) = 0;

        virtual const BufferDescription& GetDescription() const = 0;
        virtual std::uint32_t GetSize() const = 0;
        virtual std::uint32_t GetStride() const = 0;
        virtual std::uint32_t GetElementCount() const = 0;

        virtual std::uint32_t GetUAVCounterOffset() const = 0;

        virtual const ResourceID& GetID() const = 0;

        virtual void* GetNative() const = 0;
    };

    template<typename Type>
    [[nodiscard]] Type* Buffer::Map(std::uint32_t begin, std::uint32_t end)
    {
        return static_cast<Type*>(Map(begin, end));
    }
} // namespace rhi
