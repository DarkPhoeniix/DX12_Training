
#include "RHI_PCH.h"

#include "VulkanBuffer.h"

#include "ResourceIdGenerator.h"

namespace rhi::vulkan
{
    VulkanBuffer::VulkanBuffer(rhi::Device* device, VmaAllocator allocator, const rhi::BufferDescription& description, ResourceState initialState, const std::string& name)
        : _buffer(nullptr)
        , _allocator(allocator)
        , _allocation(nullptr)
        , _description(description)
        , _ID(rhi::ResourceIdGenerator::GenerateID())
        , _initialState(initialState)
        , _currentState(initialState)
        , _uavCounterOffset(description.UAVCounterOffset)
        , _device(device)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
        NOT_IMPLEMENTED();
    }

    VulkanBuffer::VulkanBuffer(rhi::Device* device, vk::Buffer nativeBuffer, const std::string& name)
        : _buffer(nativeBuffer)
        , _allocator(nullptr)
        , _allocation(nullptr)
        , _ID(rhi::ResourceIdGenerator::GenerateID())
        , _initialState(ResourceState::Common)
        , _currentState(ResourceState::Common)
        , _uavCounterOffset(std::uint32_t(-1))
        , _device(device)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
    }

    VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept
        : rhi::Buffer(std::move(other))
        , _buffer(std::exchange(other._buffer, nullptr))
        , _allocator(std::exchange(other._allocator, nullptr))
        , _allocation(std::exchange(other._allocation, nullptr))
        , _description(std::move(other._description))
        , _ID(std::move(other._ID))
        , _initialState(other._initialState)
        , _currentState(other._currentState)
        , _uavCounterOffset(other._uavCounterOffset)
        , _device(other._device)
#if ENABLE_DEBUG_NAMES
        , _name(std::move(other._name))
#endif // ENABLE_DEBUG_NAMES
    {
    }

    VulkanBuffer::~VulkanBuffer()
    {
        NOT_IMPLEMENTED();
    }

    VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& other) noexcept
    {
        if (this != &other)
        {
            rhi::Buffer::operator=(std::move(other));
            _buffer          = std::exchange(other._buffer, nullptr);
            _allocator       = std::exchange(other._allocator, nullptr);
            _allocation      = std::exchange(other._allocation, nullptr);
            _description     = std::move(other._description);
            _ID              = std::move(other._ID);
            _initialState    = other._initialState;
            _currentState    = other._currentState;
            _uavCounterOffset = other._uavCounterOffset;
            _device          = other._device;
#if ENABLE_DEBUG_NAMES
            _name = std::move(other._name);
#endif // ENABLE_DEBUG_NAMES
        }
        return *this;
    }

    void* VulkanBuffer::Map(std::uint32_t, std::uint32_t)
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    void VulkanBuffer::Unmap()
    {
        NOT_IMPLEMENTED();
    }

    std::uint64_t VulkanBuffer::GetVirtualAddress(std::uint64_t offset)
    {
        NOT_IMPLEMENTED();
        return offset;
    }

    ResourceState VulkanBuffer::GetInitialState() const
    {
        return _initialState;
    }

    ResourceState VulkanBuffer::GetCurrentState() const
    {
        return _currentState;
    }

    void VulkanBuffer::SetCurrentState(ResourceState state)
    {
        _currentState = state;
    }

    const BufferDescription& VulkanBuffer::GetDescription() const
    {
        return _description;
    }

    std::uint32_t VulkanBuffer::GetSize() const
    {
        return _description.Size;
    }

    std::uint32_t VulkanBuffer::GetStride() const
    {
        return _description.Stride;
    }

    std::uint32_t VulkanBuffer::GetElementCount() const
    {
        return (_description.Stride > 0) ? static_cast<std::uint32_t>(_description.Size / _description.Stride) : 0;
    }

    std::uint32_t VulkanBuffer::GetUAVCounterOffset() const
    {
        return _uavCounterOffset;
    }

    const ResourceID& VulkanBuffer::GetID() const
    {
        return _ID;
    }

    void* VulkanBuffer::GetNative() const
    {
        return VulkanNative(_buffer);
    }
} // namespace rhi::vulkan
