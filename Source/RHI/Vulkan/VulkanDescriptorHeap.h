#pragma once

#include "DescriptorHeap.h"

#include "VulkanHelpers.h"

namespace rhi::vulkan
{
    class VulkanDevice;

    class VulkanDescriptorHeap final : public DescriptorHeap
    {
    public:
        VulkanDescriptorHeap(const VulkanDescriptorHeap& other) = delete;
        VulkanDescriptorHeap(VulkanDescriptorHeap&& other) noexcept;
        ~VulkanDescriptorHeap() override;

        VulkanDescriptorHeap& operator=(const VulkanDescriptorHeap& other) = delete;
        VulkanDescriptorHeap& operator=(VulkanDescriptorHeap&& other) noexcept;

        void Reset() override;

        std::uint32_t CopyResourceDescriptor(CPUDescriptor descriptor) override;

        CPUDescriptor GetHeapStartCPUHandle() override;
        GPUDescriptor GetHeapStartGPUHandle() override;

        CPUDescriptor GetCPUHandleWithOffset(std::uint32_t offset) override;
        GPUDescriptor GetGPUHandleWithOffset(std::uint32_t offset) override;

        std::uint32_t Offset() override;
        std::uint32_t GetCurrentOffset() const override;

        void* GetNative() const override;

        vk::DescriptorSet GetDescriptorSet() const;
        vk::DescriptorSetLayout GetDescriptorSetLayout() const;

        void SetImageView(std::uint32_t slot, vk::ImageView view, vk::Extent2D extent = {});

        vk::ImageView GetImageView(std::uint32_t slot) const;
        vk::Extent2D GetImageViewExtent(std::uint32_t slot) const;

        void WriteImageDescriptor(std::uint32_t slot, vk::ImageView view, vk::ImageLayout layout, vk::DescriptorType type);
        void WriteBufferDescriptor(std::uint32_t slot, vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range, vk::DescriptorType type);

    private:
        friend class VulkanDevice;

        VulkanDescriptorHeap(VulkanDevice* device, const DescriptorHeapDescription& description, const std::string& name = "");

        void CreateDescriptorSet(const std::string& name);
        void DestroyImageViews();

        VulkanDevice* _device;
        DescriptorHeapDescription _description;

        // Null for RTV/DSV heaps, which hold image views only
        vk::DescriptorPool _pool;
        vk::DescriptorSetLayout _layout;
        vk::DescriptorSet _set;

        std::vector<vk::ImageView> _imageViews;
        std::vector<vk::Extent2D> _imageViewExtents;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::vulkan
