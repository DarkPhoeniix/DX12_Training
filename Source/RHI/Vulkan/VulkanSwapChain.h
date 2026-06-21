#pragma once

#include "SwapChain.h"

#include "VulkanDevice.h"

namespace rhi
{
    class DescriptorHeap;
    struct ScissorRect;
} // namespace rhi

namespace rhi::vulkan
{
    class VulkanSwapChain final : public rhi::SwapChain
    {
    public:
        VulkanSwapChain(const VulkanSwapChain& other) = delete;
        VulkanSwapChain(VulkanSwapChain&& other) noexcept;
        ~VulkanSwapChain() override;

        VulkanSwapChain& operator=(const VulkanSwapChain& other) = delete;
        VulkanSwapChain& operator=(VulkanSwapChain&& other) noexcept;

        std::shared_ptr<rhi::Texture> GetBuffer(std::uint32_t index) override;
        std::shared_ptr<rhi::Texture> GetBackBuffer() override;

        std::uint32_t Present() override;

        void OnResize(std::uint32_t width, std::uint32_t height) override;

        rhi::ScissorRect GetDesktopCoordinates() override;

        void* GetNative() const override;

    private:
        friend class VulkanDevice;

        VulkanSwapChain(VulkanDevice* device, HWND windowHandle, std::uint32_t width, std::uint32_t height, bool vSync);

        vk::SurfaceKHR CreateSurface(HWND windowHandle);
        vk::SwapchainKHR CreateSwapChain(HWND windowHandle, std::uint32_t width, std::uint32_t height, bool vSync);

        VulkanDevice* _device;

        vk::SurfaceKHR _surface;
        vk::SwapchainKHR _swapChain;
    };
} // namespace rhi::vulkan
