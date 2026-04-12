#pragma once

#include "Format.h"
#include "ResourceCommon.h"

namespace rhi
{
    // BufferDescription encapsulates the properties and configuration of a GPU buffer resource
    struct BufferDescription
    {
        std::uint32_t Size              = 0;
        std::uint32_t Stride            = 0;
        Format Format                   = Format::UNKNOWN;
        ResourceUsage Usage             = ResourceUsage::Default;
        ResourceFlags Flags             = ResourceFlags::None;
        std::uint32_t UAVCounterOffset  = std::uint32_t(-1);
    };

    // Buffer is an abstract interface representing a GPU buffer resource
    class Buffer
    {
    public:
        Buffer() = default;
        Buffer(const Buffer&) = delete;
        Buffer(Buffer&&) noexcept = default;
        virtual ~Buffer() = default;

        Buffer& operator=(const Buffer&) = delete;
        Buffer& operator=(Buffer&&) noexcept = default;

        // Maps the buffer resource to CPU accessible memory, allowing the application to read or write data to the buffer.
        // The begin and end parameters specify the range of the buffer to map, with default values indicating the entire buffer.
        // The template version of the Map function provides a convenient way to map the buffer and obtain a typed pointer to the mapped memory
        template<typename Type>
        [[nodiscard]] Type* Map(std::uint32_t begin = 0, std::uint32_t end = 0);
        // Maps the buffer resource to CPU accessible memory, allowing the application to read or write data to the buffer.
        // The begin and end parameters specify the range of the buffer to map, with default values indicating the entire buffer
        virtual void* Map(std::uint32_t begin = 0, std::uint32_t end = 0) = 0;
        // Unmaps the buffer resource, indicating that the application has finished accessing the mapped memory and allowing the GPU to use the buffer again
        virtual void Unmap() = 0;

        // Retrieves the virtual address of the buffer resource, which can be used for GPU access and binding. 
        // The offset parameter allows retrieving the virtual address with an additional offset from the base address of the buffer
        [[nodiscard]] virtual std::uint64_t GetVirtualAddress(std::uint64_t offset = 0) = 0;
        // Retrieves the initial resource state of the buffer, which indicates how the buffer is expected to be used when it is first created
        [[nodiscard]] virtual ResourceState GetInitialState() const = 0;
        // Retrieves the current resource state of the buffer, which indicates how the buffer is currently being used and may differ from the initial state
        [[nodiscard]] virtual ResourceState GetCurrentState() const = 0;
        // Sets the current resource state of the buffer, allowing the application to track and manage resource state transitions for proper synchronization and usage
        virtual void SetCurrentState(ResourceState state) = 0;

        // Retrieves the description of the buffer, which contains information about its size, stride, format, usage, flags, and UAV counter offset
        virtual const BufferDescription& GetDescription() const = 0;
        // Convenience methods for retrieving size of the buffer
        virtual std::uint32_t GetSize() const = 0;
        // Convenience methods for retrieving stride of the buffer, which is the size of each element in the buffer and is used for structured buffers or vertex buffers
        virtual std::uint32_t GetStride() const = 0;
        // Convenience methods for retrieving the number of elements in the buffer, which is calculated based on the size and stride
        virtual std::uint32_t GetElementCount() const = 0;
        
        // Retrieves the UAV counter offset of the buffer, which is used for unordered access views with counters
        virtual std::uint32_t GetUAVCounterOffset() const = 0;

        // Retrieves the unique identifier of the buffer resource.
        // ID unique across all resources (both buffers and textures) created by the device
        virtual const ResourceID& GetID() const = 0;

        // Retrieves a pointer to the native buffer resource
        virtual void* GetNative() const = 0;
    };

    template<typename Type>
    [[nodiscard]] Type* Buffer::Map(std::uint32_t begin, std::uint32_t end)
    {
        return static_cast<Type*>(Map(begin, end));
    }
} // namespace rhi
