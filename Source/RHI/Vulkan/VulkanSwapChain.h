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
    class VulkanSwapChain final : public SwapChain
    {
    public:
        VulkanSwapChain(const VulkanSwapChain& other) = delete;
        VulkanSwapChain(VulkanSwapChain&& other) noexcept;
        ~VulkanSwapChain() override;

        VulkanSwapChain& operator=(const VulkanSwapChain& other) = delete;
        VulkanSwapChain& operator=(VulkanSwapChain&& other) noexcept;

        std::shared_ptr<Texture> GetBuffer(std::uint32_t index) override;
        std::shared_ptr<Texture> GetBackBuffer() override;

        std::uint32_t Present() override;

        void OnResize(std::uint32_t width, std::uint32_t height) override;

        ScissorRect GetDesktopCoordinates() override;

        void* GetNative() const override;

        bool OwnsTexture(const Texture* texture) const;

        vk::Semaphore ConsumeAcquireSemaphore();
        vk::Semaphore GetRenderFinishedSemaphore() const;

    private:
        friend class VulkanDevice;

        VulkanSwapChain(VulkanDevice* device, HWND windowHandle, std::uint32_t width, std::uint32_t height, bool vSync);

        vk::SurfaceKHR CreateSurface(HWND windowHandle);
        vk::SwapchainKHR CreateSwapChain(vk::SwapchainKHR oldSwapChain);
        void CreateBackBuffers();
        void CreateSemaphores();
        void DestroySemaphores();
        void RecreateSwapChain();
        std::uint32_t AcquireNextImage();

        VulkanDevice* _device;

        HWND _windowHandle;
        std::uint32_t _width;
        std::uint32_t _height;

        bool _vSync;
        bool _tearingSupport;

        vk::Queue _presentQueue;
        vk::Format _imageFormat;
        vk::SurfaceKHR _surface;
        vk::SwapchainKHR _swapChain;

        std::vector<std::shared_ptr<Texture>> _backBuffers;
        std::uint32_t _currentBackBufferIndex;

        // Acquire semaphores are indexed by frame, not by image: the image index is only
        // known once the acquire has completed
        std::vector<vk::Semaphore> _acquireSemaphores;
        std::vector<vk::Semaphore> _renderFinishedSemaphores;
        std::uint32_t _semaphoreIndex;
        bool _acquireSemaphoreConsumed;
    };
} // namespace rhi::vulkan
