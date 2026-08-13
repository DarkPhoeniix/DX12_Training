
#include "RHI_PCH.h"

#include "VulkanDescriptorHeap.h"

#include "VulkanDevice.h"

namespace rhi::vulkan
{
    namespace
    {
        const std::array<vk::DescriptorType, 4> kMutableTypes =
        {
            vk::DescriptorType::eSampledImage,
            vk::DescriptorType::eStorageImage,
            vk::DescriptorType::eUniformBuffer,
            vk::DescriptorType::eStorageBuffer
        };
    } // namespace unnamed

    VulkanDescriptorHeap::VulkanDescriptorHeap(VulkanDevice* device, const DescriptorHeapDescription& description, const std::string& name)
        : _device(device)
        , _description(description)
        , _pool(nullptr)
        , _layout(nullptr)
        , _set(nullptr)
        , _imageViews(description.NumDescriptors, nullptr)
        , _imageViewExtents(description.NumDescriptors)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
        if (_description.Type == DescriptorHeapType::CBV_SRV_UAV)
        {
            CreateDescriptorSet(name);
        }
    }

    VulkanDescriptorHeap::VulkanDescriptorHeap(VulkanDescriptorHeap&& other) noexcept
        : _device(std::exchange(other._device, nullptr))
        , _description(other._description)
        , _pool(std::exchange(other._pool, nullptr))
        , _layout(std::exchange(other._layout, nullptr))
        , _set(std::exchange(other._set, nullptr))
        , _imageViews(std::move(other._imageViews))
        , _imageViewExtents(std::move(other._imageViewExtents))
#if ENABLE_DEBUG_NAMES
        , _name(std::move(other._name))
#endif // ENABLE_DEBUG_NAMES
    {
    }

    VulkanDescriptorHeap::~VulkanDescriptorHeap()
    {
        if (!_device)
        {
            return;
        }

        vk::Device logicalDevice = VulkanCast<vk::Device>(_device->GetNative());

        DestroyImageViews();

        if (_pool)
        {
            logicalDevice.destroyDescriptorPool(_pool);
        }
        if (_layout)
        {
            logicalDevice.destroyDescriptorSetLayout(_layout);
        }
    }

    VulkanDescriptorHeap& VulkanDescriptorHeap::operator=(VulkanDescriptorHeap&& other) noexcept
    {
        if (this != &other)
        {
            _device = std::exchange(other._device, nullptr);
            _description = other._description;
            _pool = std::exchange(other._pool, nullptr);
            _layout = std::exchange(other._layout, nullptr);
            _set = std::exchange(other._set, nullptr);
            _imageViews = std::move(other._imageViews);
            _imageViewExtents = std::move(other._imageViewExtents);
#if ENABLE_DEBUG_NAMES
            _name = std::move(other._name);
#endif // ENABLE_DEBUG_NAMES
        }

        return *this;
    }

    void VulkanDescriptorHeap::Reset()
    {
        DestroyImageViews();
    }

    std::uint32_t VulkanDescriptorHeap::CopyResourceDescriptor(CPUDescriptor)
    {
        NOT_IMPLEMENTED();
        return std::uint32_t();
    }

    CPUDescriptor VulkanDescriptorHeap::GetHeapStartCPUHandle()
    {
        NOT_IMPLEMENTED();
        return CPUDescriptor();
    }

    GPUDescriptor VulkanDescriptorHeap::GetHeapStartGPUHandle()
    {
        NOT_IMPLEMENTED();
        return GPUDescriptor();
    }

    CPUDescriptor VulkanDescriptorHeap::GetCPUHandleWithOffset(std::uint32_t offset)
    {
        ASSERT(offset < _description.NumDescriptors, "Descriptor offset is out of bounds for the heap.");

        CPUDescriptor descriptor;
        descriptor.ptr = offset;
        descriptor.Heap = this;

        return descriptor;
    }

    GPUDescriptor VulkanDescriptorHeap::GetGPUHandleWithOffset(std::uint32_t offset)
    {
        ASSERT(offset < _description.NumDescriptors, "Descriptor offset is out of bounds for the heap.");

        GPUDescriptor descriptor;
        descriptor.ptr = offset;
        descriptor.Heap = this;

        return descriptor;
    }

    std::uint32_t VulkanDescriptorHeap::Offset()
    {
        NOT_IMPLEMENTED();
        return std::uint32_t();
    }

    std::uint32_t VulkanDescriptorHeap::GetCurrentOffset() const
    {
        NOT_IMPLEMENTED();
        return std::uint32_t();
    }

    void* VulkanDescriptorHeap::GetNative() const
    {
        return VulkanNative(_set);
    }

    vk::DescriptorSet VulkanDescriptorHeap::GetDescriptorSet() const
    {
        return _set;
    }

    vk::DescriptorSetLayout VulkanDescriptorHeap::GetDescriptorSetLayout() const
    {
        return _layout;
    }

    void VulkanDescriptorHeap::SetImageView(std::uint32_t slot, vk::ImageView view, vk::Extent2D extent)
    {
        ASSERT(slot < _imageViews.size(), "Descriptor slot is out of bounds for the heap.");

        vk::Device logicalDevice = VulkanCast<vk::Device>(_device->GetNative());
        if (_imageViews[slot])
        {
            logicalDevice.destroyImageView(_imageViews[slot]);
        }

        _imageViews[slot] = view;
        _imageViewExtents[slot] = extent;
    }

    vk::ImageView VulkanDescriptorHeap::GetImageView(std::uint32_t slot) const
    {
        ASSERT(slot < _imageViews.size(), "Descriptor slot is out of bounds for the heap.");
        return _imageViews[slot];
    }

    vk::Extent2D VulkanDescriptorHeap::GetImageViewExtent(std::uint32_t slot) const
    {
        ASSERT(slot < _imageViewExtents.size(), "Descriptor slot is out of bounds for the heap.");
        return _imageViewExtents[slot];
    }

    void VulkanDescriptorHeap::WriteImageDescriptor(std::uint32_t slot, vk::ImageView view, vk::ImageLayout layout, vk::DescriptorType type)
    {
        ASSERT(_set, "Trying to write a descriptor into a heap that has no descriptor set.");

        SetImageView(slot, view);

        const vk::DescriptorImageInfo imageInfo =
        {
            .imageView = view,
            .imageLayout = layout
        };

        const vk::WriteDescriptorSet write =
        {
            .dstSet = _set,
            .dstBinding = 0,
            .dstArrayElement = slot,
            .descriptorCount = 1,
            .descriptorType = type,
            .pImageInfo = &imageInfo
        };

        vk::Device logicalDevice = VulkanCast<vk::Device>(_device->GetNative());
        logicalDevice.updateDescriptorSets(write, {});
    }

    void VulkanDescriptorHeap::WriteBufferDescriptor(std::uint32_t slot, vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range, vk::DescriptorType type)
    {
        ASSERT(_set, "Trying to write a descriptor into a heap that has no descriptor set.");
        ASSERT(slot < _description.NumDescriptors, "Descriptor slot is out of bounds for the heap.");

        const vk::DescriptorBufferInfo bufferInfo =
        {
            .buffer = buffer,
            .offset = offset,
            .range = range
        };

        const vk::WriteDescriptorSet write =
        {
            .dstSet = _set,
            .dstBinding = 0,
            .dstArrayElement = slot,
            .descriptorCount = 1,
            .descriptorType = type,
            .pBufferInfo = &bufferInfo
        };

        vk::Device logicalDevice = VulkanCast<vk::Device>(_device->GetNative());
        logicalDevice.updateDescriptorSets(write, {});
    }

    void VulkanDescriptorHeap::CreateDescriptorSet(const std::string& name)
    {
        vk::Device logicalDevice = VulkanCast<vk::Device>(_device->GetNative());

        // One mutable-type array gives the flat index space that ResourceDescriptorHeap expects
        const vk::MutableDescriptorTypeListEXT typeList =
        {
            .descriptorTypeCount = static_cast<std::uint32_t>(kMutableTypes.size()),
            .pDescriptorTypes = kMutableTypes.data()
        };

        const vk::MutableDescriptorTypeCreateInfoEXT mutableInfo =
        {
            .mutableDescriptorTypeListCount = 1,
            .pMutableDescriptorTypeLists = &typeList
        };

        constexpr vk::DescriptorBindingFlags bindingFlags =
            vk::DescriptorBindingFlagBits::ePartiallyBound |
            vk::DescriptorBindingFlagBits::eUpdateAfterBind |
            vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending;

        const vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo =
        {
            .pNext = &mutableInfo,
            .bindingCount = 1,
            .pBindingFlags = &bindingFlags
        };

        const vk::DescriptorSetLayoutBinding binding =
        {
            .binding = 0,
            .descriptorType = vk::DescriptorType::eMutableEXT,
            .descriptorCount = _description.NumDescriptors,
            .stageFlags = vk::ShaderStageFlagBits::eAll
        };

        const vk::DescriptorSetLayoutCreateInfo layoutInfo =
        {
            .pNext = &bindingFlagsInfo,
            .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
            .bindingCount = 1,
            .pBindings = &binding
        };

        auto [layoutResult, layout] = logicalDevice.createDescriptorSetLayout(layoutInfo);
        VK_CHECK(layoutResult, "Failed to create bindless descriptor set layout");
        _layout = layout;

        const vk::DescriptorPoolSize poolSize =
        {
            .type = vk::DescriptorType::eMutableEXT,
            .descriptorCount = _description.NumDescriptors
        };

        const vk::DescriptorPoolCreateInfo poolInfo =
        {
            .pNext = &mutableInfo,
            .flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
            .maxSets = 1,
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize
        };

        auto [poolResult, pool] = logicalDevice.createDescriptorPool(poolInfo);
        VK_CHECK(poolResult, "Failed to create bindless descriptor pool");
        _pool = pool;

        const vk::DescriptorSetAllocateInfo allocateInfo =
        {
            .descriptorPool = _pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &_layout
        };

        auto [setResult, sets] = logicalDevice.allocateDescriptorSets(allocateInfo);
        VK_CHECK(setResult, "Failed to allocate bindless descriptor set");
        _set = sets[0];

        SetVulkanName(logicalDevice, _set, name);
    }

    void VulkanDescriptorHeap::DestroyImageViews()
    {
        vk::Device logicalDevice = VulkanCast<vk::Device>(_device->GetNative());

        for (vk::ImageView& view : _imageViews)
        {
            if (view)
            {
                logicalDevice.destroyImageView(view);
                view = nullptr;
            }
        }
    }
} // namespace rhi::vulkan
