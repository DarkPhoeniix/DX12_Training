
#include "RHI_PCH.h"

#include "VulkanBuffer.h"

#include "ResourceIdGenerator.h"

#include <vma/vk_mem_alloc.h>

namespace rhi::vulkan
{
    namespace
    {
        constexpr vk::BufferUsageFlags kBufferUsage =
            vk::BufferUsageFlagBits::eTransferSrc |
            vk::BufferUsageFlagBits::eTransferDst |
            vk::BufferUsageFlagBits::eUniformBuffer |
            vk::BufferUsageFlagBits::eStorageBuffer |
            vk::BufferUsageFlagBits::eIndexBuffer |
            vk::BufferUsageFlagBits::eVertexBuffer |
            vk::BufferUsageFlagBits::eIndirectBuffer |
            vk::BufferUsageFlagBits::eShaderDeviceAddress;

        VmaAllocationCreateInfo GetAllocationInfo(ResourceUsage usage)
        {
            VmaAllocationCreateInfo info = {};
            info.usage = VMA_MEMORY_USAGE_AUTO;

            switch (usage)
            {
            case ResourceUsage::Default:
                break;
            case ResourceUsage::Upload:
                info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
                break;
            case ResourceUsage::GPUUpload:
                info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
                info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
                break;
            case ResourceUsage::Readback:
                info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
                break;
            default:
                UNREACHABLE("Unsupported resource usage!");
                break;
            }

            return info;
        }
    } // namespace unnamed

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
        const vk::BufferCreateInfo createInfo =
        {
            .size = description.Size,
            .usage = kBufferUsage,
            .sharingMode = vk::SharingMode::eExclusive
        };

        const VmaAllocationCreateInfo allocationInfo = GetAllocationInfo(description.Usage);

        VkBuffer buffer = VK_NULL_HANDLE;
        VkResult result = vmaCreateBuffer(_allocator,
            reinterpret_cast<const VkBufferCreateInfo*>(&createInfo),
            &allocationInfo,
            &buffer,
            &_allocation,
            nullptr);
        VK_CHECK(static_cast<vk::Result>(result), "Failed to allocate buffer");

        _buffer = buffer;

        SetVulkanName(VulkanCast<vk::Device>(_device->GetNative()), _buffer, name);
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
        if (_allocation)
        {
            vmaDestroyBuffer(_allocator, _buffer, _allocation);
        }
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
        ASSERT(_allocation, "Trying to map a buffer that owns no allocation.");

        void* data = nullptr;
        VkResult result = vmaMapMemory(_allocator, _allocation, &data);
        VK_CHECK(static_cast<vk::Result>(result), "Failed to map buffer");

        return data;
    }

    void VulkanBuffer::Unmap()
    {
        ASSERT(_allocation, "Trying to unmap a buffer that owns no allocation.");
        vmaUnmapMemory(_allocator, _allocation);
    }

    std::uint64_t VulkanBuffer::GetVirtualAddress(std::uint64_t offset)
    {
        vk::Device logicalDevice = VulkanCast<vk::Device>(_device->GetNative());

        const vk::BufferDeviceAddressInfo addressInfo = { .buffer = _buffer };

        return logicalDevice.getBufferAddress(addressInfo) + offset;
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
